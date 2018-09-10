#pragma once

#include <regex>

#include "command.hh"

// ----------------------------------------------------------------------

class Command_chains : public Command
{
 public:
    using Command::Command;

    void run() override;

    int get_chunk_size() const { return get("chunk_size", 0); }
    int get_skip() const { return get("skip", 0); }
    int get_limit() const { return get("limit", 0); }
    const rjson::v1::array& get_owners() const { return get_array("owners"); }
    const rjson::v1::array& get_keywords() const { return get_array("keywords"); }
    // const rjson::v1::array& get_search() const { return get_array("search"); }
    const rjson::v1::array& get_types() const { return get_array("types"); }

    static const char* description();

}; // class Command_root_charts

// ----------------------------------------------------------------------

class Command_chain_keywords : public Command
{
 public:
    using Command::Command;

    void run() override;

    bool include_rd_keywords() const { return get("include_rd_keywords", false); }

    static const char* description();

 private:
    static std::regex sExludeRdKeywords;

}; // class Command_chain_keywords

// ----------------------------------------------------------------------

class Command_chain_owners : public Command
{
 public:
    using Command::Command;

    void run() override;

    static const char* description();

}; // class Command_chain_owners

// ----------------------------------------------------------------------

class Command_chain_types : public Command
{
 public:
    using Command::Command;

    void run() override;

    static const char* description();

}; // class Command_chain_types

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
