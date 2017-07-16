#include <iostream>
#include <getopt.h>

#include "acmacs-base/stream.hh"

#include "acmacs-webserver/server.hh"
#include "acmacs-webserver/server-settings.hh"

#include "md5.hh"
#include "mongo-access.hh"
#include "session.hh"

// ----------------------------------------------------------------------

class RootPage : public WsppHttpLocationHandler
{
 public:
    virtual inline bool handle(std::string aLocation, WsppHttpResponseData& aResponse)
        {
            bool handled = false;
            if (aLocation == "/") {
                aResponse.body = std::string{"<html><head><script src=\"/js/acmacs-api-client.js\"></script></head><body><h1>acmacs-api-server</h1></body></html>"};
                handled = true;
            }
            return handled;
        }

}; // class RootPage

// ----------------------------------------------------------------------

class AcmacsAPIServer : public WsppWebsocketLocationHandler
{
 public:
    inline AcmacsAPIServer() : WsppWebsocketLocationHandler{} {}
    inline AcmacsAPIServer(const AcmacsAPIServer& aSrc) : WsppWebsocketLocationHandler{aSrc} {}

 protected:
    virtual std::shared_ptr<WsppWebsocketLocationHandler> clone() const
        {
            return std::make_shared<AcmacsAPIServer>(*this);
        }

    virtual inline bool use(std::string aLocation) const
        {
            return aLocation == "/api";
        }

    virtual inline void opening(std::string)
        {
            send("{\"hello\": \"acmacs-api-server-v1\"}");
        }

    virtual inline void message(std::string aMessage)
        {
            std::cerr << std::this_thread::get_id() << " message: \"" << aMessage << '"' << std::endl;
            send("{\"E\": \"unrecognized message\"}", websocketpp::frame::opcode::text);
        }

    virtual void after_close(std::string)
        {
              //std::cout << std::this_thread::get_id() << " MyWS after_close" << std::endl;
        }

}; // class MyWS

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    if (argc != 2 || std::string{"-h"} == argv[1] || std::string{"--help"} == argv[1]) {
        std::cerr << "Usage: " << argv[0] << " <settings.json>" << std::endl;
        return 1;
    }

    ServerSettings settings;
    settings.read_from_file(argv[1]);
    Wspp wspp{settings};

    wspp.add_location_handler(std::make_shared<RootPage>());
    wspp.add_location_handler(std::make_shared<AcmacsAPIServer>());

    wspp.run();

    return 0;

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
