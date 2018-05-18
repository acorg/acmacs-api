#pragma once

#include <string>
#include <chrono>

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <mongocxx/database.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/from-json.hh"
#include "client-connection.hh"

// ----------------------------------------------------------------------

class MongoAcmacsC2Access;

// ----------------------------------------------------------------------

class Command : public from_json::object
{
 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    using time_point = decltype(std::chrono::high_resolution_clock::now());

    Command(from_json::object&& aSrc, MongoAcmacsC2Access& aMongoAccess, ClientConnection& aClientConnection, size_t aCommandNumber);

    std::string command_name() const { return get_string("C"); }
    std::string command_id() const { return get_string("D"); }
    std::string add_to_response() const { return get_as_string("add_to_response"); }
    size_t command_number() const { return mCommandNumber; }

    virtual void run() = 0;

    void send(std::string aMessage, send_message_type aMessageType = send_message_type::text);
    void send_error(std::string aMessage);

 protected:
    mongocxx::database& db() { return mDb; }
    Session& session() { return mClientConnection.session(); }
    void make_session() { return mClientConnection.make_session(db()); }

    time_point now() const { return std::chrono::high_resolution_clock::now(); }
    void set_command_start() { mCommandStart = now(); }
    auto command_start() const { return mCommandStart; }
    double command_duration() const { return std::chrono::duration<double>{now() - command_start()}.count(); }

 private:
    mongocxx::database mDb;
    ClientConnection& mClientConnection;
    const size_t mCommandNumber;
    time_point mCommandStart;

}; // class Command

// ----------------------------------------------------------------------

class Command_unknown : public Command
{
  public:
    using Command::Command;

    void run() override { send_error("unrecognized message"); }

}; // class Command_unknown

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
