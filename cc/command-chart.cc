#include <limits>
#include "command-chart.hh"
#include "session.hh"
#include "bson-to-json.hh"
#include "print.hh"

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
    bson_in_for_optional_array_of_strings(criteria_bld, "search", "$all", std::bind(&Command_root_charts::get_search, this), &json_importer::get_string_uppercase);

    auto criteria = criteria_bld.extract();
    print2("Command_root_charts::run ", bsoncxx::to_json(criteria));
    for (int chunk_no = 0; limit == 0 || skip < limit; skip += chunk_size, ++chunk_no) {
        DocumentFindResults results{acmacs_web_db, "charts",
                    criteria,
                      //MongodbAccess::exclude("_id", "_t", "table", "search", "conformance").sort("_m", -1)};
                    MongodbAccess::include("name", "parent", "_m", "keywords", "search", "p.o").sort("_m", -1).skip(skip).limit(limit == 0 ? chunk_size : std::min(chunk_size, limit - skip))
                    };
        const auto results_json = results.json(false); // results.count() is available only after calling results.json()
        if (chunk_no != 0 && results.count() == 0)
            break; // no more data but at least one chunk alreay reported
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

void Command_chart_keywords::run()
{
    auto acmacs_web_db = db();
    print1("Command_chart_keywords::run1");
    auto cursor = MongodbAccess{acmacs_web_db}.distinct("charts", "keywords", session().read_permissions());
    print1("Command_chart_keywords::run2");
    auto values = (*cursor.begin())["values"];
    if (!values)
        send_error("No data from server");
    std::string result = json_writer::compact_json(values.get_array().value);
    print2("Command_chart_keywords::run3 ", result);
    send(json_object("keywords", json_raw{result}));

} // Command_chart_keywords::run

// ----------------------------------------------------------------------

const char* Command_chart_keywords::description()
{
    return "returns set of keywords for available charts";

} // Command_chart_keywords::description

// ----------------------------------------------------------------------

void Command_chart_owners::run()
{
    auto acmacs_web_db = db();
    print1("Command_chart_owners::run1");
    auto cursor = MongodbAccess{acmacs_web_db}.distinct("charts", "p.o", session().read_permissions());
    print1("Command_chart_owners::run2");
    auto values = (*cursor.begin())["values"];
    if (!values)
        send_error("No data from server");
    std::string result = json_writer::compact_json(values.get_array().value);
    print2("Command_chart_owners::run3 ", result);
    send(json_object("owners", json_raw{result}));

} // Command_chart_owners::run

// ----------------------------------------------------------------------

const char* Command_chart_owners::description()
{
    return "returns set of owners for available charts";

} // Command_chart_owners::description

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
