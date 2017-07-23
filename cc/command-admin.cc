#include "command-admin.hh"

// ----------------------------------------------------------------------

void Command_users::run()
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

} // Command_users::run

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
