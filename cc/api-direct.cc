#include <iostream>
#include <getopt.h>

#include "acmacs-base/stream.hh"
#include "acmacs-base/to-json.hh"

#include "mongo-access.hh"
#include "session.hh"
#include "command-factory.hh"
#include "command.hh"

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
static void send(std::string aMessage, send_message_type aMessageType = send_message_type::text);

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        Args args;
        parse_command_line(argc, argv, args);

        mongocxx::instance inst{};
        mongocxx::pool pool{args.mongo_uri.empty() ? mongocxx::uri{} : mongocxx::uri{args.mongo_uri}};
        auto conn = pool.acquire(); // shared_ptr<mongocxx::client>
        auto db = (*conn)["acmacs_web"]; // mongocxx::database

        Session session{db};
        login(session, args);

        CommandFactory command_factory;
        for (const auto& command_json: args.commands) {
            auto command = command_factory.find(command_json, db, session, &send);
            try {
                command->run();
            }
            catch (std::exception& err) {
                send(to_json::object("C", command->command_name(), "CN", command->command_number(), "E", err.what()));
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

void send(std::string aMessage, send_message_type /*aMessageType*/)
{
    std::cout << "SEND: " << aMessage << std::endl;

} // send

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
