#include <iostream>
#include <memory>
#include <getopt.h>
#include <atomic>
#include <csignal>
#include <fstream>

#include "hidb-5/hidb-set.hh"
#include "seqdb-3/seqdb.hh"
#include "acmacs-map-draw/setup-dbs.hh"
#include "acmacs-webserver/server-settings.hh"
#include "acmacs-webserver/print.hh"
#include "acmacs-api/acmacs-api-server.hh"
#include "acmacs-api/acmacs-c2.hh"
#include "acmacs-api/command.hh"
#include "acmacs-api/command-factory.hh"

// ----------------------------------------------------------------------

static std::atomic<Wspp*> sWspp;
[[noreturn]] static void signal_handler(int signal);

// ----------------------------------------------------------------------

void WsppThreadWithMongoAccess::initialize()
{
    WsppThread::initialize();
    mongo_access().create_client();

} // WsppThreadWithMongoAccess::initialize

// ----------------------------------------------------------------------

void WebsocketConnection::opening(std::string, WsppThread& /*aThread*/)
{
    send(to_json::v1::object("C", "hello", "server", "acmacs-api-server-v1"));

} // WebsocketConnection::opening

// ----------------------------------------------------------------------

void WebsocketConnection::message(std::string aMessage, WsppThread& aThread)
{
    auto& thread = dynamic_cast<WsppThreadWithMongoAccess&>(aThread);

    print_receive(log_send_receive(), aMessage);
    auto command = mCommandFactory.find(aMessage, thread.mongo_access(), *this);
    try {
        command->run();
    }
    catch (std::exception& err) {
        command->send_error(err.what());
    }

} // WebsocketConnection::message

// ----------------------------------------------------------------------

void WebsocketConnection::send(std::string aMessage, send_message_type aMessageType)
{
    auto op_code = websocketpp::frame::opcode::text;
    switch (aMessageType) {
      case send_message_type::text:
          op_code = websocketpp::frame::opcode::text;
          break;
      case send_message_type::binary:
          op_code = websocketpp::frame::opcode::binary;
          break;
    }
    print_send(log_send_receive(), aMessage);
    WsppWebsocketLocationHandler::send(aMessage, op_code);

} // WebsocketConnection::send

// ----------------------------------------------------------------------

class AcmacsAPISettings : public ServerSettings
{
 public:
    using ServerSettings::ServerSettings;

    std::string mongodb_uri() const { return std::string(rjson::get_or(doc_, "mongodb_uri", "mongodb://localhost:27017/")); }
    std::string acmacs_c2_uri() const { return std::string(rjson::get_or(doc_, "acmacs_c2_uri", "https://localhost:1168/api")); }
    std::string root_page() const { return std::string(rjson::get_or(doc_, "root_page", "/tmp/not-found")); }

}; // class AcmacsAPISettings

// ----------------------------------------------------------------------

class RootPage : public WsppHttpLocationHandler
{
 public:
    RootPage(std::string filename) : filename_{filename} {}

    bool handle(const HttpResource& aResource, WsppHttpResponseData& aResponse) override
        {
            if (aResource.location().substr(0, 4) == "/js/")
                return false;
            constexpr size_t bufsize = 1024;
            aResponse.body.resize(bufsize);
            std::ifstream input(filename_);
            input.read(aResponse.body.data(), bufsize);
            aResponse.body.resize(static_cast<size_t>(input.gcount()));
            aResponse.append_header("Content-Type", "text/html; charset=utf-8");
            return true;
        }

 private:
    std::string filename_;

}; // class RootPage


// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    if (argc != 2 || std::string{"-h"} == argv[1] || std::string{"--help"} == argv[1]) {
        std::cerr << "Usage: " << argv[0] << " <settings.json>" << std::endl;
        return 1;
    }

    try {
        mongocxx::instance inst{};
        CommandFactory command_factory;

        AcmacsAPISettings settings{argv[1]};
        // std::cout << "mongodb_uri: [" << settings.mongodb_uri() << "]" << std::endl;
        // std::cout << "acmacs_c2_uri: [" << settings.acmacs_c2_uri() << "]" << std::endl;
        // setup_dbs("", false);
        acmacs::seqdb::get();   // preload seqdb before making threads
        hidb::load_all(report_time::yes);   // preload hidb before making threads
        AcmacsC2 acmacs_c2;
        acmacs_c2.uri(settings.acmacs_c2_uri());
        auto thread_maker = [&settings,&acmacs_c2](Wspp& aWspp) -> WsppThread* {
            return new WsppThreadWithMongoAccess{aWspp, settings.mongodb_uri(), acmacs_c2};
        };
        Wspp wspp{settings, thread_maker};
        sWspp = &wspp;

        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        std::signal(SIGQUIT, signal_handler);

        wspp.add_location_handler(std::make_shared<RootPage>(settings.root_page()));
        wspp.add_location_handler(std::make_shared<WebsocketConnection>(command_factory));

        wspp.run();
        return 0;
    }
    catch (std::exception& err) {
        print_cerr("ERROR: ", err.what());
        return 1;
    }
}

// ----------------------------------------------------------------------

[[noreturn]] static void signal_handler(int signal)
{
    std::cerr << "Interrupted with signal " << signal << std::endl;
    sWspp.load()->stop_listening();
    std::exit(-signal);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
