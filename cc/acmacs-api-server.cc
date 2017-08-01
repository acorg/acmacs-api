#include <iostream>
#include <memory>
#include <getopt.h>
#include <atomic>
#include <csignal>

#include "acmacs-webserver/server-settings.hh"
#include "acmacs-webserver/print.hh"

#include "acmacs-api-server.hh"
#include "acmacs-c2.hh"
#include "command.hh"
#include "command-factory.hh"

// ----------------------------------------------------------------------

static std::atomic<Wspp*> sWspp;
[[noreturn]] static void signal_handler(int signal);

// ----------------------------------------------------------------------

void WsppThreadWithMongoAccess::initialize()
{
    WsppThread::initialize();
    create_client();

} // WsppThreadWithMongoAccess::initialize

// ----------------------------------------------------------------------

void BrowserConnection::message(std::string aMessage, WsppThread& aThread)
{
    auto& thread = dynamic_cast<WsppThreadWithMongoAccess&>(aThread);

    print_cerr("MSG: ", aMessage.substr(0, 80));
    auto command = mCommandFactory.find(aMessage, thread, *this);
    try {
        command->run();
    }
    catch (std::exception& err) {
        command->send_error(err.what());
    }

} // BrowserConnection::message

// ----------------------------------------------------------------------

void BrowserConnection::send(std::string aMessage, send_message_type aMessageType)
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
    print_cerr("SEND: ", aMessage.substr(0, 100));
    WsppWebsocketLocationHandler::send(aMessage, op_code);

} // BrowserConnection::send

// ----------------------------------------------------------------------

class AcmacsAPISettings : public ServerSettings
{
 public:
    using ServerSettings::ServerSettings;

    inline auto mongodb_uri() const
        {
            auto uri = mDoc.get("mongodb_uri", std::string{});
            if (uri.empty())
                uri = "mongodb://localhost:27017/";
            return uri;
        }

    inline auto acmacs_c2_uri() const
        {
            auto uri = mDoc.get("acmacs_c2_uri", std::string{});
            if (uri.empty())
                uri = "https://localhost:1168/api";
            return uri;
        }
};

// ----------------------------------------------------------------------

class RootPage : public WsppHttpLocationHandler
{
 public:
    virtual inline bool handle(const HttpResource& aResource, WsppHttpResponseData& aResponse)
        {
            // std::cerr << "ARGV: " << aResource.argv() << std::endl;
            // std::cerr << "ARGV: " << to_json::object(aResource.argv()) << std::endl;
            bool handled = false;
            if (aResource.location() == "/") {
                aResponse.body = R"(<html><head>
                                    <link rel="stylesheet" type="text/css" href="css/acmacs-api-client.css">
                                    <script src="/js/acmacs-api-client.js"></script>
                                    <script src="/js/lib/md5.js"></script>)";
                aResponse.body += "<script>ARGV = " + to_json::object(aResource.argv()) + "</script>";
                aResponse.body += "</head><body><h1>acmacs-api-server</h1></body></html>";
                handled = true;
            }
            return handled;
        }

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
        std::cout << "mongodb_uri: [" << settings.mongodb_uri() << "]" << std::endl;
        std::cout << "acmacs_c2_uri: [" << settings.acmacs_c2_uri() << "]" << std::endl;
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

        wspp.add_location_handler(std::make_shared<RootPage>());
        wspp.add_location_handler(std::make_shared<BrowserConnection>(command_factory));

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
