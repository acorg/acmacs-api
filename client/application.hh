#pragma once

#include "command.hh"
#include "session.hh"

// ----------------------------------------------------------------------

namespace client
{
    struct RawMessage : public Object
    {
        String* get_hello();
        String* get_C();
        String* get_D();
        String* get_E();

    }; // struct RawMessage

} // namespace client

// ----------------------------------------------------------------------

class Handler;
class Login;

class Application
{
 public:
    inline Application() : mSession{new Session{}}, mLogin{nullptr} {}
    virtual ~Application();

    void send(client::CommandData* aCommand, Handler* aHandler);

 protected:
    void make_session();
    void make_connection();

 private:
    client::WebSocket* mWS;
    Session* mSession;
    Login* mLogin;

    void on_message(client::RawMessage* aMessage); // parsed json from the server
    void on_error(String* aData); // cannot parse response from the server or json contains "E"
    void on_close(client::CloseEvent* aEvent);

    void on_hello(client::RawMessage* aMessage); // ws connection established

    void on_raw_message_event(client::MessageEvent* aEvent);

}; // class Application

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
