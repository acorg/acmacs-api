#pragma once

#include <vector>

#include "command.hh"
#include "session.hh"

// ----------------------------------------------------------------------

class Handler;
class Login;
namespace client { struct RawMessage; }

// ----------------------------------------------------------------------

class Responder
{
 public:
    inline Responder(String* aCommand, String* aCommandId, Handler* aHandler)
        : command{aCommand}, command_id{aCommandId}, handler{aHandler} {}
    inline bool equals(String* aCommand, String* aCommandId) { return aCommand == command && aCommandId == command_id; }

    String* command;
    String* command_id;
    Handler* handler;

}; // class Responder

class Responders : public std::vector<Responder>
{
 public:
    inline Responders() = default;

    inline void add(client::CommandData* aCommand, Handler* aHandler) { emplace_back(aCommand->get_C(), aCommand->get_D(), aHandler); }
    Handler* find(String* aCommand, String* aCommandId);
    void remove(String* aCommand, String* aCommandId);
    inline void reset() { clear(); }

}; // class Responders

// ----------------------------------------------------------------------

class Application
{
 public:
    inline Application() : mSession{new Session{}}, mLogin{nullptr}, mCommandId{0}, mConnectionClosedTimeStamp{0} {}
    virtual ~Application();

    virtual void send(client::CommandData* aCommand, Handler* aHandler);
    virtual void logged_in();

    Session* session() { return mSession; }

 protected:
    virtual void make_session(bool cancel_existing_session);
    virtual void make_connection();
    virtual void logout();
    virtual void reset();

 private:
    client::WebSocket* mWS;
    Session* mSession;
    Login* mLogin;
    size_t mCommandId;
    Responders mResponders;
    double mConnectionClosedTimeStamp; // to avoid reconnecting too often

    void on_message(client::RawMessage* aMessage); // parsed json from the server, may contain "E"
    void on_error(String* aData); // cannot parse response from the server
    void on_close(client::CloseEvent* aEvent);

    void on_hello(client::RawMessage* aMessage); // ws connection established

    void on_raw_message_event(client::MessageEvent* aEvent);

}; // class Application

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
