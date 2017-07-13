#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <set>
//#include <iterator> // make_ostream_joiner
#include <random>

#include <openssl/md5.h>

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/exception/exception.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/stream.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/iterator.hh"
#include "acmacs-base/float.hh"
#include "acmacs-base/time.hh"

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

inline auto projection_to_include_fields(std::initializer_list<std::string>&& fields, bool aExcludeId = false)
{
    auto proj_doc = bsoncxx::builder::stream::document{};
    for (auto field: fields)
        proj_doc << field << true;
    if (aExcludeId)
        proj_doc << "_id" << false;
    return proj_doc << bsoncxx::builder::stream::finalize;
}

// ----------------------------------------------------------------------

inline std::string md5(std::string aSource)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char *>(aSource.c_str()), aSource.size(), digest);
    std::ostringstream os;
    os << std::hex << std::setfill('0') << std::nouppercase;
    for(auto c: digest)
        os << std::setw(2) << static_cast<long long>(c);
      // std::cerr << "md5 " << aSource << " --> " << os.str() << std::endl;
    return os.str();
}

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
// ----------------------------------------------------------------------

class SessionError : public std::runtime_error { public: using std::runtime_error::runtime_error; };

class Session
{
 public:
    inline Session(mongocxx::database& aDb) : mDb(aDb) {}
    void use_session(std::string aSessionId); // throws SessionError
    void find_user(std::string aUser, bool aGetPassword);
    std::string get_nonce();
    void login(std::string aCNonce, std::string aPasswordDigest);

    inline std::string id() const { return mId; }
    inline std::string user() const { return mUser; }
    inline std::string display_name() const { return mDisplayName; }
    inline const std::vector<std::string>& groups() const { return mGroups; }

 private:
    mongocxx::database& mDb;
    std::string mId;
    std::string mUser;
    std::string mDisplayName;
    std::string mPassword;
    std::vector<std::string> mGroups;
    std::string mNonce;

    using document = bsoncxx::builder::stream::document;
    static constexpr auto finalize = bsoncxx::builder::stream::finalize;

