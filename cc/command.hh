#pragma once

#include <string>
#include <chrono>

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <mongocxx/database.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/from-json.hh"
#include "acmacs-base/to-json.hh"

#include "send-func.hh"
#include "session.hh"

// ----------------------------------------------------------------------

class WsppThreadWithMongoAccess;

// ----------------------------------------------------------------------

class Command : public from_json::object
{
 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    using time_point = decltype(std::chrono::high_resolution_clock::now());

    Command(from_json::object&& aSrc, WsppThreadWithMongoAccess& aMongoAccess, SendFunc aSendFunc, size_t aCommandNumber);

    // inline Command(from_json::object&& aSrc, mongocxx::database aDb, Session& aSession, SendFunc aSendFunc, size_t aCommandNumber)
    //     : from_json::object{std::move(aSrc)}, mDb{aDb}, mSession{aSession}, mSendFunc{aSendFunc}, mCommandNumber{aCommandNumber}
    //     {
    //         set_command_start();
    //     }

    inline std::string command_name() const { return get_string("C"); }
    inline size_t command_number() const { return mCommandNumber; }

    virtual void run() = 0;

    void send(std::string aMessage, send_message_type aMessageType = send_message_type::text);
    void send_error(std::string aMessage);

 protected:
    inline mongocxx::database& db() { return mDb; }
    inline Session& session() { return mSession; }

    inline time_point now() const { return std::chrono::high_resolution_clock::now(); }
    inline void set_command_start() { mCommandStart = now(); }
    inline auto command_start() const { return mCommandStart; }
    inline double command_duration() const { return std::chrono::duration<double>{now() - command_start()}.count(); }

 private:
    mongocxx::database mDb;
    Session mSession;
    SendFunc mSendFunc;
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
