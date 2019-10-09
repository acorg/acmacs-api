#pragma once

#include "command.hh"

// ----------------------------------------------------------------------

class Command_login_session : public Command
{
 public:
    using Command::Command;

    void run() override;

    std::string session_id() const { return data()["S"].to<std::string>(); }

    static const char* description();

}; // class Command_login_session

// ----------------------------------------------------------------------

class Command_login_nonce : public Command
{
 public:
    using Command::Command;

    void run() override;

    std::string user() const { return data()["user"].to<std::string>(); }

    static const char* description();

}; // class Command_login_nonce

// ----------------------------------------------------------------------

class Command_login_digest : public Command
{
 public:
    using Command::Command;

    void run() override;

    std::string cnonce() const { return data()["cnonce"].to<std::string>(); }
    std::string digest() const { return data()["digest"].to<std::string>(); }

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
