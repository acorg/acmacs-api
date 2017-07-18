#include <iostream>
#include <memory>
#include <getopt.h>

#include "acmacs-base/stream.hh"
#include "acmacs-base/rapidjson.hh"

#include "acmacs-webserver/server.hh"
#include "acmacs-webserver/server-settings.hh"

#include "md5.hh"
#include "mongo-access.hh"
#include "session.hh"

// ----------------------------------------------------------------------

class RootPage : public WsppHttpLocationHandler
{
 public:
    virtual inline bool handle(const HttpResource& aResource, WsppHttpResponseData& aResponse)
        {
            bool handled = false;
            if (aResource.location() == "/") {
                aResponse.body = R"(<html><head><script src="/js/acmacs-api-client.js"></script>)";
                aResponse.body += "<script>ARGV = " + json_writer::compact_json(aResource.argv(), "argv") + "</script>";
                aResponse.body += "</head><body><h1>acmacs-api-server</h1></body></html>";
                handled = true;
            }
            return handled;
        }

}; // class RootPage

// ----------------------------------------------------------------------

class AcmacsAPIServer : public WsppWebsocketLocationHandler
{
 public:
    inline AcmacsAPIServer(mongocxx::pool& aPool) : WsppWebsocketLocationHandler{}, mPool{aPool} {}
    inline AcmacsAPIServer(const AcmacsAPIServer& aSrc) : WsppWebsocketLocationHandler{aSrc}, mPool{aSrc.mPool} {}

 protected:
    inline auto connection()
        {
            if (!mConnection)
                mConnection = mPool.acquire();
            return mConnection;
        }

    inline auto db(const char* aName = "acmacs_web")
        {
            return (*connection())[aName];
        }

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
            send(R"({"hello": "acmacs-api-server-v1"})");
        }

    virtual inline void message(std::string aMessage)
        {
              // std::cerr << std::this_thread::get_id() << " message: \"" << aMessage << '"' << std::endl;
            rapidjson::Document msg;
            msg.Parse(aMessage.c_str(), aMessage.size());
            auto command = get<std::string>(msg, "C");
            if (command == "echo") {
                send(aMessage);
            }
            else if (command == "users") {
                try {
                    auto acmacs_web_db = db();
                    DocumentFindResults results{acmacs_web_db, "users_groups",
                                (DocumentFindResults::bson_doc{} << "_t" << "acmacs.mongodb_collections.users_groups.User"
                                   // << bsoncxx::builder::concatenate(aSession.read_permissions().view())
                                 << DocumentFindResults::bson_finalize),
                                MongodbAccess::exclude{"_id", "_t", "_m", "password", "nonce"}};
                    send("{\"R\": " + results.json() + "}");
                }
                catch (DocumentFindResults::Error& err) {
                    send(std::string{"{\"E\": \""} + err.what() + "\"}");
                }
            }
            else {
                send(R"({"E": "unrecognized message"})", websocketpp::frame::opcode::text);
            }
        }

    virtual void after_close(std::string)
        {
              //std::cout << std::this_thread::get_id() << " MyWS after_close" << std::endl;
        }

 private:
    mongocxx::pool& mPool;
    std::shared_ptr<mongocxx::client> mConnection;

}; // class AcmacsAPIServer

// ----------------------------------------------------------------------

class AcmacsAPISettings : public ServerSettings
{
 public:
    inline auto mongodb_uri() const
        {

            auto uri = get(mDoc, "mongodb_uri", std::string{});
            if (uri.empty())
                uri = "mongodb://localhost:27017/";
            return uri;
        }
};

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    if (argc != 2 || std::string{"-h"} == argv[1] || std::string{"--help"} == argv[1]) {
        std::cerr << "Usage: " << argv[0] << " <settings.json>" << std::endl;
        return 1;
    }

    AcmacsAPISettings settings;
    settings.read(argv[1]);
    Wspp wspp{settings};

    mongocxx::instance inst{};
    std::cerr << "mongodb_uri: [" << settings.mongodb_uri() << "]" << std::endl;
    mongocxx::pool pool{mongocxx::uri{settings.mongodb_uri()}};

    wspp.add_location_handler(std::make_shared<RootPage>());
    wspp.add_location_handler(std::make_shared<AcmacsAPIServer>(pool));

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
