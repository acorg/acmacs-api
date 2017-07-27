#include <limits>
#include "acmacs-webserver/print.hh"
#include "command-chart.hh"
#include "session.hh"
#include "bson-to-json.hh"

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
      // print_cerr("Command_root_charts::run ", bsoncxx::to_json(criteria));
    for (int chunk_no = 0; limit == 0 || skip < limit; skip += chunk_size, ++chunk_no) {
        const int use_limit = limit == 0 ? chunk_size : std::min(chunk_size, limit - skip);
        DocumentFindResults results{acmacs_web_db, "charts",
                    criteria,
                      //MongodbAccess::exclude("_id", "_t", "table", "search", "conformance")
                    MongodbAccess::include("name", "parent", "_m", "keywords", "search", "p.o")
                    .sort("_m", -1).skip(skip).limit(use_limit)
                    };
        const auto results_json = results.json(false); // results.count() is available only after calling results.json()
        if (chunk_no != 0 && results.count() == 0)
            break; // no more data but at least one chunk alreay reported
        send(json_object("chart_count", results.count(), "charts", json_raw{results_json}));
        if (chunk_size == 0)
            break;
    }

} // Command_root_charts::run

// ----------------------------------------------------------------------

const char* Command_root_charts::description()
{
    return R"(lists root charts (table charts) available to the user.
  search :[string case-insensitive]
  keywords :[string]
  owners :[string]
  skip :number = 0
  limit :number = 0
  chunk_size :number = 0)";

} // Command_root_charts::description

// ----------------------------------------------------------------------

void Command_chart_keywords::run()
{
    auto acmacs_web_db = db();
    auto cursor = MongodbAccess{acmacs_web_db}.distinct("charts", "keywords", session().read_permissions());
    if (auto values = (*cursor.begin())["values"]; values) {
        const std::string result = json_writer::compact_json(values.get_array().value);
        send(json_object("keywords", json_raw{result}));
    }
    else {
        send_error("No data from server");
    }

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
    auto cursor = MongodbAccess{acmacs_web_db}.distinct("charts", "p.o", session().read_permissions());
    if (auto values = (*cursor.begin())["values"]; values) {
        const std::string result = json_writer::compact_json(values.get_array().value);
        send(json_object("owners", json_raw{result}));
    }
    else {
        send_error("No data from server");
    }

} // Command_chart_owners::run

// ----------------------------------------------------------------------

const char* Command_chart_owners::description()
{
    return "returns set of owners for available charts";

} // Command_chart_owners::description

// ----------------------------------------------------------------------

inline json_raw json_value(const bsoncxx::document::value& value)
{
    json_writer::compact writer;
    return writer << value << json_writer::finalize;
}

void Command_chart::run()
{
    auto acmacs_web_db = db();
    auto chart = MongodbAccess{acmacs_web_db}.find_one(
        "charts",
        bson_object("_id", get_id(), session().read_permissions()),
        MongodbAccess::exclude("_id", "projections", "conformance"));
    if (!chart)
        throw Error{"not found"};
    send(json_object("chart", json_value(*chart)));

} // Command_chart::run

// ----------------------------------------------------------------------

const char* Command_chart::description()
{
    return R"(gets chart in ace format (json, unzipped) by id
    id :id)";

} // Command_chart::description

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
