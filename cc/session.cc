#include <iostream>
#include <random>

#include "acmacs-base/string.hh"
#include "md5.hh"
#include "session.hh"

// ----------------------------------------------------------------------

// Session::~Session()
// {

// } // Session::~Session

// ----------------------------------------------------------------------

void Session::reset()
{
    std::unique_lock<decltype(mAccess)> lock{mAccess};
    mId.reset();
    mUser.clear();
    mGroups.clear();

} // Session::reset

// ----------------------------------------------------------------------

void Session::use_session(std::string aSessionId)
{
    reset();

    auto found = find_one(to_bson::object("_id", bsoncxx::oid{aSessionId}, "expires", to_bson::object("$gte", time_now())),
                          exclude("_t", "_m", "I", "expires", "expiration_in_seconds"));
    if (!found)
        throw Error{"invalid session"};
    for (auto entry: found->view()) {
        const std::string key = entry.key().to_string();
        std::unique_lock<decltype(mAccess)> lock{mAccess};
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
    update(mId.id());

} // Session::use_session

// ----------------------------------------------------------------------

std::string Session::login_nonce(std::string aUser)
{
    find_user(aUser, true);
    return get_nonce();

} // Session::login_nonce

// ----------------------------------------------------------------------

void Session::find_user(std::string aUser, bool aGetPassword)
{
    auto found = find_one("users_groups", to_bson::object("name", aUser, "_t", "acmacs.mongodb_collections.users_groups.User"), exclude("_id", "_t", "recent_logins", "created", "p", "_m"));
    if (!found)
        throw Error{"invalid user or password"};
      // std::cerr << json_writer::json(*found, "user", 1) << std::endl;
    for (auto entry: found->view()) {
        const std::string key = entry.key().to_string();
        std::unique_lock<decltype(mAccess)> lock{mAccess};
        if (key == "name")
            mUser = entry.get_value().get_utf8().value.to_string();
        else if (aGetPassword && key == "password")
            mPassword = entry.get_value().get_utf8().value.to_string();
        else if (key == "display_name")
            mDisplayName = entry.get_value().get_utf8().value.to_string();
    }

} // Session::find_user

// ----------------------------------------------------------------------

void Session::login(std::string aUser, std::string aPassword)
{
    find_user(aUser, true);
    const auto nonce = get_nonce();
    std::random_device rd;
    const auto cnonce = string::to_hex_string(rd() & 0xFFFFFFFF, false);
    std::unique_lock<decltype(mAccess)> lock{mAccess};
    const auto digest = md5(mUser + ";acmacs-web;" + aPassword);
    lock.unlock();
    const auto hashed_password = md5(nonce + ";" + cnonce + ";" + digest);
    login_with_password_digest(cnonce, hashed_password);

} // Session::login

// ----------------------------------------------------------------------

std::string Session::get_nonce()
{
    std::random_device rd;
    std::unique_lock<decltype(mAccess)> lock{mAccess};
    mNonce = string::to_hex_string(rd() & 0xFFFFFFFF, false);
    return mNonce;

} // Session::get_nonce

// ----------------------------------------------------------------------

std::string Session::hashed_password(std::string aCNonce)
{
    std::unique_lock<decltype(mAccess)> lock{mAccess};
    return md5(mNonce + ";" + aCNonce + ";" + mPassword);

} // Session::hashed_password

// ----------------------------------------------------------------------

void Session::login_with_password_digest(std::string aCNonce, std::string aPasswordDigest)
{
    if (aPasswordDigest != hashed_password(aCNonce))
        throw Error{"invalid user or password"};
    create_session();
      //! user.add_recent_login(session=session)

} // Session::login_with_password_digest

// ----------------------------------------------------------------------

void Session::create_session()
{
    find_groups_of_user();
      // std::cerr << "Groups: " << mGroups << std::endl;
    const auto id = create();
    std::unique_lock<decltype(mAccess)> lock{mAccess};
    mId = id;

        // history.SessionLog(session=session, user_agent=user_agent, changed_user=changed_user).save(session=None)

} // Session::create_session

// ----------------------------------------------------------------------

void Session::add_fields_for_creation(bld_doc& aDoc)
{
    StoredInMongodb::add_fields_for_creation(aDoc);
    to_bson::append(aDoc,
                "_t", "acmacs.mongodb_collections.permissions.Session",
                "user", mUser,
                "user_and_groups", to_bson::array(std::begin(mGroups), std::end(mGroups)),
                "expiration_in_seconds", mExpirationInSeconds,
                "expires", time_in_seconds(mExpirationInSeconds),
                "commands", mCommands);

} // Session::add_fields_for_creation

// ----------------------------------------------------------------------

void Session::add_fields_for_updating(bld_doc& aDoc)
{
    StoredInMongodb::add_fields_for_updating(aDoc);
    to_bson::append(aDoc,
                "expires", time_in_seconds(mExpirationInSeconds),
                "commands", mCommands);

} // Session::add_fields_for_updating

// ----------------------------------------------------------------------

void Session::find_groups_of_user()
{
    auto found = find("users_groups", to_bson::object("members", mUser), include("name").exclude("_id"));
    std::unique_lock<decltype(mAccess)> lock{mAccess};
    mGroups.clear();
    mGroups.push_back(mUser);
    std::transform(found.begin(), found.end(), std::back_inserter(mGroups), [](const auto& entry) { return entry["name"].get_utf8().value.to_string(); });

} // Session::find_groups_of_user

// ----------------------------------------------------------------------

Session::bson_value Session::read_permissions() const
{
    if (!mId)
        throw Error{"Session has no id"};
    std::unique_lock<decltype(mAccess)> lock{mAccess};
    return to_bson::object("p.r", to_bson::object("$in", to_bson::array(std::begin(mGroups), std::end(mGroups))));

} // Session::read_permissions

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
