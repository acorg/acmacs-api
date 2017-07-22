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

    struct CommandUsers : public CommandData
    {
        inline CommandUsers() : CommandData{"users"_S} {}
    };

    // struct ResultUsers : public ResponseData
    // {
    // };
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

    virtual void upon_transfer()
        {
            // this->send("C", "echo");
            // this->send("C", "echo", "handler", "EchoResponder");
            this->send(make_json("C", "users"));
        }

 protected:
    virtual void process_message(EchoMessage* aMessage)
        {
            console_log("EchoResponder: ", aMessage);
        }
};

// ----------------------------------------------------------------------

class JsonPrinter : public OnMessage<ResponseData>
{
 public:
    using OnMessage::OnMessage;

    virtual void upon_transfer()
        {
            this->send(new CommandUsers{}); // "C", "users");
        }

 protected:
    virtual void process_message(ResponseData* aMessage)
        {
            console_log("WHOA");
            auto* pre = document.createElement("pre");
            pre->set_textContent(aMessage->get_R());
            document.get_body()->appendChild(pre);
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
    login(ws, [](client::WebSocket* aWS) { return new JsonPrinter{aWS}; });

    static_cast<EventTarget&>(window).set_("session", new Session{});
}


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
