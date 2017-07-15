#include <cheerp/client.h>
#include <cheerp/clientlib.h>

// ----------------------------------------------------------------------

using namespace client;

void webMain();
static void on_message(MessageEvent* aEvent);

// ----------------------------------------------------------------------

void webMain()
{
      // var host_port = window.location.href.match(/https?:\/\/([^\/]+)/i)[1];
      // var ws = new WebSocket("wss://" + host_port + "/myws", "protocolOne");

    console.log("acmacs-api-client");
    auto* ws = new WebSocket("wss://localhost:1169/api");
    ws->set_onmessage(cheerp::Callback(on_message));
}

// ----------------------------------------------------------------------

void on_message(MessageEvent* aEvent)
{
    console.log("on_message", aEvent->get_origin());
    console.log(JSON.stringify(aEvent->get_data()));
    auto& message = *JSON.parse(static_cast<String*>(aEvent->get_data()));
    auto version = static_cast<String*>(message[String{"hello"}]);
    console.log(version, version == new String{"acmacs-api-server-v1"});

} // on_message

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
