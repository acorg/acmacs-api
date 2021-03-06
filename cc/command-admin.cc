#include "acmacs-api/command-admin.hh"
#include "acmacs-api/session.hh"

// ----------------------------------------------------------------------

void CommandAdmin::run()
{
    if (!session().is_admin())
        send_error("permission denied");
    else
        run_admin();

} // CommandAdmin::run

// ----------------------------------------------------------------------

void Command_users::run_admin()
{
    auto acmacs_web_db = db();
    DocumentFindResults results{acmacs_web_db, "users_groups",
                to_bson::object("_t", "acmacs.mongodb_collections.users_groups.User"),
                MongodbAccess::exclude("_id", "_t", "_m", "password", "nonce")};
    send(to_json::v1::object("users", to_json::v1::raw{results.json()}));

} // Command_users::run_admin

// ----------------------------------------------------------------------

const char* Command_users::description()
{
    return "lists users registered in acmacs-web";

} // Command_users::description

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
