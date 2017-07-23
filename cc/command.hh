#pragma once

#include <string>
#include <chrono>

#include "acmacs-base/rapidjson.hh"
#include "acmacs-webserver/websocketpp-asio.hh"

#include "mongo-access.hh"

// ----------------------------------------------------------------------

class AcmacsAPIServer;
class Session;

class Command : public json_importer::Object
{
 public:
    using time_point = decltype(std::chrono::high_resolution_clock::now());

    inline Command(json_importer::Object&& aSrc, AcmacsAPIServer& aServer, size_t aCommandNumber)
        : json_importer::Object{std::move(aSrc)}, mServer{aServer}, mCommandNumber{aCommandNumber}
        {
            set_command_start();
        }

    inline std::string command_name() const { return get_string("C"); }
    inline size_t command_number() const { return mCommandNumber; }

    virtual void run() = 0;

 protected:
    void send(std::string aMessage, websocketpp::frame::opcode::value op_code = websocketpp::frame::opcode::text);
    void send_error(std::string aMessage);
    mongocxx::database db();
    Session& session();

    inline time_point now() const { return std::chrono::high_resolution_clock::now(); }
    inline void set_command_start() { std::cerr << "set_command_start" << std::endl; mCommandStart = now(); }
    inline auto command_start() const { return mCommandStart; }
    inline double command_duration() const { return std::chrono::duration<double>{now() - command_start()}.count(); }

 private:
    AcmacsAPIServer& mServer;
    const size_t mCommandNumber;
    time_point mCommandStart;

}; // class Command

// ----------------------------------------------------------------------

class Command_unknown : public Command
{
 public:
    using Command::Command;

    virtual inline void run()
        {
            send_error("unrecognized message");
        }

}; // class Command_users

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
