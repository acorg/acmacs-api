#pragma once

#include "hidb-5/hidb.hh"
#include "acmacs-api/command.hh"

// ----------------------------------------------------------------------

class Command_hidb_antigen_serum : public Command
{
 public:
    using Command::Command;

    std::string get_virus_type() const { return data()["virus_type"].to<std::string>(); }
    std::string get_name() const { return data()["name"].to<std::string>(); }
    std::string get_reassortant() const { return data()["reassortant"].to<std::string>(); }
    const rjson::value& get_annotations() const { return data()["annotations"]; }

 protected:
    std::string make_tables(const hidb::Tables& tables, const hidb::TableIndexList& indexes);
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
