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
// ----------------------------------------------------------------------

class DbAccess
{
 public:
    inline DbAccess(mongocxx::database& aDb) : mDb(aDb) {}
    virtual inline ~DbAccess() {}

    using bson_doc = bsoncxx::builder::stream::document;
    static constexpr auto bson_finalize = bsoncxx::builder::stream::finalize;
    static constexpr auto bson_open_document = bsoncxx::builder::stream::open_document;
    static constexpr auto bson_close_document = bsoncxx::builder::stream::close_document;
    static constexpr auto bson_open_array = bsoncxx::builder::stream::open_array;
    static constexpr auto bson_close_array = bsoncxx::builder::stream::close_array;

    using doc_value = bsoncxx::document::value;
    using doc_view = bsoncxx::document::view;
    using cursor = mongocxx::cursor;
    using find_options = mongocxx::options::find;

      // ----------------------------------------------------------------------

    class exclude
    {
     public:
        inline exclude(std::initializer_list<std::string>&& fields)
            {
                auto proj_doc = bson_doc{};
                std::for_each(std::begin(fields), std::end(fields), [&proj_doc](const auto& field) { proj_doc << field << false; });
                mOptions.projection(proj_doc << bson_finalize);
            }

        template <typename ... Args> inline exclude(Args&& ...) : exclude({Args::value...}) {}

        inline operator const find_options& () const { return mOptions; }

     private:
        find_options mOptions;

    }; // class exclude

    class include_exclude
    {
     public:
        inline include_exclude(std::initializer_list<std::string>&& include, std::initializer_list<std::string>&& exclude)
            {
                auto proj_doc = bson_doc{};
                std::for_each(std::begin(include), std::end(include), [&proj_doc](const auto& field) { proj_doc << field << true; });
                std::for_each(std::begin(exclude), std::end(exclude), [&proj_doc](const auto& field) { proj_doc << field << false; });
                mOptions.projection(proj_doc << bson_finalize);
            }

        inline operator const find_options& () const { return mOptions; }

     private:
        find_options mOptions;

    }; // class include_exclude

      // ----------------------------------------------------------------------

    inline auto find(const char* aCollection, doc_value&& aFilter, const find_options& aOptions = find_options{})
        {
            return mDb[aCollection].find(std::move(aFilter), aOptions);
        }

    inline auto find(const char* aCollection)
        {
            return mDb[aCollection].find({});
        }

    inline auto find_one(const char* aCollection, doc_value&& aFilter, const find_options& aOptions = find_options{})
        {
            return mDb[aCollection].find_one(std::move(aFilter), aOptions);
        }

    inline auto insert_one(const char* aCollection, doc_value&& aDoc)
        {
            return mDb[aCollection].insert_one(std::move(aDoc));
        }

    inline auto update_one(const char* aCollection, doc_value&& aFilter, doc_value&& aDoc)
        {
            return mDb[aCollection].update_one(std::move(aFilter), std::move(aDoc));
        }

      // ----------------------------------------------------------------------

    inline std::string time_now() const
        {
            return time_format_gm(std::chrono::system_clock::now(), "%F %T");
        }

    inline std::string time_in_seconds(std::int32_t aSeconds) const
        {
            return time_format_gm(std::chrono::system_clock::now() + std::chrono::seconds{aSeconds}, "%F %T");
        }

 private:
    mongocxx::database& mDb;

}; // class DbAccess

// ----------------------------------------------------------------------

class DocumentFindResults : public DbAccess
{
 public:

    inline DocumentFindResults(mongocxx::database& aDb) : DbAccess{aDb} {}
    inline DocumentFindResults(mongocxx::database& aDb, const char* aCollection) : DbAccess{aDb} { build(aCollection); }
    inline DocumentFindResults(mongocxx::database& aDb, const char* aCollection, doc_value&& aFilter, const find_options& aOptions = find_options{})
        : DbAccess{aDb} { build(aCollection, std::move(aFilter), aOptions); }

    inline void build(const char* aCollection, doc_value&& aFilter, const find_options& aOptions = find_options{})
        {
            auto found = find(aCollection, std::move(aFilter), aOptions);
            std::copy(std::begin(found), std::end(found), std::back_inserter(mRecords));
        }

    inline void build(const char* aCollection)
        {
            auto found = find(aCollection);
            std::copy(std::begin(found), std::end(found), std::back_inserter(mRecords));
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
    std::vector<doc_view> mRecords;

}; // class DocumentFindResults

// ----------------------------------------------------------------------

class StoredInDb : public DbAccess
{
 public:
    inline StoredInDb(mongocxx::database& aDb, const char* aCollection) : DbAccess{aDb}, mCollection{aCollection} {}

 protected:
    using bson_key_context = bsoncxx::builder::stream::key_context<bsoncxx::builder::stream::key_context<bsoncxx::builder::stream::closed_context>>;

