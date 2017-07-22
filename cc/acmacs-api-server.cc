#include <iostream>
#include <memory>
#include <getopt.h>
#include <atomic>
#include <csignal>

#include "acmacs-base/stream.hh"
#include "acmacs-base/rapidjson.hh"

#include "acmacs-webserver/server.hh"
#include "acmacs-webserver/server-settings.hh"

#include "md5.hh"
#include "mongo-access.hh"
#include "session.hh"

// ----------------------------------------------------------------------

class JOKV : public std::pair<std::string, std::string>                     // json object key value
{
 public:
    inline JOKV(std::string key, std::string value) : std::pair<std::string, std::string>(key, value) {}
};

class JOS : public std::string  // json object string
{
 public:
    inline JOS() : std::string{"{}"} {}
    inline JOS& operator << (JOKV&& key_value)
        {
            size_t pos = size() - 1;
            if (size() > 2) {
                insert(pos, ",");
                ++pos;
            }
            insert(pos, "\"" + key_value.first + "\":\"" + key_value.second + "\"");
            return *this;
        }
};

class JOE : public std::string  // json object error
{
 public:
    inline JOE(std::string aMessage) : std::string{"{\"E\":\"" + aMessage + "\"}"} {}
};

// ----------------------------------------------------------------------

class RootPage : public WsppHttpLocationHandler
{
 public:
    virtual inline bool handle(const HttpResource& aResource, WsppHttpResponseData& aResponse)
        {
            bool handled = false;
            if (aResource.location() == "/") {
                aResponse.body = R"(<html><head><script src="/js/acmacs-api-client.js"></script><script src="/js/lib/md5.js"></script>)";
                aResponse.body += "<script>ARGV = " + json_writer::compact_json(aResource.argv(), "argv") + "</script>";
                aResponse.body += "</head><body><h1>acmacs-api-server</h1></body></html>";
                handled = true;
            }
            return handled;
        }

}; // class RootPage

// ----------------------------------------------------------------------

class AcmacsAPIServer;

class Command : public json_importer::Object
{
 public:
    inline Command(json_importer::Object&& aSrc, AcmacsAPIServer& aServer) : json_importer::Object{std::move(aSrc)}, mServer{aServer} {}

    inline std::string command_name() const { return get_string("C"); }

    virtual void run() = 0;

 protected:
    void send(std::string aMessage, websocketpp::frame::opcode::value op_code = websocketpp::frame::opcode::text);
    mongocxx::database db();
    Session& session();

 private:
    AcmacsAPIServer& mServer;

}; // class Command

// ----------------------------------------------------------------------

class Command_unknown : public Command
{
 public:
    using Command::Command;

    virtual inline void run()
        {
            send(R"({"E": "unrecognized message"})");
        }

}; // class Command_users

// ----------------------------------------------------------------------

class Command_users : public Command
{
 public:
    using Command::Command;

    virtual void run();

}; // class Command_users

// ----------------------------------------------------------------------

// class Command_login : public Command
// {
//  public:
//     using Command::Command;

//     virtual void run();

//     inline std::string session_id() const { return get_string("S"); }
//     inline std::string user() const { return get_string("U"); }
//     inline std::string password() const { return get_string("P"); }

// }; // class Command_users

// ----------------------------------------------------------------------

class Command_login_session : public Command
{
 public:
    using Command::Command;

    virtual void run();

    inline std::string session_id() const { return get_string("S"); }

}; // class Command_users

// ----------------------------------------------------------------------

class Command_login_nonce : public Command
{
 public:
    using Command::Command;

    virtual void run();

    inline std::string user() const { return get_string("user"); }

}; // class Command_users

// ----------------------------------------------------------------------

class Command_login_digest : public Command
{
 public:
    using Command::Command;

    virtual void run();

    inline std::string cnonce() const { return get_string("cnonce"); }
    inline std::string digest() const { return get_string("digest"); }

}; // class Command_users

// ----------------------------------------------------------------------

class CommandFactory
{
 public:
    CommandFactory();

