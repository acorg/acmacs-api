#include <limits>
#include "command-chart.hh"
#include "session.hh"

// ----------------------------------------------------------------------

void Command_root_charts::run()
{
    auto acmacs_web_db = db();
    int chunk_size = get_chunk_size();
    if (chunk_size == 0)
        chunk_size = std::numeric_limits<decltype(chunk_size)>::max();
    int skip = get_skip();
    const int limit = get_limit() + skip;

    MongodbAccess::bld_doc criteria_bld;
    bson_append(criteria_bld, session().read_permissions(), MongodbAccess::field_null_or_absent("parent"), MongodbAccess::field_null_or_absent("backup_of"));
    bson_in_for_optional_array_of_strings(criteria_bld, "p.o", "$in", std::bind(&Command_root_charts::get_owners, this));
    bson_in_for_optional_array_of_strings(criteria_bld, "keywords", "$in", std::bind(&Command_root_charts::get_keywords, this));
    bson_in_for_optional_array_of_strings(criteria_bld, "search", "$all", std::bind(&Command_root_charts::get_search, this));

    auto criteria = criteria_bld.extract();
    std::cerr << "Command_root_charts::run " << bsoncxx::to_json(criteria) << std::endl;
    for (int chunk_no = 0; limit == 0 || skip < limit; skip += chunk_size, ++chunk_no) {
        DocumentFindResults results{acmacs_web_db, "charts",
                    criteria,
//?
                      //MongodbAccess::exclude("_id", "_t", "table", "search", "conformance").sort("_m", -1)};
                    MongodbAccess::include("name", "parent", "_m", "keywords", "search", "p.o").sort("_m", -1).skip(skip).limit(limit == 0 ? chunk_size : std::min(chunk_size, limit - skip))
                    };
        const auto results_json = results.json(false); // results.count() is available only after calling results.json()
        if (chunk_no != 0 && results.count() == 0)
            break;
        send(json_object("charts_count", results.count(), "charts", json_raw{results_json}));
        if (chunk_size == 0)
            break;
    }

} // Command_root_charts::run

// ----------------------------------------------------------------------

const char* Command_root_charts::description()
{
    return "lists root charts (table charts) available to the user.\n  search :[string case-insensitive]\nkeywords :[string]\n  owners :[string]\n skip :number = 0\n  limit :number = 0\n  chunk_size :number = 10";

} // Command_root_charts::description

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
