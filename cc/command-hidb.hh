#pragma once

#include "command.hh"

// ----------------------------------------------------------------------

class Command_hidb_antigen_serum : public Command
{
 public:
    using Command::Command;

    std::string get_virus_type() const { return get_string("virus_type"); }
    std::string get_name() const { return get_string("name"); }
    std::string get_reassortant() const { return get_string("reassortant"); }
    from_json::ConstArray get_annotations() const { return get_array("annotations"); } // throws rapidjson_assert

}; // class Command_hidb_antigen_serum

// ----------------------------------------------------------------------

class Command_hidb_antigen : public Command_hidb_antigen_serum
{
 public:
    using Command_hidb_antigen_serum::Command_hidb_antigen_serum;

    void run() override;
    static const char* description();

}; // class Command_hidb_antigen

// ----------------------------------------------------------------------

class Command_hidb_serum : public Command_hidb_antigen_serum
{
 public:
    using Command_hidb_antigen_serum::Command_hidb_antigen_serum;

    void run() override;
    static const char* description();

}; // class Command_hidb_serum

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