    inline std::shared_ptr<Command> find(std::string aMessage, AcmacsAPIServer& aServer) const
        {
            json_importer::Object msg{aMessage};
            std::shared_ptr<Command> result;
            const auto found = mFactory.find(msg.get_string("C"));
            if (found != mFactory.end())
                result = (this->*found->second)(std::move(msg), aServer);
            else
                result = make<Command_unknown>(std::move(msg), aServer);
            return result;
        }

 private:
    using FactoryFunc = std::shared_ptr<Command> (CommandFactory::*)(json_importer::Object&&, AcmacsAPIServer&) const;

    template <typename Cmd> inline std::shared_ptr<Command> make(json_importer::Object&& aSrc, AcmacsAPIServer& aServer) const
        {
            return std::make_shared<Cmd>(std::move(aSrc), aServer);
        }

      // std::map<std::string, std::function<std::shared_ptr<Command> (std::string)>> mFactory;
    std::map<std::string, FactoryFunc> mFactory;

}; // class CommandFactory

CommandFactory::CommandFactory()
    : mFactory{
    {"users", &CommandFactory::make<Command_users>},
    {"login_session", &CommandFactory::make<Command_login_session>},
    {"login_nonce", &CommandFactory::make<Command_login_nonce>},
    {"login_digest", &CommandFactory::make<Command_login_digest>},
}
{
}

// ----------------------------------------------------------------------

class AcmacsAPIServer : public WsppWebsocketLocationHandler
{
 public:
    inline AcmacsAPIServer(mongocxx::pool& aPool, CommandFactory& aCommandFactory)
        : WsppWebsocketLocationHandler{}, mPool{aPool}, mCommandFactory{aCommandFactory}, mSession{db()} {}
    inline AcmacsAPIServer(const AcmacsAPIServer& aSrc)
        : WsppWebsocketLocationHandler{aSrc}, mPool{aSrc.mPool}, mCommandFactory{aSrc.mCommandFactory}, mSession{aSrc.mSession} {}

 protected:
    inline auto connection()
        {
            if (!mConnection)
                mConnection = mPool.acquire();
            return mConnection;
        }

    inline mongocxx::database db(const char* aName)
        {
            return (*connection())[aName];
        }

    inline mongocxx::database& db()
        {
            if (!mAcmacsWebDb)
                mAcmacsWebDb = db("acmacs_web");
            return mAcmacsWebDb;
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
            std::cerr << std::this_thread::get_id() << " MSG: " << aMessage.substr(0, 80) << std::endl;
            auto command = mCommandFactory.find(aMessage, *this);
            command->run();

            // json_importer::Object msg{aMessage};
            // auto command = msg.get_string("C");

            // if (command == "echo") {
            //     send(aMessage);
            // }
            // else if (command == "users") {
            //     try {
            //         auto acmacs_web_db = db();
            //         DocumentFindResults results{acmacs_web_db, "users_groups",
            //                     (DocumentFindResults::bson_doc{} << "_t" << "acmacs.mongodb_collections.users_groups.User"
            //                        // << bsoncxx::builder::concatenate(aSession.read_permissions().view())
            //                      << DocumentFindResults::bson_finalize),
            //                     MongodbAccess::exclude{"_id", "_t", "_m", "password", "nonce"}};
            //         send("{\"R\": " + results.json() + "}");
            //     }
            //     catch (DocumentFindResults::Error& err) {
            //         send(std::string{"{\"E\": \""} + err.what() + "\"}");
            //     }
            // }
            // else {
            //     send(R"({"E": "unrecognized message"})", websocketpp::frame::opcode::text);
            // }
        }

    virtual void after_close(std::string)
        {
              //std::cout << std::this_thread::get_id() << " MyWS after_close" << std::endl;
        }

 private:
    mongocxx::pool& mPool;
    mongocxx::database mAcmacsWebDb;
    CommandFactory& mCommandFactory;
    std::shared_ptr<mongocxx::client> mConnection;
    Session mSession;

