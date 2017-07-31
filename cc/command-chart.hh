#pragma once

#include "command.hh"

class AcmacsC2;

// ----------------------------------------------------------------------

class Command_root_charts : public Command
{
 public:
    using Command::Command;

    virtual void run();

    inline int get_chunk_size() const { return get("chunk_size", 0); }
    inline int get_skip() const { return get("skip", 0); }
    inline int get_limit() const { return get("limit", 0); }
    inline from_json::ConstArray get_owners() const { return get_array("owners"); } // throws rapidjson_assert
    inline from_json::ConstArray get_keywords() const { return get_array("keywords"); } // throws rapidjson_assert
    inline from_json::ConstArray get_search() const { return get_array("search"); } // throws rapidjson_assert

    static const char* description();

}; // class Command_root_charts

// ----------------------------------------------------------------------

class Command_chart_keywords : public Command
{
 public:
    using Command::Command;

    virtual void run();

    static const char* description();

}; // class Command_chart_keywords

// ----------------------------------------------------------------------

class Command_chart_owners : public Command
{
 public:
    using Command::Command;

    virtual void run();

    static const char* description();

}; // class Command_chart_owners

// ----------------------------------------------------------------------

class Command_chart : public Command
{
 public:
    Command_chart(from_json::object&& aSrc, MongoAcmacsC2Access& aMongoAccess, ClientConnection& aClientConnection, size_t aCommandNumber);

    virtual void run();

    inline auto get_id() const { return bsoncxx::oid{get_string("id")}; }

    static const char* description();

 private:
    AcmacsC2& mAcmacsC2;

}; // class Command_chart

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
