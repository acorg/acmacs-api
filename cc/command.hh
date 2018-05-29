#pragma once

#include <string>
#include <chrono>

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <mongocxx/database.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/rjson.hh"
#include "client-connection.hh"

// ----------------------------------------------------------------------

class MongoAcmacsC2Access;

// ----------------------------------------------------------------------

class Command
{
 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    using time_point = decltype(std::chrono::high_resolution_clock::now());

    Command(rjson::object&& aSrc, MongoAcmacsC2Access& aMongoAccess, ClientConnection& aClientConnection, size_t aCommandNumber);
    virtual ~Command() = default;

    std::string command_name() const { return get_string("C"); }
    std::string command_id() const { return get_string("D"); }
    const rjson::value& add_to_response() const { return data_["add_to_response"]; } // may throw rjson::field_not_found
    size_t command_number() const { return mCommandNumber; }

    virtual void run() = 0;

    void send(std::string aMessage, send_message_type aMessageType = send_message_type::text);
    void send_error(std::string aMessage);
    void send_binary(std::string aName, std::string aData);

 protected:
    mongocxx::database& db() { return mDb; }
    Session& session() { return mClientConnection.session(); }
    void make_session() { return mClientConnection.make_session(db()); }

    const rjson::object& data() const { return data_; }
    std::string get_string(const char* field_name) const { return data().get_string_or_throw(field_name); }
    template <typename T> T get(const char* field_name, T&& aDefault) const { return data().get_or_default(field_name, std::forward<T>(aDefault)); }
    const rjson::array& get_array(const char* field_name) const { return data().get_or_empty_array(field_name); }

    time_point now() const { return std::chrono::high_resolution_clock::now(); }
    void set_command_start() { mCommandStart = now(); }
    auto command_start() const { return mCommandStart; }
    double command_duration() const { return std::chrono::duration<double>{now() - command_start()}.count(); }
    std::ostream& log_send_receive() { return mClientConnection.log_send_receive(); }

 private:
    rjson::object data_;
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
