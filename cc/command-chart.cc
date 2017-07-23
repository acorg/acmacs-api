#include "command-chart.hh"
#include "session.hh"

// ----------------------------------------------------------------------

void Command_root_charts::run()
{
    auto acmacs_web_db = db();
    DocumentFindResults results{acmacs_web_db, "charts",
                (DocumentFindResults::bson_doc{}
                 << bsoncxx::builder::concatenate(MongodbAccess::field_null_or_absent("parent").view()) //  << "_t" << "acmacs.mongodb_collections.chart.Table"
                 << bsoncxx::builder::concatenate(MongodbAccess::field_null_or_absent("backup_of").view())
                 << bsoncxx::builder::concatenate(session().read_permissions().view())
                 << DocumentFindResults::bson_finalize),
                MongodbAccess::exclude{"_id", "_t", "table", "search", "conformance"}};
      // MongodbAccess::include_exclude{{"name", "parent"}, {}}};
    send(json_object("charts_count", results.count(), "charts", json_raw{results.json(false)}));

} // Command_root_charts::run

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
