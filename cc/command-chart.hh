#pragma once

#include "command.hh"

// ----------------------------------------------------------------------

class Command_root_charts : public Command
{
 public:
    using Command::Command;

    virtual void run();

    inline int get_chunk_size() const { return get("chunk_size", 10); }
    inline int get_skip() const { return get("skip", 0); }
    inline int get_limit() const { return get("limit", 0); }

    static const char* description();

}; // class Command_root_charts

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