    using DbAccess::find;
    inline auto find(doc_value&& aFilter, const find_options& aOptions = find_options{}) { return DbAccess::find(mCollection, std::move(aFilter), aOptions); }
    inline auto find() { return DbAccess::find(mCollection); }
    using DbAccess::find_one;
    inline auto find_one(doc_value&& aFilter, const find_options& aOptions = find_options{}) { return DbAccess::find_one(mCollection, std::move(aFilter), aOptions); }
    inline auto insert_one(doc_value&& aDoc) { return DbAccess::insert_one(mCollection, std::move(aDoc)); }
    inline auto update_one(doc_value&& aFilter, doc_value&& aDoc) { return DbAccess::update_one(mCollection, std::move(aFilter), std::move(aDoc)); }

    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

      // returns new _id
      // throws Error
    inline std::string create()
        {
            auto doc = bson_doc{};
            add_fields_for_creation(doc);
            try {
                auto result = insert_one(doc << bson_finalize);
                if (!result)
                    throw Error{"unacknowledged write during doc insertion"};
                if (result->inserted_id().type() != bsoncxx::type::k_oid)
                    throw Error{"cannot insert doc: inserted id was not an OID type"};
                return result->inserted_id().get_oid().value.to_string();
            }
            catch (const mongocxx::exception& err) {
                throw Error{std::string{"cannot insert doc: "} + err.what()};
            }
        }

      // throws Error
    inline void update(std::string aId)
        {
            auto doc = bson_doc{} << "$set" << bson_open_document;
            add_fields_for_updating(doc);
            auto result = update_one(bson_doc{} << "_id" << bsoncxx::oid{aId} << bson_finalize, doc << bson_close_document << bson_finalize);
            if (!result)
                throw Error{"unacknowledged write during doc updating"};
        }

    virtual inline void add_fields_for_creation(bson_doc& aDoc)
        {
            aDoc << "_m" << time_now();
        }

    virtual inline void add_fields_for_updating(bson_key_context& aDoc)
        {
            aDoc << "_m" << time_now();
        }

 private:
    const char* mCollection;

}; // class StoredInDb

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

class SessionError : public std::runtime_error { public: using std::runtime_error::runtime_error; };

class Session : public StoredInDb
{
 public:
    inline Session(mongocxx::database& aDb) : StoredInDb{aDb, "sessions"}, mCommands{0}, mExpirationInSeconds{3600} {}
    void use_session(std::string aSessionId); // throws SessionError
    void find_user(std::string aUser, bool aGetPassword);
    std::string get_nonce();
    void login(std::string aCNonce, std::string aPasswordDigest);

    inline std::string id() const { return mId; }
    inline std::string user() const { return mUser; }
    inline std::string display_name() const { return mDisplayName; }
    inline const std::vector<std::string>& groups() const { return mGroups; }

    inline void increment_commands() { ++mCommands; }

 protected:
    virtual void add_fields_for_creation(bson_doc& aDoc);
    virtual void add_fields_for_updating(bson_key_context& aDoc);

 private:
    std::string mId;
    std::string mUser;
    std::string mDisplayName;
    std::string mPassword;
    std::vector<std::string> mGroups;
    std::string mNonce;
    std::int32_t mCommands;
    std::int32_t mExpirationInSeconds; // mDb["configuration"] system.sessions.expiration_in_seconds

    void create_session();
    void find_groups_of_user();

}; // class Session

// ----------------------------------------------------------------------

void Session::use_session(std::string aSessionId)
{
    mId.clear();
    mUser.clear();
    mGroups.clear();

    auto found = find_one((bson_doc{} << "_id" << bsoncxx::oid{aSessionId} << "expires" << bson_open_document << "$gte" << time_now() << bson_close_document << bson_finalize),
                          exclude{"_t", "_m", "I", "expires", "expiration_in_seconds", "commands"});
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
        else if (key == "commands")
            mCommands = entry.get_value().get_int32().value;
    }

    increment_commands();
      //update(mId);

} // Session::use_session

// ----------------------------------------------------------------------

void Session::find_user(std::string aUser, bool aGetPassword)
{
    // auto filter = document{} << "name" << aUser << "_t" << "acmacs.mongodb_collections.users_groups.User" << finalize;
    // auto options = mongocxx::options::find{};
    // options.projection(projection_to_exclude_fields({"_id", "_t", "recent_logins", "created", "p", "_m"}));
    auto found = find_one("users_groups", bson_doc{} << "name" << aUser << "_t" << "acmacs.mongodb_collections.users_groups.User" << bson_finalize, exclude{"_id", "_t", "recent_logins", "created", "p", "_m"});
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
    find_groups_of_user();
      // std::cerr << "Groups: " << mGroups << std::endl;
    create();

        // history.SessionLog(session=session, user_agent=user_agent, changed_user=changed_user).save(session=None)

} // Session::create_session

// ----------------------------------------------------------------------

