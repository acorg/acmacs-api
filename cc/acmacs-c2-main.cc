#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>

#include "acmacs-base/to-json.hh"
#include "acmacs-c2.hh"
#include "session-id.hh"

// ----------------------------------------------------------------------

struct Args
{
    Args() : acmacs_uri{"https://localhost:1168/api"}, verbose{false} {}
    std::string acmacs_uri;
    std::string session;
    std::vector<std::string> commands;
    bool verbose;
};

static void parse_command_line(int argc, char* const argv[], Args& aArgs);

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    // const char* postthis = R"({"C":"version"})";
    int exit_code = 0;
    try {
        Args args;
        parse_command_line(argc, argv, args);

        SessionId session{args.session};
        AcmacsC2 acmacs;
        acmacs.uri(args.acmacs_uri);
        acmacs.verbose(args.verbose);
        for (const auto& command: args.commands) {
            std::cout << "==> " << command << std::endl;
            std::string response;
            if (command.substr(0, 17) == "ace_uncompressed:") { // ace_uncompressed:object-id:max-num_projections
                response = acmacs.ace_uncompressed(session, command.substr(17, 24), std::stoul(command.substr(17 + 25)));
            }
            else {
                response = rjson::to_string(acmacs.command(session, command));
            }
            std::cout << "<== " << response << std::endl;
        }
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << std::endl;
        exit_code = 1;
    }
    return exit_code;

} // main

// ----------------------------------------------------------------------

void parse_command_line(int argc, char* const argv[], Args& aArgs)
{
    const char* const short_opts = "a:s:vh";
    const option long_opts[] = {
        {"acmacs", required_argument, nullptr, 'a'},
        {"session", required_argument, nullptr, 's'},
        {"verbose", no_argument, nullptr, 'v'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, no_argument, nullptr, 0}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
        switch (opt) {
          case 'a':
              aArgs.acmacs_uri = optarg;
              break;
          case 's':
              aArgs.session = optarg;
              break;
          case 'v':
              aArgs.verbose = true;
              break;
          case 'h':
              std::cerr << "Usage: " << argv[0] << " [--session|-s <session-id>] [--acmacs|-a <acmacs-uri: https://localhost:1168/api https://acmacs-web.antigenic-cartography.org/api> <command-json> ... " << std::endl;
              aArgs.commands.clear();
              return;
          default:
              break;
        }
    }
    argc -= optind;
    argv += optind;

    if (aArgs.session.empty())
        throw std::runtime_error{"Please pass session"};
    if (argc < 1)
        throw std::runtime_error{"command(s) expected in the command line"};

    std::copy(argv, argv + argc, std::back_inserter(aArgs.commands));

} // parse_command_line

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
