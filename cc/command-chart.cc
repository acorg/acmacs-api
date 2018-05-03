#include <limits>
#include "acmacs-webserver/print.hh"
#include "command-chart.hh"
#include "session.hh"
#include "acmacs-api-server.hh"
#include "acmacs-c2.hh"

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
    to_bson::append(criteria_bld, session().read_permissions(), MongodbAccess::field_null_or_absent("parent"), MongodbAccess::field_null_or_absent("backup_of"));
    to_bson::in_for_optional_array_of_strings(criteria_bld, "p.o", "$in", std::bind(&Command_root_charts::get_owners, this));
    to_bson::in_for_optional_array_of_strings(criteria_bld, "keywords", "$in", std::bind(&Command_root_charts::get_keywords, this));
    to_bson::in_for_optional_array_of_strings(criteria_bld, "search", "$all", std::bind(&Command_root_charts::get_search, this), &from_json::get_string_uppercase);

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
        const auto results_json = results.json(); // results.count() is available only after calling results.json()
        if (chunk_no != 0 && results.count() == 0)
            break; // no more data but at least one chunk alreay reported
        send(to_json::object("chart_count", results.count(), "charts", to_json::raw{results_json}));
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
        const auto& val = values.get_array().value;
        send(to_json::object("keywords", to_json::raw{to_json::value(val)}));
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
        const auto& val = values.get_array().value;
        send(to_json::object("owners", to_json::raw{to_json::value(val)}));
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

void Command_doc::run()
{
    auto acmacs_web_db = db();
    bool found = false;
    for (const auto collection : {"charts", "inspectors", "logs", "configuration", "users_groups"}) {
        const auto doc = MongodbAccess{acmacs_web_db}.find_one(
        collection,
        to_bson::object("_id", get_id(), session().read_permissions()),
        MongodbAccess::exclude("projections", "table"));
        if (doc) {
            send(to_json::object("doc", doc->view()));
            found = true;
            break;
        }
    }
    if (!found)
        throw Error{"not found"};

} // Command_doc::run

// ----------------------------------------------------------------------

const char* Command_doc::description()
{
    return R"(gets document (json) by id, looking in collections: "charts", "inspectors", "logs", "configuration", "users_groups"
    id :id)";

} // Command_doc::description

// ----------------------------------------------------------------------

Command_chart::Command_chart(from_json::object&& aSrc, MongoAcmacsC2Access& aMongoAccess, ClientConnection& aClientConnection, size_t aCommandNumber)
    : Command{std::move(aSrc), aMongoAccess, aClientConnection, aCommandNumber}, mAcmacsC2{aMongoAccess.acmacs_c2()}
{

} // Command_chart::Command_chart

// ----------------------------------------------------------------------

void Command_chart::run()
{
    auto acmacs_web_db = db();
    const auto chart = MongodbAccess{acmacs_web_db}.find_one(
        "charts",
        to_bson::object("_id", get_id(), session().read_permissions()),
        MongodbAccess::exclude("_id", "table", "projections", "conformance"));
    if (!chart)
        throw Error{"not found"};
    const auto ace = mAcmacsC2.ace_uncompressed(session().id(), get_string("id"), 5);
      // send(to_json::object("chart", to_json::raw{to_json::value(chart->view())}, "chart_ace", to_json::raw{ace}));
    send(to_json::object("chart", chart->view(), "chart_ace", to_json::raw{ace}));

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
