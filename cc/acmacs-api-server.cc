#include <iostream>
#include <memory>
#include <getopt.h>
#include <atomic>
#include <csignal>

#include "acmacs-webserver/server-settings.hh"
#include "acmacs-webserver/print.hh"

#include "acmacs-api-server.hh"
#include "command.hh"
#include "command-factory.hh"
#include "bson-to-json.hh"

// ----------------------------------------------------------------------

static std::atomic<Wspp*> sWspp;
[[noreturn]] static void signal_handler(int signal);

// ----------------------------------------------------------------------

void WsppThreadWithMongoAccess::initialize()
{
    WsppThread::initialize();
    mClient = mongocxx::client{mongocxx::uri{mMongoURI}};

} // WsppThreadWithMongoAccess::initialize

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
};

// ----------------------------------------------------------------------

class RootPage : public WsppHttpLocationHandler
{
 public:
    virtual inline bool handle(const HttpResource& aResource, WsppHttpResponseData& aResponse)
        {
            bool handled = false;
            if (aResource.location() == "/") {
                aResponse.body = R"(<html><head>
                                    <link rel="stylesheet" type="text/css" href="css/acmacs-api-client.css">
                                    <script src="/js/acmacs-api-client.js"></script>
                                    <script src="/js/lib/md5.js"></script>)";
                aResponse.body += "<script>ARGV = " + json_writer::compact_json(aResource.argv()) + "</script>";
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
        auto thread_maker = [&settings](Wspp& aWspp) -> WsppThread* {
            return new WsppThreadWithMongoAccess{aWspp, settings.mongodb_uri()};
        };
        Wspp wspp{settings, thread_maker};
        sWspp = &wspp;

        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        std::signal(SIGQUIT, signal_handler);

        wspp.add_location_handler(std::make_shared<RootPage>());
        wspp.add_location_handler(std::make_shared<AcmacsAPIServer>(command_factory));

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

AcmacsAPIServer::~AcmacsAPIServer()
{
      // print_cerr("~AcmacsAPIServer ", this);

} // AcmacsAPIServer::~AcmacsAPIServer

// ----------------------------------------------------------------------

void AcmacsAPIServer::message(std::string aMessage, WsppThread& aThread)
{
    auto& thread = dynamic_cast<WsppThreadWithMongoAccess&>(aThread);

      // print_cerr("MSG: ", aMessage.substr(0, 80));
    using namespace std::placeholders;
    auto command = mCommandFactory.find(aMessage, thread.client()["acmacs_web"], session(thread.client()["acmacs_web"]), std::bind(&AcmacsAPIServer::send, this, _1, _2));
    try {
        command->run();
    }
    catch (std::exception& err) {
        command->send_error(err.what());
    }

} // AcmacsAPIServer::message

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
