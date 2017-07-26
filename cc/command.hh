#pragma once

#include <string>
#include <chrono>

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <mongocxx/database.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/rapidjson.hh"

#include "send-func.hh"

// ----------------------------------------------------------------------

class Session;

// ----------------------------------------------------------------------

class Command : public json_importer::Object
{
 public:
    using time_point = decltype(std::chrono::high_resolution_clock::now());

    inline Command(json_importer::Object&& aSrc, mongocxx::database& aDb, Session& aSession, SendFunc aSendFunc, size_t aCommandNumber)
        : json_importer::Object{std::move(aSrc)}, mDb{aDb}, mSession{aSession}, mSendFunc{aSendFunc}, mCommandNumber{aCommandNumber}
        {
            set_command_start();
        }

    inline std::string command_name() const { return get_string("C"); }
    inline size_t command_number() const { return mCommandNumber; }

    virtual void run() = 0;

 protected:
    void send(std::string aMessage, send_message_type aMessageType = send_message_type::text);
    void send_error(std::string aMessage);
    inline mongocxx::database& db() { return mDb; }
    inline Session& session() { return mSession; }

    inline time_point now() const { return std::chrono::high_resolution_clock::now(); }
    inline void set_command_start() { mCommandStart = now(); }
    inline auto command_start() const { return mCommandStart; }
    inline double command_duration() const { return std::chrono::duration<double>{now() - command_start()}.count(); }

 private:
    mongocxx::database& mDb;
    Session& mSession;
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
