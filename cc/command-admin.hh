#pragma once

#include "command.hh"

// ----------------------------------------------------------------------

class Command_users : public Command
{
 public:
    using Command::Command;

    virtual void run();

}; // class Command_users

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