    inline Session& session() { return mSession; }

    friend class Command;

}; // class AcmacsAPIServer

// ----------------------------------------------------------------------

Session& Command::session()
{
    return mServer.session();
}

// ----------------------------------------------------------------------

void Command::send(std::string aMessage, websocketpp::frame::opcode::value op_code)
{
    std::cerr << "Command::send: " << aMessage << std::endl;
    mServer.send(aMessage, op_code);

} // Command::send

// ----------------------------------------------------------------------

mongocxx::database Command::db()
{
    return mServer.db();

} // Command::db

// ----------------------------------------------------------------------

void Command_users::run()
{
    try {
        auto acmacs_web_db = db();
        DocumentFindResults results{acmacs_web_db, "users_groups",
                    (DocumentFindResults::bson_doc{} << "_t" << "acmacs.mongodb_collections.users_groups.User"
                       // << bsoncxx::builder::concatenate(aSession.read_permissions().view())
                     << DocumentFindResults::bson_finalize),
                    MongodbAccess::exclude{"_id", "_t", "_m", "password", "nonce"}};
          // send(JOS{} << JOKV{"E", "who cares?"});
        send(results.json(false, "R"));
    }
    catch (DocumentFindResults::Error& err) {
        send(JOE(err.what())); // std::string{"{\"E\": \""} + err.what() + "\"}");
    }

} // Command_users::run

// ----------------------------------------------------------------------

// void Command_login::run()
// {
//     try {
//         const auto session_id_ = session_id();
//         const auto username = user();
//         if (!session_id_.empty()) {
//             try {
//                 session().use_session(session_id_);
//             }
//             catch (std::exception& err) {
//                 if (username.empty())
//                     throw std::runtime_error{std::string{"invalid session-id: "} + err.what()};
//                 else
//                     std::cerr << "Invalid session: " << err.what() << std::endl;
//             }
//         }
//         // if (aSession.id().empty() && !username.empty()) {
//         //     // // aSession.login(user, password);
//         //     // // std::cout << "--session " << aSession.id() << std::endl;
//         // }
//           //  send(std::string{"{\"R\":\"session\",\"S\":\"" + session().id() + "\"}"});
//         send(JOS{} << JOKV{"R", "session"} << JOKV{"S", session().id()} << JOKV{"user", session().user()} << JOKV{"display_name", session().display_name()});
//     }
//     catch (std::exception& err) {
//         send(JOE(err.what())); // send(std::string{"{\"E\": \""} + err.what() + "\"}");
//     }

// } // Command_login::run

// ----------------------------------------------------------------------

void Command_login_session::run()
{
    try {
        session().use_session(session_id());
        send(JOS{} << JOKV{"R", "session"} << JOKV{"S", session().id()} << JOKV{"user", session().user()} << JOKV{"display_name", session().display_name()});
    }
    catch (std::exception& err) {
        send(JOE(err.what()));
    }

} // Command_login_session::run

// ----------------------------------------------------------------------

void Command_login_nonce::run()
{
    try {
        const auto nonce = session().login_nonce(user());
        send(JOS{} << JOKV{"R", "login_nonce"} << JOKV{"login_nonce", nonce});
    }
    catch (std::exception& err) {
        send(JOE(err.what()));
    }

} // Command_login_nonce::run

// ----------------------------------------------------------------------

void Command_login_digest::run()
{
    try {
        session().login_with_password_digest(cnonce(), digest());
        send(JOS{} << JOKV{"R", "session"} << JOKV{"S", session().id()} << JOKV{"user", session().user()} << JOKV{"display_name", session().display_name()});
    }
    catch (std::exception& err) {
        send(JOE(err.what()));
    }

} // Command_login_digest::run

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

static std::atomic<Wspp*> sWspp;

[[noreturn]] inline static void signal_handler(int signal)
{
    std::cerr << "Interrupted with signal " << signal << std::endl;
    sWspp.load()->stop_listening();
    std::exit(-signal);
}

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
