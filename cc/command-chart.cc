#include "command-chart.hh"
#include "session.hh"

// ----------------------------------------------------------------------

void Command_root_charts::run()
{
    auto acmacs_web_db = db();
    const int chunk_size = get_chunk_size();
    int skip = get_skip();
    const int limit = get_limit() + skip;
    auto criteria = bson_make_value(session().read_permissions(), MongodbAccess::field_null_or_absent("parent"), MongodbAccess::field_null_or_absent("backup_of"));
    std::cerr << "Command_root_charts::run " << bsoncxx::to_json(criteria) << std::endl;
    for (; limit == 0 || skip < limit; skip += chunk_size) {
        DocumentFindResults results{acmacs_web_db, "charts",
                    criteria.view(),
//?
                      //MongodbAccess::exclude("_id", "_t", "table", "search", "conformance").sort("_m", -1)};
                    MongodbAccess::include("name", "parent", "_m", "keywords", "p.o").sort("_m", -1).skip(skip).limit(limit == 0 ? chunk_size : std::min(chunk_size, limit - skip))
                    };
        const auto results_json = results.json(false); // results.count() is available only after calling results.json()
        if (results.count() == 0)
            break;
        send(json_object("charts_count", results.count(), "charts", json_raw{results_json}));
        if (chunk_size == 0)
            break;
    }

} // Command_root_charts::run

// ----------------------------------------------------------------------

const char* Command_root_charts::description()
{
    return "lists root charts (table charts) available to the user.\n  search :[string]\nkeywords :[string]\n  owners :[string]\n skip :number = 0\n  limit :number = 0\n  chunk_size :number = 10";

} // Command_root_charts::description

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
