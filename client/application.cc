#include <algorithm>

#include "application.hh"
#include "login.hh"
#include "argv.hh"

// ----------------------------------------------------------------------

namespace client
{
    struct Command_logout : public CommandData
    {
        inline Command_logout() : CommandData{"logout"_S} {}
    }; // struct Command_logout
} // namespace client

// ----------------------------------------------------------------------

Handler* Responders::find(String* aCommand, String* aCommandId)
{
    auto found = std::find_if(begin(), end(), [&](auto& responder) { return responder.equals(aCommand, aCommandId); });
    return found != end() ? found->handler : nullptr;

} // Responders::find

// ----------------------------------------------------------------------

void Responders::remove(String* aCommand, String* aCommandId)
{
    erase(std::remove_if(begin(), end(), [&](auto& responder) { return responder.equals(aCommand, aCommandId); }));

} // Responders::remove

// ----------------------------------------------------------------------

Application::~Application()
{
    client::console_error("~Application");

} // Application::~Application

// ----------------------------------------------------------------------

void Application::send(client::CommandData* aCommand, Handler* aHandler)
{
    aCommand->set_D(to_String(++mCommandId));
    if (aHandler)
        mResponders.add(aCommand, aHandler);
    mWS->send(to_String(aCommand));
    console_log("Application::send", aCommand);

} // Application::send

// ----------------------------------------------------------------------

void Application::make_session(bool cancel_existing_session)
{
    if (!mLogin)
        mLogin = new Login(this);
    mLogin->use_session(cancel_existing_session);

} // Application::make_session

// ----------------------------------------------------------------------

void Application::logout()
{
    send(new client::Command_logout{}, nullptr);
    make_session(true);
    session()->expired();
    reset();

} // Application::logout

// ----------------------------------------------------------------------

void Application::reset()
{
    mResponders.reset();
    if (mLogin)
        mLogin->reset();
    auto* local_storage = client::app_local_storage();
    if (is_not_null(local_storage))
        local_storage->removeItem(LocalStorageKeySession);

} // Application::reset

// ----------------------------------------------------------------------

void Application::make_connection()
{
    using namespace client;

    mWS = new WebSocket(concat("wss://", ws_host_port(), "/api"));
    mWS->set_onmessage(cheerp::Callback([this](MessageEvent* aEvent) { on_raw_message_event(aEvent); }));
    mWS->set_onclose(cheerp::Callback([this](CloseEvent* aEvent) { on_close(aEvent); }));

    // mWS->set_onopen(cheerp::Callback([]() { console_log("ws onopen"); }));
    mWS->set_onerror(cheerp::Callback([](Event* aEvent) { console_error("ws onerror", aEvent); }));

} // Application::make_connection

// ----------------------------------------------------------------------

void Application::on_message(client::RawMessage* aMessage)
{
    console_log("MSG: ", aMessage);
    if (!is_undefined_or_null(aMessage->get_C())) {
        auto* handler = mResponders.find(aMessage->get_C(), aMessage->get_D());
        if (handler) {
            mResponders.remove(aMessage->get_C(), aMessage->get_D());
            auto* err = aMessage->get_E();
            if (is_undefined_or_null(err))
                handler->on_message(aMessage);
            else
                handler->on_error(aMessage->get_E());
        }
        else {
            on_error(concat("Application: no handler: ", stringify(aMessage)));
        }
    }
    else if (!client::is_undefined_or_null(aMessage->get_hello())) {
        on_hello(aMessage);
    }
    else {
        on_error(concat("Application: unrecognized message: ", stringify(aMessage)));
    }

} // Application::on_message

// ----------------------------------------------------------------------

void Application::on_hello(client::RawMessage* aMessage) // ws connection established
{
    auto* server_version = aMessage->get_hello();
    if (eq("acmacs-api-server-v1", server_version)) {
        make_session(false);
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
    mWS = nullptr;
    constexpr const double minimum_delay_between_connections = 1000;
    const auto closed_time_stamp = aEvent->get_timeStamp();
    const auto diff = closed_time_stamp - mConnectionClosedTimeStamp - minimum_delay_between_connections;
    mConnectionClosedTimeStamp = closed_time_stamp;
    client::setTimeout(cheerp::Callback([this]() { make_connection(); }), diff < 0 ? 0 : minimum_delay_between_connections);

} // Application::on_close

// ----------------------------------------------------------------------

  // cannot parse response from the server or json contains "E"
void Application::on_error(String* aData)
{
    console_error("ERROR: ", aData);

} // Application::on_error

// ----------------------------------------------------------------------

void Application::logged_in()
{
    auto* local_storage = client::app_local_storage();
    if (is_not_null(local_storage))
        local_storage->setItem(LocalStorageKeySession, session()->id());
    client::console_log("Logged in: ", session());

} // Application::logged_in

// ----------------------------------------------------------------------

void Application::on_raw_message_event(client::MessageEvent* aEvent)
{
    auto* data = static_cast<String*>(aEvent->get_data());
    if (!is_undefined_or_null(data)) {
        auto* parsed = static_cast<client::RawMessage*>(client::JSON.parse(data));
        if (!is_undefined_or_null(parsed)) {
            on_message(parsed);
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
