#pragma once

#include "command.hh"

// ----------------------------------------------------------------------

class Command_version : public Command
{
 public:
    using Command::Command;
    void run() override;
    static const char* description();

}; // class Command_version

// ----------------------------------------------------------------------

class Command_list_commands : public Command
{
 public:
    using Command::Command;
    void run() override;
    static const char* description();

}; // class Command_list_commands

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
