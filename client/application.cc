#include "application.hh"
#include "login.hh"

// ----------------------------------------------------------------------

Application::~Application()
{

} // Application::~Application

// ----------------------------------------------------------------------

void Application::send(client::CommandData* aCommand, Handler* aHandler)
{

} // Application::send

// ----------------------------------------------------------------------

void Application::make_session()
{
    if (!mSession->valid()) {
        client::console_log("make_session");
        mLogin = new Login(this);
        mLogin->run();
    }

} // Application::make_session

// ----------------------------------------------------------------------

void Application::make_connection()
{
    using namespace client;

    mWS = new WebSocket(concat("wss://", ws_host_port(), "/api"));
    mWS->set_onmessage(cheerp::Callback([this](MessageEvent* aEvent) { on_raw_message_event(aEvent); }));
    mWS->set_onclose(cheerp::Callback([this](CloseEvent* aEvent) { on_close(aEvent); }));

    // mWS->set_onopen(cheerp::Callback([]() { console_log("ws onopen"); }));
    mWS->set_onerror(cheerp::Callback([]() { console_error("ws onerror"); }));

} // Application::make_connection

// ----------------------------------------------------------------------

void Application::on_message(client::RawMessage* aMessage)
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

} // Application::on_message

// ----------------------------------------------------------------------

void Application::on_hello(client::RawMessage* aMessage) // ws connection established
{
    auto* server_version = aMessage->get_hello();
    if (eq("acmacs-api-server-v1", server_version)) {
        make_session();
    }
    else {
        client::window.alert(concat("Unsupported server version: ", server_version));
        on_error(concat("Unsupported server version: ", server_version));
    }

} // Application::on_hello

// ----------------------------------------------------------------------

void Application::on_close(client::CloseEvent* aEvent)
{
    console_log("WS CLOSED: ", aEvent);

} // Application::on_close

// ----------------------------------------------------------------------

  // cannot parse response from the server or json contains "E"
void Application::on_error(String* aData)
{
    console_error("ERROR: ", aData);

} // Application::on_error

// ----------------------------------------------------------------------

void Application::on_raw_message_event(client::MessageEvent* aEvent)
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

} // Application::on_raw_message_event

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
