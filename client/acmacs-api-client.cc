#include <cheerp/client.h>
#include <cheerp/clientlib.h>

#include <map>
#include <vector>

#include "string.hh"
#include "argv.hh"
#include "session.hh"

// ----------------------------------------------------------------------

namespace client
{

    struct EchoMessage : public Object {};

    struct UsersData : public CommandData
    {
        inline UsersData() : CommandData{"users"_S} {}
    };

}

// ----------------------------------------------------------------------

using namespace client;

void webMain();
static void on_load();

// ----------------------------------------------------------------------

class EchoResponder : public OnMessage<EchoMessage>
{
 public:
    using OnMessage::OnMessage;

 protected:
    virtual void process_message(EchoMessage* aMessage)
        {
            console_log("EchoResponder: ", aMessage);
        }
};

// ----------------------------------------------------------------------

void webMain()
{
    window.set_onload(cheerp::Callback(on_load));
    make_asm_definitions();
}

// ----------------------------------------------------------------------

inline Object* get_value(Object* obj, String&& key)
{
    return (*obj)[key];
}

// ----------------------------------------------------------------------

void on_load()
{
    console.log("acmacs-api-client");
      // var host_port = window.location.href.match(/https?:\/\/([^\/]+)/i)[1];
      // var ws = new WebSocket("wss://" + host_port + "/myws", "protocolOne");
    auto* ws = new WebSocket("wss://localhost:1169/api");
    login(ws, [](client::WebSocket* aWS) { return new EchoResponder{aWS}; });
    // ws->set_onmessage(cheerp::Callback(on_message));

    static_cast<EventTarget&>(window).set_("session", new Session{});
}


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
