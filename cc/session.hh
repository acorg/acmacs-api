#pragma once

#include <string>

#include "mongo-access.hh"

// ----------------------------------------------------------------------

class SessionError : public std::runtime_error { public: using std::runtime_error::runtime_error; };

class Session : public StoredInMongodb
{
 public:
    inline Session(mongocxx::database& aDb) : StoredInMongodb{aDb, "sessions"}, mCommands{0}, mExpirationInSeconds{3600} {}
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
    virtual void add_fields_for_updating(bson_doc& aDoc);

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
