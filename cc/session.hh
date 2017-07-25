#pragma once

#include <string>
#include <thread>
#include <mutex>

#include "mongo-access.hh"

// ----------------------------------------------------------------------

class Session : public StoredInMongodb
{
 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    inline Session(mongocxx::database& aDb)
        : StoredInMongodb{aDb, "sessions"}, mCommands{0}, mExpirationInSeconds{3600} {}
    inline Session(const Session& aSrc)
        : StoredInMongodb{aSrc}, mId{aSrc.mId}, mUser{aSrc.mUser}, mDisplayName{aSrc.mDisplayName},
          mGroups{aSrc.mGroups}, mCommands{aSrc.mCommands}, mExpirationInSeconds{aSrc.mExpirationInSeconds} {}

    void use_session(std::string aSessionId); // throws Error
    std::string login_nonce(std::string aUser);
    void login_with_password_digest(std::string aCNonce, std::string aPasswordDigest);

    void login(std::string aUser, std::string aPassword);

    inline std::string id() const { return mId; }
    inline std::string user() const { return mUser; }
    inline std::string display_name() const { return mDisplayName; }
    inline const std::vector<std::string>& groups() const { return mGroups; }

    inline void increment_commands() { ++mCommands; }

    bson_value read_permissions() const;

    inline bool is_admin() const
        {
            return std::find_if(std::begin(mGroups), std::end(mGroups), [](const auto& group) { return group == "admin"; }) != std::end(mGroups);
        }

 protected:
    virtual void add_fields_for_creation(bld_doc& aDoc);
    virtual void add_fields_for_updating(bld_doc& aDoc);

 private:
    mutable std::mutex mAccess;
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
    void reset();
    std::string hashed_password(std::string aCNonce);
    void find_user(std::string aUser, bool aGetPassword);
    std::string get_nonce();

}; // class Session

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