    void create_session();
    void find_groups_of_user();
    void save(size_t expiration_in_seconds);

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

void Session::find_user(std::string aUser, bool aGetPassword)
{
    auto filter = document{} << "name" << aUser << "_t" << "acmacs.mongodb_collections.users_groups.User" << finalize;
    auto options = mongocxx::options::find{};
    options.projection(projection_to_exclude_fields({"_id", "_t", "recent_logins", "created", "p", "_m"}));
    auto found = mDb["users_groups"].find_one(std::move(filter), options);
    if (!found)
        throw SessionError{"invalid user or password"};
    std::cout << json_writer::json(*found, "user", 1) << std::endl;
    for (auto entry: found->view()) {
        const std::string key = entry.key().to_string();
        if (key == "name")
            mUser = entry.get_value().get_utf8().value.to_string();
        else if (aGetPassword && key == "password")
            mPassword = entry.get_value().get_utf8().value.to_string();
        else if (key == "display_name")
            mDisplayName = entry.get_value().get_utf8().value.to_string();
    }

} // Session::find_user

// ----------------------------------------------------------------------

std::string Session::get_nonce()
{
    std::random_device rd;
    mNonce = string::to_hex_string(rd() & 0xFFFFFFFF, false);
    return mNonce;

} // Session::get_nonce

// ----------------------------------------------------------------------

void Session::login(std::string aCNonce, std::string aPasswordDigest)
{
    const auto hashed_password = md5(mNonce + ";" + aCNonce + ";" + mPassword);
    if (aPasswordDigest != hashed_password)
        throw SessionError{"invalid user or password"};
    create_session();
      //! user.add_recent_login(session=session)

} // Session::login

// ----------------------------------------------------------------------

void Session::create_session()
{
    const size_t expiration_in_seconds = 3600; // mDb["configuration"] system.sessions.expiration_in_seconds
    find_groups_of_user();
      // std::cerr << "Groups: " << mGroups << std::endl;
    save(expiration_in_seconds);

    // auto filter = bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid{aSessionId} << bsoncxx::builder::stream::finalize;
    // auto options = mongocxx::options::find{};
    // options.projection(projection_to_exclude_fields({"_t", "_m", "I", "expires", "expiration_in_seconds", "commands"}));
    // auto found = mDb["configuration"].find_one(std::move(filter), options);
        // session = Session(user=user, user_and_groups=mongodb_collections.users_groups.find_groups_of_user(session=guest_session(), user=user) | {user}, I=ip_address, changed_user=changed_user, commands=0)
        // if not expiration_in_seconds:
        //     expiration_in_seconds = mongodb_collections.configuration.find_setting(session=session, name='system.sessions.expiration_in_seconds')
        // session.expiration_in_seconds = expiration_in_seconds
        // session.touch()
        // session.save(session=None)
        // # module_logger.info('new session {} user: {} groups: {} expires: {}'.format(session._id, session.user, session.user_and_groups, session.expires))
        // history.SessionLog(session=session, user_agent=user_agent, changed_user=changed_user).save(session=None)

} // Session::create_session

// ----------------------------------------------------------------------

void Session::save(size_t expiration_in_seconds)
{
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::open_array;
    using bsoncxx::builder::stream::close_array;


    auto put_fields = [&](auto& doc2) {
        doc2 << "_t" << "acmacs.mongodb_collections.permissions.Session";
        doc2 << "_id" << bsoncxx::oid{"5967844900be7c983007cc62"};
        doc2 << "user" << mUser;
        auto groups = doc2 << "user_and_groups" << open_array;
        for (const auto& group: mGroups)
            groups << group;
        groups << bsoncxx::builder::stream::close_array;
        doc2 << "I" << "127.0.0.1"
        << "expiration_in_seconds" << static_cast<std::int32_t>(expiration_in_seconds)
        << "expires" << time_format(std::chrono::system_clock::now() + std::chrono::seconds{expiration_in_seconds}, "%F %T")
        << "commands" << 0;
    };

    mId = "5967844900be7c983007cc62";
    if (mId.empty()) {
        auto doc = document{};
        put_fields(doc);
        try {
            auto result = mDb["sessions"].insert_one(doc << finalize);
            if (!result)
                throw SessionError{"unacknowledged write during session creation"};
            if (result->inserted_id().type() == bsoncxx::type::k_oid)
                mId = result->inserted_id().get_oid().value.to_string();
            else
                throw SessionError{"cannot create session: inserted id was not an OID type"};
        }
        catch (const mongocxx::exception& err) {
            throw SessionError{std::string{"cannot create session: "} + err.what()};
        }
    }
    else {

        auto doc = document{} << "$set"
                << open_document << "expires"
                << time_format(std::chrono::system_clock::now() + std::chrono::seconds{expiration_in_seconds}, "%F %T")
                << close_document << finalize;
                                     auto filter = document{} << "_id" << bsoncxx::oid{mId} << finalize;
                                                        auto result = mDb["sessions"].update_one(filter.view(), doc.view());

        // auto doc_set = document{} << "$set" << open_document;
        // put_fields(doc_set);
        // auto doc = doc_set << close_document;
        // auto result = mDb["sessions"].update_one(document{} << "_id" << bsoncxx::oid{mId} << finalize, doc << finalize);
    }

    // // if (!mId.empty())
    // doc << "_t" << "acmacs.mongodb_collections.permissions.Session";
    // // if (!mId.empty())
    // //     doc << "_id" << bsoncxx::oid{mId};
    // doc << "_id" << bsoncxx::oid{"5967844900be7c983007cc62"};
    // doc << "user" << mUser;
    // auto groups = doc << "user_and_groups" << open_array;
    // for (const auto& group: mGroups)
    //     groups << group;
    // groups << bsoncxx::builder::stream::close_array;
    // doc << "I" << "127.0.0.1"
    //     << "expiration_in_seconds" << static_cast<std::int32_t>(expiration_in_seconds)
    //     << "expires" << time_format(std::chrono::system_clock::now() + std::chrono::seconds{expiration_in_seconds}, "%F %T")
    //     << "commands" << 0;
    std::cerr << "session_id: " << mId << std::endl;

} // Session::save

// ----------------------------------------------------------------------

void Session::find_groups_of_user()
{
    auto filter = bsoncxx::builder::stream::document{} << "members" << mUser << bsoncxx::builder::stream::finalize;
    auto options = mongocxx::options::find{};
    options.projection(projection_to_include_fields({"name"}, true));
    auto found = mDb["users_groups"].find(std::move(filter), options);
    mGroups.clear();
    mGroups.push_back(mUser);
    std::transform(found.begin(), found.end(), std::back_inserter(mGroups), [](const auto& entry) { return entry["name"].get_utf8().value.to_string(); });

} // Session::find_groups_of_user

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
            session.login(cnonce, hashed_password);

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
