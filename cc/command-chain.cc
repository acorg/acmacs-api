#include <limits>

#include "acmacs-webserver/print.hh"
#include "acmacs-api/command-chain.hh"
#include "acmacs-api/session.hh"

// ----------------------------------------------------------------------

void Command_chains::run()
{
    auto acmacs_web_db = db();
    int chunk_size = get_chunk_size();
    if (chunk_size == 0)
        chunk_size = std::numeric_limits<decltype(chunk_size)>::max();
    int skip = get_skip();
    const int limit = get_limit() + skip;

    MongodbAccess::bld_doc criteria_bld;
    to_bson::append(criteria_bld, session().read_permissions(), MongodbAccess::field_null_or_absent("parent"), MongodbAccess::field_null_or_absent("backup_of"));
    to_bson::in_for_optional_array_of_strings(criteria_bld, "p.o", "$in", [this](){return this->get_owners();});
    to_bson::in_for_optional_array_of_strings(criteria_bld, "keywords", "$in", [this](){return this->get_keywords();});
    to_bson::in_for_optional_array_of_strings(criteria_bld, "_t", "$in", [this](){return this->get_types();});

    auto criteria = criteria_bld.extract();
      // print_cerr("Command_chains::run ", bsoncxx::to_json(criteria));
    for (int chunk_no = 0; limit == 0 || skip < limit; skip += chunk_size, ++chunk_no) {
        const int use_limit = limit == 0 ? chunk_size : std::min(chunk_size, limit - skip);
        DocumentFindResults results{acmacs_web_db, "inspectors",
                    criteria,
                                      //MongodbAccess::exclude("_id").
                      // MongodbAccess::include("name", "parent", "_m", "keywords", "search", "p.o")
                    MongodbAccess::sort("_m", -1).skip(skip).limit(use_limit)
                    };
        const auto results_json = results.json(); // results.count() is available only after calling results.json()
          // print_cerr("INFO: command_chains: ", results_json);
        if (chunk_no != 0 && results.count() == 0)
            break; // no more data but at least one chunk alreay reported
        send(to_json::v1::object("chain_count", results.count(), "chains", to_json::v1::raw{results_json}));
        if (chunk_size == 0)
            break;
    }

} // Command_chains::run

// ----------------------------------------------------------------------

const char* Command_chains::description()
{
    return R"(lists chains available to the user.
  types :[string] - either raw type (see chain_types command results) or "incremental", "rd", "dimension_test"
  keywords :[string]
  owners :[string]
  skip :number = 0
  limit :number = 0
  chunk_size :number = 0)";

} // Command_chains::description

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif
std::regex Command_chain_keywords::sExludeRdKeywords{"^(rd_[1is]:|rd_1[lm]:|imerge:|opt-no=|num-dim=[0-9]+:proportion-dont-care=0\\.[1-9]+).*"};
#pragma GCC diagnostic pop

void Command_chain_keywords::run()
{
    auto acmacs_web_db = db();
    auto cursor = MongodbAccess{acmacs_web_db}.distinct("inspectors", "keywords", session().read_permissions());
    if (auto values = (*cursor.begin())["values"]; values) {
        std::string result;
        const auto& array = values.get_array().value;
        if (include_rd_keywords()) {
            result = to_json::v1::array(std::begin(array), std::end(array));
        }
        else {
            for (const auto& element: array) {
                const auto kw = element.get_utf8().value.to_string();
                if (!std::regex_match(kw, sExludeRdKeywords))
                    result = to_json::v1::array_append(result, kw);
            }
        }
        send(to_json::v1::object("keywords", to_json::v1::raw{result}));
    }
    else {
        send_error("No data from server");
    }

} // Command_chain_keywords::run

// ----------------------------------------------------------------------

const char* Command_chain_keywords::description()
{
    return "returns set of keywords for available chains\n  include_rd_keywords :bool = false";

} // Command_chain_keywords::description

// ----------------------------------------------------------------------

void Command_chain_owners::run()
{
    auto acmacs_web_db = db();
    auto cursor = MongodbAccess{acmacs_web_db}.distinct("inspectors", "p.o", session().read_permissions());
    if (auto values = (*cursor.begin())["values"]; values) {
        const auto& val = values.get_array().value;
        send(to_json::v1::object("owners", to_json::v1::raw{to_json::v1::value(val)}));
    }
    else {
        send_error("No data from server");
    }

} // Command_chain_owners::run

// ----------------------------------------------------------------------

const char* Command_chain_owners::description()
{
    return "returns set of owners for available chains";

} // Command_chain_owners::description

// ----------------------------------------------------------------------

void Command_chain_types::run()
{
    auto acmacs_web_db = db();
    auto cursor = MongodbAccess{acmacs_web_db}.distinct("inspectors", "_t", session().read_permissions());
    if (auto values = (*cursor.begin())["values"]; values) {
        const auto& val = values.get_array().value;
        send(to_json::v1::object("types", to_json::v1::raw{to_json::v1::value(val)}));
    }
    else {
        send_error("No data from server");
    }

} // Command_chain_types::run

// ----------------------------------------------------------------------

const char* Command_chain_types::description()
{
    return "returns set of types for available chains";

} // Command_chain_types::description

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
