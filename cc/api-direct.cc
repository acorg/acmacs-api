#include <iostream>
#include <getopt.h>

#include "acmacs-base/stream.hh"

#include "md5.hh"
#include "mongo-access.hh"
#include "session.hh"

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
    if (aSession.id().empty() && !aArgs.user.empty()) {
        aSession.login(aArgs.user, aArgs.password);
        std::cout << "--session " << aSession.id() << std::endl;
    }

} // login

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
