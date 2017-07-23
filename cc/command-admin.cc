#include "command-admin.hh"
#include "session.hh"

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
    // try {
        auto acmacs_web_db = db();
        DocumentFindResults results{acmacs_web_db, "users_groups",
                    (DocumentFindResults::bson_doc{} << "_t" << "acmacs.mongodb_collections.users_groups.User"
                       // << bsoncxx::builder::concatenate(aSession.read_permissions().view())
                     << DocumentFindResults::bson_finalize),
                    MongodbAccess::exclude{"_id", "_t", "_m", "password", "nonce"}};
        send(json_object("users", json_raw{results.json(false)}));
    // }
    // catch (DocumentFindResults::Error& err) {
    //     send_error(err.what());
    // }

} // Command_users::run_admin

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
