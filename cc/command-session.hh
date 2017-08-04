#pragma once

#include "command.hh"

// ----------------------------------------------------------------------

class Command_login_session : public Command
{
 public:
    using Command::Command;

    virtual void run();

    inline std::string session_id() const { return get_string("S"); }

    static const char* description();

}; // class Command_login_session

// ----------------------------------------------------------------------

class Command_login_nonce : public Command
{
 public:
    using Command::Command;

    virtual void run();

    inline std::string user() const { return get_string("user"); }

    static const char* description();

}; // class Command_login_nonce

// ----------------------------------------------------------------------

class Command_login_digest : public Command
{
 public:
    using Command::Command;

    virtual void run();

    inline std::string cnonce() const { return get_string("cnonce"); }
    inline std::string digest() const { return get_string("digest"); }

    static const char* description();

}; // class Command_login_digest

// ----------------------------------------------------------------------

class Command_logout : public Command
{
 public:
    using Command::Command;

    virtual void run();

    static const char* description();

}; // class Command_logout

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
