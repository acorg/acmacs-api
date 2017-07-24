#include <iostream>
#include <memory>
#include <getopt.h>
#include <atomic>
#include <csignal>

#include "acmacs-webserver/server-settings.hh"

#include "acmacs-api-server.hh"
#include "command.hh"
#include "command-factory.hh"

// ----------------------------------------------------------------------

static std::atomic<Wspp*> sWspp;
[[noreturn]] static void signal_handler(int signal);

// ----------------------------------------------------------------------

class AcmacsAPISettings : public ServerSettings
{
 public:
    inline auto mongodb_uri() const
        {
            auto uri = json_importer::get(mDoc, "mongodb_uri", std::string{});
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
                aResponse.body += "<script>ARGV = " + json_writer::compact_json(aResource.argv(), "argv") + "</script>";
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
        CommandFactory command_factory;

        AcmacsAPISettings settings;
        settings.read(argv[1]);
        Wspp wspp{settings};
        sWspp = &wspp;

        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        std::signal(SIGQUIT, signal_handler);

        mongocxx::instance inst{};
        std::cerr << "mongodb_uri: [" << settings.mongodb_uri() << "]" << std::endl;
        mongocxx::pool pool{mongocxx::uri{settings.mongodb_uri()}};

        wspp.add_location_handler(std::make_shared<RootPage>());
        wspp.add_location_handler(std::make_shared<AcmacsAPIServer>(pool, command_factory));

        wspp.run();
        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << std::endl;
        return 1;
    }

      // std::string hostname{"localhost"}, port{"1169"};
      // const char* const short_opts = "x:p:h";
      // const option long_opts[] = {
      //     {"host", required_argument, nullptr, 'x'},
      //     {"port", required_argument, nullptr, 'p'},
      //     {"help", no_argument, nullptr, 'h'},
      //     {nullptr, no_argument, nullptr, 0}
      // };
      // int opt;
      // while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
      //     switch (opt) {
      //       case 'x':
      //           hostname = optarg;
      //           break;
      //       case 'p':
      //           port = optarg;
      //           break;
      //       case 'h':
      //           std::cerr << "Usage: " << argv[0] << " [--host|-x <hostname>] [--port|-p <port>]" << std::endl;
      //           return 0;
      //       default:
      //           break;
      //     }
      // }
      // argc -= optind;
      // argv += optind;

      // Wspp wspp{hostname, port, 3 /* std::thread::hardware_concurrency() */, "/Users/eu/AD/sources/acmacs-webserver/ssl/self-signed.crt", "/Users/eu/AD/sources/acmacs-webserver/ssl/self-signed.key", "/Users/eu/AD/sources/acmacs-webserver/ssl/dh.pem"};
      // wspp.setup_logging("/tmp/acmacs-api-server.access.log", "/tmp/acmacs-api-server.error.log");
      // wspp.add_location_handler(std::make_shared<RootPage>());
      // wspp.add_location_handler(std::make_shared<WsppHttpLocationHandlerFile>("/js/acmacs-api-client.js", std::vector<std::string>{"dist/acmacs-api-client.js.gz"}));
      // wspp.add_location_handler(std::make_shared<WsppHttpLocationHandlerFile>("/js/acmacs-api-client.js.gz.map", std::vector<std::string>{"dist/acmacs-api-client.js.gz.map"}));
      // wspp.add_location_handler(std::make_shared<WsppHttpLocationHandlerFile>("/favicon.ico", std::vector<std::string>{"/Users/eu/AD/sources/acmacs-webserver/favicon.ico"}));
      // wspp.add_location_handler(std::make_shared<AcmacsAPIServer>());

      // wspp.run();
      // return 0;
}

// ----------------------------------------------------------------------

[[noreturn]] static void signal_handler(int signal)
{
    std::cerr << "Interrupted with signal " << signal << std::endl;
    sWspp.load()->stop_listening();
    std::exit(-signal);
}

// ----------------------------------------------------------------------

void AcmacsAPIServer::message(std::string aMessage)
{
    std::cerr << std::this_thread::get_id() << " MSG: " << aMessage.substr(0, 80) << std::endl;
    auto command = mCommandFactory.find(aMessage, *this, ++mCommandNumber);
    try {
        command->run();
    }
    catch (std::exception& err) {
        send(json_object("C", command->command_name(), "CN", command->command_number(), "E", err.what()));
    }

} // AcmacsAPIServer::message

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
