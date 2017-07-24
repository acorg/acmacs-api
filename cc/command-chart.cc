#include "command-chart.hh"
#include "session.hh"

// ----------------------------------------------------------------------

void Command_root_charts::run()
{
    auto acmacs_web_db = db();
    DocumentFindResults results{acmacs_web_db, "charts",
                (MongodbAccess::field_null_or_absent("parent")
                 <= MongodbAccess::field_null_or_absent("backup_of")
                 <= session().read_permissions()
                 <= MongodbAccess::bson_finalize),
                  //MongodbAccess::exclude("_id", "_t", "table", "search", "conformance").sort("_m", -1)};
                MongodbAccess::include("name", "parent", "_m").sort("_m", -1)};
    const auto results_json = results.json(false); // results.count() is available only after calling results.json()
    send(json_object("charts_count", results.count(), "charts", json_raw{results_json}));

      // sort by _m
      // chunks

} // Command_root_charts::run

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
