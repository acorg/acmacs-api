#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <set>
#include <iterator> // make_ostream_joiner

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/stream.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/iterator.hh"
#include "acmacs-base/float.hh"

#include "bson-to-json.hh"

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

inline auto projection_to_exclude_fields(std::initializer_list<std::string>&& fields)
{
    auto proj_doc = bsoncxx::builder::stream::document{};
    for (auto field: fields)
        proj_doc << field << false;
    return proj_doc << bsoncxx::builder::stream::finalize;
}


// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

class SessionError : public std::runtime_error { public: using std::runtime_error::runtime_error; };

class Session
{
 public:
    inline Session(mongocxx::database& aDb) : mDb(aDb) {}
    void use_session(std::string aSessionId); // throws SessionError
    std::string get_nonce(std::string aUser);
    void login(std::string aUser, std::string aCNonce, std::string aPasswordDigest);

    inline std::string id() const { return mId; }
    inline std::string user() const { return mUser; }
    inline const std::vector<std::string>& groups() const { return mGroups; }

 private:
    mongocxx::database& mDb;
    std::string mId;
    std::string mUser;
    std::vector<std::string> mGroups;

    void find_user(std::string aUser);

    using document = bsoncxx::builder::stream::document;
    static constexpr auto finalize = bsoncxx::builder::stream::finalize;

}; // class Session

// ----------------------------------------------------------------------

void Session::use_session(std::string aSessionId)
{
    mId.clear();
    mUser.clear();
    mGroups.clear();

    auto filter = bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid{aSessionId} << bsoncxx::builder::stream::finalize;
    auto options = mongocxx::options::find{};
    options.projection(projection_to_exclude_fields({"_t", "_m", "I", "expires", "expiration_in_seconds", "commands"}));
    auto found = mDb["sessions"].find_one(std::move(filter), options);
    if (!found)
        throw SessionError{"invalid session"};
    for (auto entry: found->view()) {
        const std::string key = entry.key().to_string();
        if (key == "_id")
            mId = entry.get_value().get_oid().value.to_string();
        else if (key == "user")
            mUser = entry.get_value().get_utf8().value.to_string();
        else if (key == "user_and_groups") {
            const auto array = entry.get_value().get_array().value;
            std::transform(array.begin(), array.end(), std::back_inserter(mGroups), [](const auto& name) -> std::string { return name.get_utf8().value.to_string(); });
        }
    }

} // Session::use_session

// ----------------------------------------------------------------------

void Session::find_user(std::string aUser)
{
    auto filter = document{} << "name" << aUser << "_t" << "acmacs.mongodb_collections.users_groups.User" << finalize;
    auto options = mongocxx::options::find{};
    options.projection(projection_to_exclude_fields({"_id", "_t", "recent_logins", "created", "p", "_m"}));
    auto found = mDb["users_groups"].find_one(std::move(filter), options);
    if (!found)
        throw SessionError{"invalid user or password"};
    std::cout << json_writer::json(*found, "user", 1) << std::endl;

} // Session::find_user

// ----------------------------------------------------------------------

std::string Session::get_nonce(std::string aUser)
{
    find_user(aUser);
      // new_nonce();

} // Session::get_nonce

// ----------------------------------------------------------------------

void Session::login(std::string aUser, std::string aCNonce, std::string aPasswordDigest)
{

} // Session::login

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

class CommandError : public std::runtime_error { public: using std::runtime_error::runtime_error; };

class CommandBase
{
 public:
    virtual inline ~CommandBase() {}

    virtual std::string process(mongocxx::database& aDb) = 0;
    virtual void args(int /*argc*/, char* const /*argv*/[]) {}

 protected:
    using document = bsoncxx::builder::stream::document;
    static constexpr auto finalize = bsoncxx::builder::stream::finalize;

}; // class CommandBase

// ----------------------------------------------------------------------

class DocumentFindResults
{
 public:
    using document_view = bsoncxx::document::view;

    inline DocumentFindResults() {}
    inline DocumentFindResults(mongocxx::v_noabi::cursor&& aCursor) { build(std::move(aCursor)); }

    void build(mongocxx::v_noabi::cursor&& aCursor)
        {
            std::copy(std::begin(aCursor), std::end(aCursor), std::back_inserter(mRecords));
        }

    inline std::string json() const
        {
            json_writer::pretty writer{"DocumentFindResults"};
            writer << json_writer::start_object
                    << json_writer::key("results") << mRecords
                    << json_writer::end_object;
            return writer << json_writer::finalize;
        }

 private:
    std::vector<document_view> mRecords;

}; // class DocumentFindResults

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
            auto nonce = session.get_nonce(mUser);

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
            auto filter = document{} << "_t" << "acmacs.mongodb_collections.users_groups.User" << finalize;
            auto options = mongocxx::options::find{};
            options.projection(projection_to_exclude_fields({"_id", "_t", "password", "nonce"}));
            DocumentFindResults results{aDb["users_groups"].find(std::move(filter), options)};
            return results.json();
        }

}; // class CommandUsers

// ----------------------------------------------------------------------

class CommandGroups : public CommandBase
{
 public:
    virtual std::string process(mongocxx::database& aDb)
        {
            auto filter = document{} << "_t" << "acmacs.mongodb_collections.users_groups.Group" << finalize;
            auto options = mongocxx::options::find{};
            options.projection(projection_to_exclude_fields({"_id", "_t"}));
            DocumentFindResults results{aDb["users_groups"].find(std::move(filter), options)};
            return results.json();
        }

}; // class CommandGroups

// ----------------------------------------------------------------------

class CommandSessions : public CommandBase
{
 public:
    virtual std::string process(mongocxx::database& aDb)
        {
            auto filter = document{} << /* "_t" << "acmacs.mongodb_collections.users_groups.User" << */ finalize;
            auto options = mongocxx::options::find{};
              //options.projection(projection_to_exclude_fields({"_id", "_t", "password", "nonce"}));
            DocumentFindResults results{aDb["sessions"].find(std::move(filter), options)};
            return results.json();
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
