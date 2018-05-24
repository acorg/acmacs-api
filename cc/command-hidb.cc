#include <algorithm>

#include "hidb-5/hidb.hh"
#include "hidb-5/hidb-set.hh"
#include "command-hidb.hh"

// ----------------------------------------------------------------------

void Command_hidb_antigen::run()
{
    try {
        auto hidb_antigens = hidb::get(get_virus_type(), report_time::No).antigens();
        hidb::AntigenPList found;
        if (const auto indexes = hidb_antigens->find(get_name(), hidb::FixLocation::No, hidb::FindFuzzy::No); !indexes.empty()) {
            std::transform(indexes.begin(), indexes.end(), std::back_inserter(found), [](const auto& antigen_index) -> hidb::AntigenP { return antigen_index.first; });
        }
        // else if (const auto lab_ids = get_lab_ids(); !lab_ids.empty()) {
        // }
        if (found.empty())
            send_error("No data for antigen \"" + get_name() + "\"");

        send(to_json::object("virus_type", get_virus_type(), "name", get_name(), "antigens",
                             to_json::raw(to_json::array(found.begin(), found.end(), [this](auto antigen) -> to_json::raw { return this->make_entry(*antigen); }))));
    }
    catch (hidb::get_error& err) {
        send_error("No HiDb for \"" + get_virus_type() + "\": " + err.what());
    }

} // Command_hidb_antigen::run

// ----------------------------------------------------------------------

std::string Command_hidb_antigen::make_entry(const hidb::Antigen& antigen)
{
    const auto lab_ids = antigen.lab_ids();
    const auto annotations = antigen.annotations();
    const auto table_indexes = antigen.tables();
    return to_json::object(
        to_json::ignore_empty_values,
        "name", static_cast<std::string>(antigen.name()),
        "date", static_cast<std::string>(antigen.date()),
        "passage", static_cast<std::string>(antigen.passage()),
        "reassortant", static_cast<std::string>(antigen.reassortant()),
        "annotations", to_json::raw(to_json::array(annotations.begin(), annotations.end())),
        "lineage", static_cast<std::string>(antigen.lineage()),
        "lab_ids", to_json::raw(to_json::array(lab_ids.begin(), lab_ids.end())),
        "tables", to_json::raw(to_json::array(table_indexes.begin(), table_indexes.end()))
        );

} // Command_hidb_antigen::make_entry

// ----------------------------------------------------------------------

const char* Command_hidb_antigen::description()
{
    return R"(gets infomation about antigen from hidb
    virus_type :string
    name :string
    passage :string
    reassortant :string
    annotations :[string]
    lab_ids :[string])";

} // Command_hidb_antigen::description

// ----------------------------------------------------------------------

void Command_hidb_serum::run()
{
    send(to_json::object("virus_type", get_virus_type(), "name", get_name()));

} // Command_hidb_serum::run

// ----------------------------------------------------------------------

const char* Command_hidb_serum::description()
{
    return R"(gets infomation about serum from hidb
    virus_type :string
    name :string
    reassortant :string
    annotations :[string]
    serum_id :string)";

} // Command_hidb_serum::description

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
