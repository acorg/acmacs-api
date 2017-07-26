#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <set>
#include <random>

#include "acmacs-base/stream.hh"
#include "acmacs-base/string.hh"

#include "md5.hh"
#include "mongo-access.hh"
#include "session.hh"

// ----------------------------------------------------------------------

class CommandError : public std::runtime_error { public: using std::runtime_error::runtime_error; };

class CommandBase
{
 public:
    virtual inline ~CommandBase() {}

    virtual std::string process(mongocxx::database& aDb) = 0;
    virtual void args(int /*argc*/, char* const /*argv*/[]) {}

}; // class CommandBase

// ----------------------------------------------------------------------

class CommandSession : public CommandBase
{
 public:
    virtual void args(int argc, char* const argv[])
        {
            if (argc != 1)
                throw CommandError{"invalid number of arguments"};
            mSessionId = argv[0];
        }

    virtual std::string process(mongocxx::database& aDb)
        {
            Session session(aDb);
            session.use_session(mSessionId);

            json_writer::pretty writer{"session"};
            writer << json_writer::start_object << json_writer::key("session")
                   << json_writer::start_object
                   << json_writer::key("session_id") << session.id()
                   << json_writer::key("user") << session.user()
                   << json_writer::key("groups") << session.groups()
                   << json_writer::end_object
                   << json_writer::end_object;
            return writer;
        }

 private:
    std::string mSessionId;

}; // class CommandSession

// ----------------------------------------------------------------------

class CommandLogin : public CommandBase
{
 public:
    virtual void args(int argc, char* const argv[])
        {
            if (argc != 2)
                throw CommandError{"invalid number of arguments"};
            mUser = argv[0];
            mPassword = argv[1];
        }

    virtual std::string process(mongocxx::database& aDb)
        {
            Session session(aDb);
            session.find_user(mUser, true);
            auto nonce = session.get_nonce();
              // login(std::string aUser, std::string aCNonce, std::string aPasswordDigest);

            std::random_device rd;
            const auto cnonce = string::to_hex_string(rd() & 0xFFFFFFFF, false);
            const auto digest = md5(mUser + ";acmacs-web;" + mPassword);
            const auto hashed_password = md5(nonce + ";" + cnonce + ";" + digest);
            session.login_with_password_digest(cnonce, hashed_password);

            json_writer::pretty writer{"session"};
            writer << json_writer::start_object << json_writer::key("session")
                   << json_writer::start_object
                   << json_writer::key("session_id") << session.id()
                   << json_writer::key("user") << session.user()
                   << json_writer::key("display_name") << session.display_name()
                   << json_writer::end_object
                   << json_writer::end_object;
            return writer;
        }

 private:
    std::string mUser;
    std::string mPassword;

}; // class CommandLogin

// ----------------------------------------------------------------------

class CommandCollections : public CommandBase
{
 public:
    virtual std::string process(mongocxx::database& aDb)
        {
            json_writer::pretty writer{"collections"};
            writer << json_writer::start_object << json_writer::key("collections") << json_writer::start_array;
            for (auto doc: aDb.list_collections())
                writer << doc["name"].get_utf8().value.to_string();
            writer << json_writer::end_array << json_writer::end_object;
            return writer;
        }

}; // class CommandCollections

// ----------------------------------------------------------------------

class CommandUsers : public CommandBase
{
 public:
    virtual std::string process(mongocxx::database& aDb)
        {
            DocumentFindResults results{aDb, "users_groups",
                        bson_make_value("_t", "acmacs.mongodb_collections.users_groups.User"),
                        DocumentFindResults::exclude{"_id", "_t", "password", "nonce"}};
            return results.json();
        }

}; // class CommandUsers

// ----------------------------------------------------------------------

class CommandGroups : public CommandBase
{
 public:
    virtual std::string process(mongocxx::database& aDb)
        {
            DocumentFindResults results{aDb, "users_groups",
                        bson_make_value("_t", "acmacs.mongodb_collections.users_groups.Group"),
                        DocumentFindResults::exclude{"_id", "_t"}};
            return results.json();
        }

}; // class CommandGroups

// ----------------------------------------------------------------------

class CommandSessions : public CommandBase
{
 public:
    virtual std::string process(mongocxx::database& aDb)
        {
            return DocumentFindResults{aDb, "sessions"}.json();
        }

}; // class CommandSessions

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

static inline std::map<std::string, std::unique_ptr<CommandBase>> make_commands()
{
    std::map<std::string, std::unique_ptr<CommandBase>> commands;
    commands.emplace("session", std::make_unique<CommandSession>());
    commands.emplace("login", std::make_unique<CommandLogin>());
    commands.emplace("collections", std::make_unique<CommandCollections>());
    commands.emplace("users", std::make_unique<CommandUsers>());
    commands.emplace("groups", std::make_unique<CommandGroups>());
    commands.emplace("sessions", std::make_unique<CommandSessions>());
    return commands;
}

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <command> [<arg> ...]" << std::endl;
        return 1;
    }

    mongocxx::instance inst{};
    mongocxx::pool pool{mongocxx::uri{}};
    auto conn = pool.acquire(); // shared_ptr<mongocxx::client>
    auto db = (*conn)["acmacs_web"]; // mongocxx::database
    auto commands = make_commands();
    auto command = commands.find(argv[1]);
    if (command != commands.end()) {
        try {
            command->second->args(argc - 2, argv + 2);
            auto result = command->second->process(db);
            std::cout << result << std::endl;
        }
        catch (CommandError& err) {
            std::cerr << "Command \"" << argv[1] << "\" error: " << err.what() << std::endl;
            return 3;
        }
    }
    else {
        std::cerr << "Unrecognized command: " << argv[1] << std::endl;
        std::cerr << " available commands:\n  ";
        std::transform(commands.begin(), commands.end(), polyfill::make_ostream_joiner(std::cerr, "\n  "), [](const auto& cmd) { return cmd.first; });
        return 2;
    }
    return 0;

    // auto collection_cursor = db.list_collections();
    // std::vector<std::string> collections;
    // std::transform(std::begin(collection_cursor), std::end(collection_cursor), std::back_inserter(collections), [](const auto& doc) { return doc["name"].get_utf8().value.to_string(); });
    // // std::cout << collections << std::endl;

    // for (const auto& collection_name: collections) {
    //     auto value_optional = db[collection_name].find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid{argv[1]} << bsoncxx::builder::stream::finalize);
    //     if (value_optional) {
    //         auto field_t = (value_optional->view())["_t"];
    //         auto field_table = (value_optional->view())["table"];
    //         if (field_t && field_t.get_utf8().value == bsoncxx::stdx::string_view{"acmacs.mongodb_collections.chart.Table"} && field_table) {
    //             auto field_binary = field_table.get_binary();
    //             std::cout << "Table " << field_binary.size << std::string{reinterpret_cast<const char*>(field_binary.bytes), 5} << /* bsoncxx::to_json(field_table.get_document().view()) <<  */std::endl;
    //         }
    //         else
    //             std::cout << collection_name << ": " << bsoncxx::to_json(*value_optional) << std::endl;
    //         break;
    //     }
    // }
}


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