void Session::add_fields_for_creation(bson_doc& aDoc)
{
    StoredInDb::add_fields_for_creation(aDoc);

    aDoc << "_t" << "acmacs.mongodb_collections.permissions.Session"
         << "user" << mUser;
          // aDoc << "I" << "127.0.0.1";
    auto groups = aDoc << "user_and_groups" << bson_open_array;
    for (const auto& group: mGroups)
        groups << group;
    groups << bson_close_array;
    aDoc << "expiration_in_seconds" << mExpirationInSeconds
         << "expires" << time_in_seconds(mExpirationInSeconds)
         << "commands" << mCommands;

} // Session::add_fields_for_creation

// ----------------------------------------------------------------------

void Session::add_fields_for_updating(bson_key_context& aDoc)
{
    StoredInDb::add_fields_for_updating(aDoc);
    aDoc << "expires" << time_in_seconds(mExpirationInSeconds)
         << "commands" << static_cast<std::int32_t>(mCommands);

} // Session::add_fields_for_updating

// ----------------------------------------------------------------------

// void Session::save(std::int32_t expiration_in_seconds)
// {

//     auto put_fields = [&](auto& doc2) {
//         doc2 << "_t" << "acmacs.mongodb_collections.permissions.Session";
//         doc2 << "_m" << time_now();
//         doc2 << "user" << mUser;
//         auto groups = doc2 << "user_and_groups" << bson_open_array;
//         for (const auto& group: mGroups)
//             groups << group;
//         groups << bson_close_array;
//           // doc2 << "I" << "127.0.0.1"
//         doc2 << "expiration_in_seconds" << expiration_in_seconds
//         << "expires" << time_in_seconds(expiration_in_seconds)
//         << "commands" << mCommands;
//     };

//     // mId = "596885c0a2589ec658d3a5e9";
//     if (mId.empty()) {
//         auto doc = bson_doc{};
//         put_fields(doc);
//         try {
//             auto result = insert_one(doc << bson_finalize);
//             if (!result)
//                 throw SessionError{"unacknowledged write during session creation"};
//             if (result->inserted_id().type() == bsoncxx::type::k_oid)
//                 mId = result->inserted_id().get_oid().value.to_string();
//             else
//                 throw SessionError{"cannot create session: inserted id was not an OID type"};
//         }
//         catch (const mongocxx::exception& err) {
//             throw SessionError{std::string{"cannot create session: "} + err.what()};
//         }
//     }
//     else {

//         auto doc = bson_doc{} << "$set"
//                 << bson_open_document
//                 << "_m" << time_now()
//                 << "expires" << time_in_seconds(expiration_in_seconds)
//                 << "commands" << static_cast<std::int32_t>(mCommands)
//                 << bson_close_document
//                 << bson_finalize;
//         auto result = update_one(bson_doc{} << "_id" << bsoncxx::oid{mId} << bson_finalize, std::move(doc));

//         // auto doc_set = bson_doc{} << "$set" << open_document;
//         // put_fields(doc_set);
//         // auto doc = doc_set << close_document;
//         // auto result = mDb["sessions"].update_one(bson_doc{} << "_id" << bsoncxx::oid{mId} << finalize, doc << finalize);
//     }

//     // // if (!mId.empty())
//     // doc << "_t" << "acmacs.mongodb_collections.permissions.Session";
//     // // if (!mId.empty())
//     // //     doc << "_id" << bsoncxx::oid{mId};
//     // doc << "_id" << bsoncxx::oid{"5967844900be7c983007cc62"};
//     // doc << "user" << mUser;
//     // auto groups = doc << "user_and_groups" << open_array;
//     // for (const auto& group: mGroups)
//     //     groups << group;
//     // groups << bsoncxx::builder::stream::close_array;
//     // doc << "I" << "127.0.0.1"
//     //     << "expiration_in_seconds" << static_cast<std::int32_t>(expiration_in_seconds)
//     //     << "expires" << time_format_gm(std::chrono::system_clock::now() + std::chrono::seconds{expiration_in_seconds}, "%F %T")
//     //     << "commands" << 0;
//     std::cerr << "session_id: " << mId << std::endl;

// } // Session::save

// ----------------------------------------------------------------------

void Session::find_groups_of_user()
{
    auto found = find("users_groups", bson_doc{} << "members" << mUser << bson_finalize, include_exclude{{"name"}, {"_id"}});
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
    using bson_doc = bsoncxx::builder::stream::document;
    static constexpr auto bson_finalize = bsoncxx::builder::stream::finalize;

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
            DocumentFindResults results{aDb, "users_groups",
                        (DocumentFindResults::bson_doc{} << "_t" << "acmacs.mongodb_collections.users_groups.User" << DocumentFindResults::bson_finalize),
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
                        (DocumentFindResults::bson_doc{} << "_t" << "acmacs.mongodb_collections.users_groups.Group" << DocumentFindResults::bson_finalize),
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
