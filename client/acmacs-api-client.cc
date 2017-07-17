#include <cheerp/client.h>
#include <cheerp/clientlib.h>

#include <map>
#include <vector>

// ----------------------------------------------------------------------

namespace client
{
      //using Argv = std::map<String, std::vector<String>>;
    extern Object& ARGV;
    Array* object_keys(Object*);
    Array* object_keys(Object&);
}


using namespace client;

void webMain();
static void on_load();
static void on_message(MessageEvent* aEvent);

// ----------------------------------------------------------------------

void webMain()
{
    window.set_onload(cheerp::Callback(on_load));

    __asm__("window.object_keys = Object.keys;");
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
    ws->set_onmessage(cheerp::Callback(on_message));
    console.log("ARGV", JSON.stringify(ARGV), static_cast<String*>((*ARGV[String{"S"}])[0]));

}

void on_message(MessageEvent* aEvent)
{
    console.log("on_message", aEvent->get_origin());
    console.log(JSON.stringify(aEvent->get_data()));
    auto message = JSON.parse(static_cast<String*>(aEvent->get_data()));
      //console.log("keys", JSON.stringify(jopa(message)));
    auto version = static_cast<String*>(get_value(message, "hello"));
    console.log(version, version == new String{"acmacs-api-server-v1"});

    // for (const auto& arg: ARGV) {
    //     console.log("ARGV"); //, arg.first);
    // }

} // on_message

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
