#pragma once

#include "command.hh"

// ----------------------------------------------------------------------

class CommandAdmin : public Command
{
 public:
    using Command::Command;

    virtual void run();

 protected:
    virtual void run_admin() = 0;

}; // class CommandAdmin

// ----------------------------------------------------------------------

class Command_users : public CommandAdmin
{
 public:
    using CommandAdmin::CommandAdmin;

    static const char* description();

 protected:
    virtual void run_admin();

}; // class Command_users

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
