#pragma once

#include "command.hh"

// ----------------------------------------------------------------------

class Command_root_charts : public Command
{
 public:
    using Command::Command;

    virtual void run();

    inline int get_chunk_size() const { return get("chunk_size", 0); }
    inline int get_skip() const { return get("skip", 0); }
    inline int get_limit() const { return get("limit", 0); }
    inline json_importer::ConstArray get_owners() const { return get_array("owners"); } // throws RapidjsonAssert
    inline json_importer::ConstArray get_keywords() const { return get_array("keywords"); } // throws RapidjsonAssert
    inline json_importer::ConstArray get_search() const { return get_array("search"); } // throws RapidjsonAssert

    static const char* description();

}; // class Command_root_charts

// ----------------------------------------------------------------------

class Command_chart_keywords : public Command
{
 public:
    using Command::Command;

    virtual void run();

    static const char* description();

}; // class Command_root_charts

// ----------------------------------------------------------------------

class Command_chart_owners : public Command
{
 public:
    using Command::Command;

    virtual void run();

    static const char* description();

}; // class Command_root_charts

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
