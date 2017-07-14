#include <iostream>
#include <getopt.h>

#include "acmacs-base/stream.hh"

#include "md5.hh"
#include "mongo-access.hh"
#include "session.hh"

// ----------------------------------------------------------------------

void parse_command_line(int argc, char* const argv[], Session& aSession, std::vector<std::vector<std::string>>& aCommands);

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        mongocxx::instance inst{};
        mongocxx::pool pool{mongocxx::uri{}};
        auto conn = pool.acquire(); // shared_ptr<mongocxx::client>
        auto db = (*conn)["acmacs_web"]; // mongocxx::database
        Session session(db);

        std::vector<std::vector<std::string>> commands;
        parse_command_line(argc, argv, session, commands);
        if (!commands.empty()) {
            std::cerr << commands << std::endl;
        }
        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << std::endl;
        return 1;
    }
}

// ----------------------------------------------------------------------

void parse_command_line(int argc, char* const argv[], Session& aSession, std::vector<std::vector<std::string>>& aCommands)
{
    std::string user, password, session;
    const char* const short_opts = "u:p:s:h";
    const option long_opts[] = {
        {"user", required_argument, nullptr, 'u'},
        {"password", required_argument, nullptr, 'p'},
        {"session", required_argument, nullptr, 's'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, no_argument, nullptr, 0}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
        switch (opt) {
          case 'u':
              user = optarg;
              break;
          case 'p':
              password = optarg;
              break;
          case 's':
              session = optarg;
              break;
          case 'h':
              std::cerr << "Usage: " << argv[0] << " [--user|-u <username>] [--password|-p <password>] [--session|-s <session-id>] <command> [arg ...] [-- <command> [arg ...]] ..." << std::endl;
              aCommands.clear();
              return;
          default:
              break;
        }
    }
    argc -= optind;
    argv += optind;

    if (user.empty() && session.empty())
        throw std::runtime_error{"Please pass login or session"};
    if (argc < 1)
        throw std::runtime_error{"command(s) expected in the command line"};

      // login
    if (!session.empty()) {
        try {
            aSession.use_session(session);
        }
        catch (std::exception& err) {
            if (user.empty())
                throw std::runtime_error{std::string{"invalid session-id: "} + err.what()};
            else
                std::cerr << "Invalid session: " << err.what() << std::endl;
        }
    }
    if (aSession.id().empty() && !user.empty()) {
        aSession.login(user, password);
        std::cout << "--session " << aSession.id() << std::endl;
    }

      // extract commands
    aCommands.emplace_back();
    for (; argc > 0; --argc, ++argv) {
        if (std::string{argv[0]} == "--") {
            if (aCommands.back().empty())
                throw std::runtime_error{"empty command in the command line"};
            aCommands.emplace_back();
        }
        else {
            aCommands.back().emplace_back(argv[0]);
        }
    }
    if (aCommands.back().empty())
        throw std::runtime_error{"empty command in the command line"};
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
