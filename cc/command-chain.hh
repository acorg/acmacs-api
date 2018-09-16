#pragma once

#include <regex>

#include "command.hh"

// ----------------------------------------------------------------------

class Command_chains : public Command
{
 public:
    using Command::Command;

    void run() override;

    int get_chunk_size() const { return rjson::get_or(data(), "chunk_size", 0); }
    int get_skip() const { return rjson::get_or(data(), "skip", 0); }
    int get_limit() const { return rjson::get_or(data(), "limit", 0); }
    const rjson::value& get_owners() const { return data()["owners"]; }
    const rjson::value& get_keywords() const { return data()["keywords"]; }
    // const rjson::value& get_search() const { return data()["search"]; }
    const rjson::value& get_types() const { return data()["types"]; }

    static const char* description();

}; // class Command_root_charts

// ----------------------------------------------------------------------

class Command_chain_keywords : public Command
{
 public:
    using Command::Command;

    void run() override;

    bool include_rd_keywords() const { return rjson::get_or(data(), "include_rd_keywords", false); }

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
