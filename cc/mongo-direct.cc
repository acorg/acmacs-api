#include <iostream>
#include <getopt.h>

#include "acmacs-base/stream.hh"

#include "md5.hh"
#include "mongo-access.hh"
#include "session.hh"

// ----------------------------------------------------------------------

class CommandBase;
static void parse_command_line(int argc, char* const argv[], Session& aSession, std::vector<std::vector<std::string>>& aCommands);
static std::map<std::string, std::unique_ptr<CommandBase>> make_commands();

// ----------------------------------------------------------------------

class CommandBase
{
 public:
    using bson_doc = bsoncxx::builder::stream::document;
    static constexpr const auto bson_finalize = bsoncxx::builder::stream::finalize;
    static constexpr const auto bson_open_document = bsoncxx::builder::stream::open_document;
    static constexpr const auto bson_close_document = bsoncxx::builder::stream::close_document;
    static constexpr const auto bson_open_array = bsoncxx::builder::stream::open_array;
    static constexpr const auto bson_close_array = bsoncxx::builder::stream::close_array;
    static constexpr const auto bson_null = bsoncxx::types::b_null{};

    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    virtual inline ~CommandBase() {}

    virtual std::string process(Session& aSession) = 0;
    virtual inline void check_permissions(Session&) {}
    virtual inline void args(const std::vector<std::string>& args) { if (args.size() != 1) throw Error{"too many arguments for the command"}; } // args[0] is a command name

}; // class CommandBase

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        mongocxx::instance inst{};
        mongocxx::pool pool{mongocxx::uri{}};
        auto conn = pool.acquire(); // shared_ptr<mongocxx::client>
        auto db = (*conn)["acmacs_web"]; // mongocxx::database
        Session session(db);

        std::vector<std::vector<std::string>> commands;
        parse_command_line(argc, argv, session, commands);
        if (!commands.empty()) {
            auto command_processors = make_commands();
            for (const auto& command: commands) {
                auto command_processor = command_processors.find(command[0]);
                if (command_processor != command_processors.end()) {
                    try {
                        auto& cmd = *command_processor->second;
                        cmd.args(command);
                        cmd.check_permissions(session);
                        auto result = cmd.process(session);
                        std::cout << result << std::endl;
                    }
                    catch (CommandBase::Error& err) {
                        std::cerr << "Command " << command << " error: " << err.what() << std::endl;
                        return 3;
                    }

                }
                else {
                    std::cerr << "Unrecognized command: " << command << std::endl;
                    std::cerr << " available commands:\n  ";
                    std::transform(command_processors.begin(), command_processors.end(), polyfill::make_ostream_joiner(std::cerr, "\n  "), [](const auto& cmd) { return cmd.first; });
                    exit_code = 2;
                    break;
                }
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
              std::cerr << "Usage: " << argv[0] << " [--user|-u <username>] [--password|-p <password>] [--session|-s <session-id>] <command> [arg ...] [/ <command> [arg ...]] ..." << std::endl;
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
        if (std::string{argv[0]} == "/") {
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

class PrivilegedCommand : public CommandBase
{
 public:
    virtual inline void check_permissions(Session& aSession)
        {
            if (!aSession.is_admin())
                throw Error{"permissions denied"};
        }

}; // class PivelegedCommand

// ----------------------------------------------------------------------

class CommandUsers : public PrivilegedCommand
{
 public:
    virtual std::string process(Session& aSession)
        {
            DocumentFindResults results{aSession.db(), "users_groups",
                        (bson_doc{} << "_t" << "acmacs.mongodb_collections.users_groups.User"
                         << bsoncxx::builder::concatenate(aSession.read_permissions().view()) << bson_finalize),
                        MongodbAccess::exclude{"_id", "_t", "_m", "password", "nonce"}};
            return results.json();
        }

}; // class CommandUsers

// ----------------------------------------------------------------------

class CommandGroups : public PrivilegedCommand
{
 public:
    virtual std::string process(Session& aSession)
        {
            DocumentFindResults results{aSession.db(), "users_groups",
                        (bson_doc{} << "_t" << "acmacs.mongodb_collections.users_groups.Group"
                         << bsoncxx::builder::concatenate(aSession.read_permissions().view()) << bson_finalize),
                        MongodbAccess::exclude{"_id", "_t", "_m"}};
            return results.json();
        }

}; // class CommandGroups

// ----------------------------------------------------------------------

class CommandCharts : public CommandBase
{
 public:
    virtual std::string process(Session& aSession)
        {
            DocumentFindResults results{aSession.db(), "charts",
                        (bson_doc{} <<
                         "$or" << bson_open_array
                         << bson_open_document << "parent" << bson_open_document << "$exists" << false << bson_close_document << bson_close_document
                         << bson_open_document << "parent" << bson_open_document << "$eq" << bsoncxx::types::b_null{} << bson_close_document << bson_close_document
                         << bson_close_array
                         << bsoncxx::builder::concatenate(aSession.read_permissions().view()) << bson_finalize),
                        MongodbAccess::exclude{"_id", "_t", "table", "conformance", "search"}};
            return results.json();
        }

}; // class CommandCharts

// ----------------------------------------------------------------------

static inline std::map<std::string, std::unique_ptr<CommandBase>> make_commands()
{
    std::map<std::string, std::unique_ptr<CommandBase>> commands;
    commands.emplace("charts", std::make_unique<CommandCharts>());
    commands.emplace("users", std::make_unique<CommandUsers>());
    commands.emplace("groups", std::make_unique<CommandGroups>());
    // commands.emplace("session", std::make_unique<CommandSession>());
    // commands.emplace("login", std::make_unique<CommandLogin>());
    // commands.emplace("collections", std::make_unique<CommandCollections>());
    // commands.emplace("sessions", std::make_unique<CommandSessions>());
    return commands;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
