#include <iostream>
#include <getopt.h>

#include "acmacs-base/to-json-v1.hh"

#include "acmacs-api/mongo-access.hh"
#include "acmacs-api/session.hh"
#include "acmacs-api/command-factory.hh"
#include "acmacs-api/command.hh"
#include "acmacs-api/acmacs-c2.hh"
#include "acmacs-api/mongo-acmacs-c2-access.hh"

// ----------------------------------------------------------------------

struct Args
{
    std::string mongo_uri;
    std::string user;
    std::string password;
    std::string session;
    std::vector<std::string> commands;
};

static void parse_command_line(int argc, char* const argv[], Args& aArgs);
static void login(Session& aSession, Args& aArgs);

// ----------------------------------------------------------------------

class SimulatedConnection : public ClientConnection
{
 public:
    using ClientConnection::ClientConnection;

    void send(std::string aMessage, send_message_type /*aMessageType*/ = send_message_type::text) override
        {
            print_cerr("SEND: ", aMessage);
        }

    std::ostream& log_send_receive() override { return std::cerr; }

}; // class SimulatedConnection

// ----------------------------------------------------------------------

// class WsppThread {};
// class WsppThreadWithMongoAccess{};

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        Args args;
        parse_command_line(argc, argv, args);

        mongocxx::instance inst{};
        AcmacsC2 acmacs_c2;
        MongoAcmacsC2Access mongo_acmacs_c2{args.mongo_uri, acmacs_c2};
        mongo_acmacs_c2.create_client();

        SimulatedConnection client_connection;
        auto db = mongo_acmacs_c2.client()["acmacs_web"];
        client_connection.make_session(db);
        login(client_connection.session(), args);

        CommandFactory command_factory;
        for (const auto& command_json: args.commands) {
            auto command = command_factory.find(command_json, mongo_acmacs_c2, client_connection);
            try {
                command->run();
            }
            catch (std::exception& err) {
                client_connection.send(to_json::v1::object("C", command->command_name(), "CN", command->command_number(), "E", err.what()));
            }
        }
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << std::endl;
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

void parse_command_line(int argc, char* const argv[], Args& aArgs)
{
    const char* const short_opts = "u:p:s:h";
    const option long_opts[] = {
        {"user", required_argument, nullptr, 'u'},
        {"password", required_argument, nullptr, 'p'},
        {"session", required_argument, nullptr, 's'},
        {"mongo", required_argument, nullptr, 'm'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, no_argument, nullptr, 0}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
        switch (opt) {
          case 'u':
              aArgs.user = optarg;
              break;
          case 'p':
              aArgs.password = optarg;
              break;
          case 's':
              aArgs.session = optarg;
              break;
          case 'm':
              aArgs.mongo_uri = optarg;
              break;
          case 'h':
              std::cerr << "Usage: " << argv[0] << " [--user|-u <username>] [--password|-p <password>] [--session|-s <session-id>] [--mongo|-m <mongo-uri>] <command-json> ... " << std::endl;
              aArgs.commands.clear();
              return;
          default:
              break;
        }
    }
    argc -= optind;
    argv += optind;

    if (aArgs.user.empty() && aArgs.session.empty())
        throw std::runtime_error{"Please pass login or session"};
    if (argc < 1)
        throw std::runtime_error{"command(s) expected in the command line"};

    std::copy(argv, argv + argc, std::back_inserter(aArgs.commands));

} // parse_command_line

// ----------------------------------------------------------------------

void login(Session& aSession, Args& aArgs)
{
    if (!aArgs.session.empty()) {
        try {
            aSession.use_session(aArgs.session);
        }
        catch (std::exception& err) {
            if (aArgs.user.empty())
                throw std::runtime_error{std::string{"invalid session-id: "} + err.what()};
            else
                std::cerr << "Invalid session: " << err.what() << std::endl;
        }
    }
    if (!aSession.id() && !aArgs.user.empty()) {
        aSession.login(aArgs.user, aArgs.password);
        std::cout << "--session " << aSession.id() << std::endl;
    }

} // login

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
