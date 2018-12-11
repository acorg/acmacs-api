#include <algorithm>

#include "hidb-5/hidb.hh"
#include "hidb-5/hidb-set.hh"
#include "command-hidb.hh"

// ----------------------------------------------------------------------

std::string Command_hidb_antigen_serum::make_tables(const hidb::Tables& tables, const hidb::indexes_t& indexes)
{
    return to_json::array(indexes.begin(), indexes.end(), [this,&tables](size_t index) -> to_json::raw { return this->make_table(*tables[index]); });

    // if (indexes.empty())
    //     return "{}";

    // return to_json::object(
    //     "most_recent", to_json::raw(make_table(*tables.most_recent(indexes))),
    //     "oldest", to_json::raw(make_table(*tables.oldest(indexes))),
    //     "count", indexes.size(),
    //     "all", to_json::raw(to_json::array(indexes.begin(), indexes.end(), [this,&tables](size_t index) -> to_json::raw { return this->make_table(*tables[index]); }))
    //                        );

} // Command_hidb_antigen_serum::make_tables

// ----------------------------------------------------------------------

std::string Command_hidb_antigen_serum::make_table(const hidb::Table& table)
{
    return to_json::object(
        to_json::ignore_empty_values,
          // "name", table.name(),
        "assay", table.assay(),
        "lab", table.lab(),
        "date", table.date(),
        "rbc", table.rbc()
                           );

} // Command_hidb_antigen_serum::make_table

// ----------------------------------------------------------------------

void Command_hidb_antigen::run()
{
    try {
        const auto& hidb = hidb::get(get_virus_type(), report_time::no);
        auto hidb_antigens = hidb.antigens();
        auto hidb_tables = hidb.tables();
        hidb::AntigenPList found;
        if (const auto indexes = hidb_antigens->find(get_name(), hidb::fix_location::no, hidb::find_fuzzy::no); !indexes.empty()) {
            std::transform(indexes.begin(), indexes.end(), std::back_inserter(found), [](const auto& antigen_index) -> hidb::AntigenP { return antigen_index.first; });
        }
        // else if (const auto lab_ids = get_lab_ids(); !lab_ids.empty()) {
        // }
        if (found.empty())
            send_error("No data for antigen \"" + get_name() + "\"");

        send(to_json::object("virus_type", get_virus_type(), "name", get_name(), "antigens",
                             to_json::raw(to_json::array(found.begin(), found.end(), [this,&hidb_tables](auto antigen) -> to_json::raw { return this->make_entry(*hidb_tables, *antigen); }))));
    }
    catch (hidb::get_error& err) {
        send_error("No HiDb for \"" + get_virus_type() + "\": " + err.what());
    }

} // Command_hidb_antigen::run

// ----------------------------------------------------------------------

std::string Command_hidb_antigen::make_entry(const hidb::Tables& tables, const hidb::Antigen& antigen)
{
    const auto lab_ids = antigen.lab_ids();
    const auto annotations = antigen.annotations();
    return to_json::object(
        to_json::ignore_empty_values,
        "name", static_cast<std::string>(antigen.name()),
        "date", static_cast<std::string>(antigen.date()),
        "passage", static_cast<std::string>(antigen.passage()),
        "reassortant", static_cast<std::string>(antigen.reassortant()),
        "annotations", to_json::raw(to_json::array(annotations.begin(), annotations.end())),
        "lineage", static_cast<std::string>(antigen.lineage()),
        "lab_ids", to_json::raw(to_json::array(lab_ids.begin(), lab_ids.end())),
        "tables", to_json::raw(make_tables(tables, antigen.tables()))
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
    try {
        const auto& hidb = hidb::get(get_virus_type(), report_time::no);
        auto hidb_sera = hidb.sera();
        auto hidb_tables = hidb.tables();
        hidb::SerumPList found;
        if (const auto indexes = hidb_sera->find(get_name(), hidb::fix_location::no, hidb::find_fuzzy::no); !indexes.empty()) {
            std::transform(indexes.begin(), indexes.end(), std::back_inserter(found), [](const auto& serum_index) -> hidb::SerumP { return serum_index.first; });
        }
        if (found.empty())
            send_error("No data for serum \"" + get_name() + "\"");

        send(to_json::object("virus_type", get_virus_type(), "name", get_name(), "sera",
                             to_json::raw(to_json::array(found.begin(), found.end(), [this,&hidb_tables](auto serum) -> to_json::raw { return this->make_entry(*hidb_tables, *serum); }))));
    }
    catch (hidb::get_error& err) {
        send_error("No HiDb for \"" + get_virus_type() + "\": " + err.what());
    }

} // Command_hidb_serum::run

// ----------------------------------------------------------------------

std::string Command_hidb_serum::make_entry(const hidb::Tables& tables, const hidb::Serum& serum)
{
    const auto annotations = serum.annotations();
    return to_json::object(
        to_json::ignore_empty_values,
        "name", static_cast<std::string>(serum.name()),
        "passage", static_cast<std::string>(serum.passage()),
        "reassortant", static_cast<std::string>(serum.reassortant()),
        "annotations", to_json::raw(to_json::array(annotations.begin(), annotations.end())),
        "serum_id", static_cast<std::string>(serum.serum_id()),
        "serum_species", static_cast<std::string>(serum.serum_species()),
        "lineage", static_cast<std::string>(serum.lineage()),
        "tables", to_json::raw(make_tables(tables, serum.tables()))
        );

} // Command_hidb_antigen::make_entry

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
