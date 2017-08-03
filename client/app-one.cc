#include <cheerp/client.h>
#include <cheerp/clientlib.h>

// #include <map>
// #include <vector>

// #include "login.hh"

#include "asm.hh"
#include "string.hh"
#include "session.hh"

#include "toolkit-basic.hh"

// ----------------------------------------------------------------------

static void on_load();

// ----------------------------------------------------------------------

void webMain()
{
    client::window.set_onload(cheerp::Callback(on_load));
    make_asm_definitions();
}

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

class ApplicationOne
{
 public:
    inline ApplicationOne() {}

    void run();

 private:
    client::WebSocket* mWS;
    // Session* mSession;

    void make_session();

    void on_message(client::RawMessage* aMessage); // parsed json from the server
    void on_error(String* aData); // cannot parse response from the server or json contains "E"
    void on_close(client::CloseEvent* aEvent);

    void on_hello(client::RawMessage* aMessage); // ws connection established

    void make_connection();
    void on_raw_message_event(client::MessageEvent* aEvent);

}; // class ApplicationOne

// ----------------------------------------------------------------------

void ApplicationOne::run()
{
    // mSession = new Session{};
    make_connection();

} // ApplicationOne::run

// ----------------------------------------------------------------------

void ApplicationOne::make_connection()
{
    using namespace client;

    mWS = new WebSocket(concat("wss://", ws_host_port(), "/api"));
    mWS->set_onmessage(cheerp::Callback([this](MessageEvent* aEvent) { on_raw_message_event(aEvent); }));
    mWS->set_onclose(cheerp::Callback([this](CloseEvent* aEvent) { on_close(aEvent); }));

    // mWS->set_onopen(cheerp::Callback([]() { console_log("ws onopen"); }));
    mWS->set_onerror(cheerp::Callback([]() { console_error("ws onerror"); }));

} // ApplicationOne::make_connection

// ----------------------------------------------------------------------

void ApplicationOne::on_message(client::RawMessage* aMessage)
{
    console_log("MSG: ", aMessage);
    if (!is_undefined_or_null(aMessage->get_C())) {
    }
    else if (!is_undefined_or_null(aMessage->get_hello())) {
        on_hello(aMessage);
    }
    else {
        on_error(concat("unrecognized message: ", stringify(aMessage)));
    }

} // ApplicationOne::on_message

// ----------------------------------------------------------------------

void ApplicationOne::on_hello(client::RawMessage* aMessage) // ws connection established
{
    auto* server_version = aMessage->get_hello();
    if (eq("acmacs-api-server-v1", server_version)) {
        make_session();
    }
    else {
        client::window.alert(concat("Unsupported server version: ", server_version));
        on_error(concat("Unsupported server version: ", server_version));
    }

} // ApplicationOne::on_hello

// ----------------------------------------------------------------------

void ApplicationOne::on_close(client::CloseEvent* aEvent)
{
    console_log("WS CLOSED: ", aEvent);

} // ApplicationOne::on_close

// ----------------------------------------------------------------------

  // cannot parse response from the server or json contains "E"
void ApplicationOne::on_error(String* aData)
{
    console_error("ERROR: ", aData);

} // ApplicationOne::on_error

// ----------------------------------------------------------------------

void ApplicationOne::on_raw_message_event(client::MessageEvent* aEvent)
{
    auto* data = static_cast<String*>(aEvent->get_data());
    if (!is_undefined_or_null(data)) {
        auto* parsed = static_cast<client::RawMessage*>(client::JSON.parse(data));
        if (!is_undefined_or_null(parsed)) {
            auto* err = parsed->get_E();
            if (is_undefined_or_null(err))
                on_message(parsed);
            else
                on_error(err);
        }
        else {
            on_error("Internal: cannot parse json message from server"_S);
        }
    }
    else {
        on_error("Internal: no data field in MessageEvent from server"_S);
    }

} // ApplicationOne::on_raw_message_event

// ----------------------------------------------------------------------

void ApplicationOne::make_session()
{
    client::console_log("make_session");

} // ApplicationOne::make_session

// ----------------------------------------------------------------------

void on_load()
{
    using namespace client;
    using namespace toolkit;

    console_log("app-one");

    append_child(document.get_body(), "div", text{"APP ONE"});
    ApplicationOne app;
    app.run();

    // static_cast<EventTarget&>(window).set_("session", new Session{});
}

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
