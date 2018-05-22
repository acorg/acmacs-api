#include <limits>

#include "locationdb/locdb.hh"
#include "seqdb/seqdb.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-chart-2/chart-modify.hh"
#include "acmacs-chart-2/ace-export.hh"
#include "acmacs-webserver/print.hh"
#include "command-chart.hh"
#include "session.hh"
#include "acmacs-api-server.hh"
#include "acmacs-c2.hh"

// #include "acmacs-map-draw/draw.hh"
// #include "acmacs-map-draw/settings.hh"

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
    bool found = false;
    for (const auto collection : {"charts", "inspectors", "logs", "configuration", "users_groups"}) {
        auto doc = MongodbAccess{db()}.find_one(collection, to_bson::object("_id", get_id(), session().read_permissions()), MongodbAccess::exclude("projections", "table"));
        if (doc) {
            if (get_chain_source_data())
                chain_source_data(*doc);
            send(to_json::object("doc", doc->view()));
            found = true;
            break;
        }
    }
    if (!found)
        throw Error{"not found"};

} // Command_doc::run

// ----------------------------------------------------------------------

void Command_doc::chain_source_data(bsoncxx::document::value& doc)
{
    const auto view = doc.view();
    if (const auto doc_type = view["_t"]; doc_type) {
        if (const auto doc_type_s = doc_type.get_utf8().value.to_string(); doc_type_s == "acmacs.inspectors.routine_diagnostics.IncrementalChain" || doc_type_s == "acmacs.inspectors.routine_diagnostics.IncrementalChainForked") {
            if (const auto sources = view["sources"]; sources) {
                bsoncxx::builder::basic::array source_array;
                for (const auto& source : static_cast<bsoncxx::array::view>(sources.get_array()))
                    append_source_data(source.get_oid().value, source_array);
                bsoncxx::builder::basic::document builder;
                builder.append(bsoncxx::builder::concatenate(view));
                builder.append(bsoncxx::builder::basic::kvp("ad_api_source_data", source_array));
                doc = builder.extract();
            }
        }
    }

} // Command_doc::chain_source_data

// ----------------------------------------------------------------------

void Command_doc::append_source_data(bsoncxx::oid&& id, bsoncxx::builder::basic::array& container)
{
    if (auto doc = MongodbAccess{db()}.find_one("charts", to_bson::object("_id", id, session().read_permissions()), MongodbAccess::include("name", "date")); doc) {
        bsoncxx::builder::basic::document source_data;
        if (const auto name = doc->view()["name"]; name)
            source_data.append(bsoncxx::builder::basic::kvp("name", name.get_utf8().value));
        if (const auto date = doc->view()["date"]; date)
            source_data.append(bsoncxx::builder::basic::kvp("date", date.get_utf8().value));
        container.append(source_data);
    }
    else
        container.append(bsoncxx::types::b_null{});

} // Command_doc::append_source_data

// ----------------------------------------------------------------------

const char* Command_doc::description()
{
    return R"(gets document (json) by id, looking in collections: "charts", "inspectors", "logs", "configuration", "users_groups"
    id: id
    chain_source_data: false - if doc is chain, extract its source names and dates and put into ad_api_source_data array)";

} // Command_doc::description

// ----------------------------------------------------------------------

Command_with_c2_access::Command_with_c2_access(from_json::object&& aSrc, MongoAcmacsC2Access& aMongoAccess, ClientConnection& aClientConnection, size_t aCommandNumber)
    : Command{std::move(aSrc), aMongoAccess, aClientConnection, aCommandNumber}, acmacs_c2_{aMongoAccess.acmacs_c2()}
{
} // Command_with_c2_access::Command_with_c2_access

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
    const auto ace = c2().ace_uncompressed(session().id(), get_string("id"), 5);
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

void Command_ace::run()
{
    const size_t projection_no = 0;

    const auto ace = c2().ace_uncompressed(session().id(), get_string("id"), projection_no + 1);
    acmacs::chart::ChartModify chart(acmacs::chart::import_from_data(ace, acmacs::chart::Verify::None, report_time::No));
    const auto exported = ace_export(chart, "mod_acmacs", 0);
    send(exported);

} // Command_ace::run

// ----------------------------------------------------------------------

const char* Command_ace::description()
{
    return R"(gets ace by id
    id :id
)";

} // Command_ace::description

// ----------------------------------------------------------------------

// void Command_map::run()
// {
//     const size_t projection_no = 0;

//     const auto ace = c2().ace_uncompressed(session().id(), get_string("id"), projection_no + 1);
//     ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_data(ace, acmacs::chart::Verify::None, report_time::No)), projection_no);
//     auto settings = settings_default();
//     settings.update(settings_builtin_mods());
//     chart_draw.calculate_viewport();
//     const auto map_data = chart_draw.draw_json(report_time::Yes);
//     send(to_json::object("map", to_json::raw{map_data}));

// } // Command_map::run

// // ----------------------------------------------------------------------

// const char* Command_map::description()
// {
//     return R"(gets map in the json format by id
//     id :id)";

// } // Command_map::description

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
