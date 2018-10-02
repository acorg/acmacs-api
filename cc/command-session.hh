#pragma once

#include "command.hh"

// ----------------------------------------------------------------------

class Command_login_session : public Command
{
 public:
    using Command::Command;

    void run() override;

    std::string session_id() const { return static_cast<std::string>(data()["S"]); }

    static const char* description();

}; // class Command_login_session

// ----------------------------------------------------------------------

class Command_login_nonce : public Command
{
 public:
    using Command::Command;

    void run() override;

    std::string user() const { return static_cast<std::string>(data()["user"]); }

    static const char* description();

}; // class Command_login_nonce

// ----------------------------------------------------------------------

class Command_login_digest : public Command
{
 public:
    using Command::Command;

    void run() override;

    std::string cnonce() const { return static_cast<std::string>(data()["cnonce"]); }
    std::string digest() const { return static_cast<std::string>(data()["digest"]); }

    static const char* description();

}; // class Command_login_digest

// ----------------------------------------------------------------------

class Command_logout : public Command
{
 public:
    using Command::Command;

    void run() override;

    static const char* description();

}; // class Command_logout

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
