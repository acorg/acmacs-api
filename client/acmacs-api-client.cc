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

    // inline String* to_String(const char* src) { return new String{src}; }

    struct EchoMessage : public Object {};

    struct Command_users : public CommandData
    {
        inline Command_users() : CommandData{"users"_S} {}
    };

    struct Command_root_charts : public CommandData
    {
        inline Command_root_charts(int skip = 0, int limit = 0, int chunk_size = 0)
            : CommandData{"root_charts"_S}
            { set_skip(skip); set_limit(limit); set_chunk_size(chunk_size); }

        template <typename ... Args> inline Command_root_charts* owners(Args ... args) { set_owners(to_Array_String(args ...)); return this; }
        template <typename ... Args> inline Command_root_charts* keywords(Args ... args) { set_keywords(to_Array_String(args ...)); return this; }
        template <typename ... Args> inline Command_root_charts* search(Args ... args) { set_search(to_Array_String(args ...)); return this; }

        void set_chunk_size(int);
        void set_skip(int);
        void set_limit(int);
        void set_owners(Array*);
        void set_keywords(Array*);
        void set_search(Array*);
    };

    struct Command_chart_keywords : public CommandData { inline Command_chart_keywords() : CommandData{"chart_keywords"_S} {} };
    struct Command_chart_owners : public CommandData { inline Command_chart_owners() : CommandData{"chart_owners"_S} {} };
    struct Command_list_commands : public CommandData { inline Command_list_commands() : CommandData{"list_commands"_S} {} };

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
            this->send(new Command_list_commands{});
              // this->send(new Command_users{});
            this->send(new Command_chart_keywords{});
            this->send(new Command_chart_owners{});
            this->send((new Command_root_charts{})->owners("eu")->search("turkey")->keywords("individual")); // ->owners("alpha")->keywords("individual")->search("labels", "TURKEY")
        }

 protected:
    virtual void process_message(ResponseData* aMessage)
        {
            auto* pre = document.createElement("pre");
            pre->set_className("json-highlight material-design-box-shadow");
            pre->set_innerHTML(json_syntax_highlight(stringify(aMessage, 2)));
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
