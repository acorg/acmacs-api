#pragma once

#include "command.hh"

// ----------------------------------------------------------------------

class CommandAdmin : public Command
{
 public:
    using Command::Command;

    void run() override;

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
    void run_admin() override;

}; // class Command_users

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
