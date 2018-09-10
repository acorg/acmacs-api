#pragma once

#include "command.hh"

namespace hidb { class Antigen; class Serum; class Tables; class Table; using indexes_t = std::vector<size_t>; }

// ----------------------------------------------------------------------

class Command_hidb_antigen_serum : public Command
{
 public:
    using Command::Command;

    std::string get_virus_type() const { return get_string("virus_type"); }
    std::string get_name() const { return get_string("name"); }
    std::string get_reassortant() const { return get_string("reassortant"); }
    const rjson::v1::array& get_annotations() const { return get_array("annotations"); }

 protected:
    std::string make_tables(const hidb::Tables& tables, const hidb::indexes_t& indexes);
    std::string make_table(const hidb::Table& table);

}; // class Command_hidb_antigen_serum

// ----------------------------------------------------------------------

class Command_hidb_antigen : public Command_hidb_antigen_serum
{
 public:
    using Command_hidb_antigen_serum::Command_hidb_antigen_serum;

    void run() override;
    static const char* description();

 private:
    std::string make_entry(const hidb::Tables& tables, const hidb::Antigen& antigen);

}; // class Command_hidb_antigen

// ----------------------------------------------------------------------

class Command_hidb_serum : public Command_hidb_antigen_serum
{
 public:
    using Command_hidb_antigen_serum::Command_hidb_antigen_serum;

    void run() override;
    static const char* description();

 private:
    std::string make_entry(const hidb::Tables& tables, const hidb::Serum& serum);

}; // class Command_hidb_serum

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
