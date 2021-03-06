#include <cheerp/client.h>
#include <cheerp/clientlib.h>

#include <map>
#include <vector>

#include "string.hh"
#include "session.hh"
#include "login.hh"

// ----------------------------------------------------------------------

namespace client
{

    // inline String* to_String(const char* src) { return new String{src}; }

    struct EchoMessage : public Object {};

    struct HelloFromServer : public ResponseData
    {
        String* get_hello();
    };

    struct Command_users : public CommandData
    {
        inline Command_users() : CommandData{"users"_S} {}
    };

    struct Command_chart : public CommandData
    {
        template <typename IdType> inline Command_chart(IdType id) : CommandData{"chart"_S} { set_id(id); }
        inline void set_id(const char* id) { set_id(to_String(id)); }
        void set_id(String*);
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

    struct Command_chains : public CommandData
    {
        inline Command_chains(int skip = 0, int limit = 0, int chunk_size = 0)
            : CommandData{"chains"_S}
            { set_skip(skip); set_limit(limit); set_chunk_size(chunk_size); }

        template <typename ... Args> inline Command_chains* owners(Args ... args) { set_owners(to_Array_String(args ...)); return this; }
        template <typename ... Args> inline Command_chains* keywords(Args ... args) { set_keywords(to_Array_String(args ...)); return this; }
        template <typename ... Args> inline Command_chains* search(Args ... args) { set_search(to_Array_String(args ...)); return this; }
        template <typename ... Args> inline Command_chains* types(Args ... args) { set_types(to_Array_String(args ...)); return this; }

        void set_chunk_size(int);
        void set_skip(int);
        void set_limit(int);
        void set_owners(Array*);
        void set_keywords(Array*);
        void set_search(Array*);
        void set_types(Array*);
    };
    struct Command_chain_keywords : public CommandData
    {
        inline Command_chain_keywords() : CommandData{"chain_keywords"_S} {}
        inline Command_chain_keywords* include_rd_keywords(bool include) { set_include_rd_keywords(include); return this; }
        void set_include_rd_keywords(bool);
    };
    struct Command_chain_owners : public CommandData { inline Command_chain_owners() : CommandData{"chain_owners"_S} {} };
    struct Command_chain_types : public CommandData { inline Command_chain_types() : CommandData{"chain_types"_S} {} };

    struct Command_list_commands : public CommandData { inline Command_list_commands() : CommandData{"list_commands"_S} {} };

    // struct ResultUsers : public ResponseData
    // {
    // };
}

// ----------------------------------------------------------------------

using namespace client;

void webMain();
static void on_load();
static void start(client::WebSocket* aWS);

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
            // this->send(new Command_list_commands{});

            // this->send(new Command_chain_types{});
            // this->send(new Command_chain_owners{});
            // this->send(new Command_chain_keywords{});
            // this->send((new Command_chains{})->owners("whocc-tables")->types("acmacs.inspectors.routine_diagnostics.IncrementalChain", "acmacs.inspectors.routine_diagnostics.IncrementalChainForked"));

            this->send(new Command_chart{"593a87ee48618fc1e72da4fe"});
            // for (int i = 0; i < 100; ++i) {
            //     this->send(new Command_chart_keywords{});
            //     this->send(new Command_chart_owners{});
            // }
              //this->send((new Command_root_charts{})->owners("eu")->search("turkey")->keywords("individual")); // ->owners("alpha")->keywords("individual")->search("labels", "TURKEY")

              // this->send(new Command_users{});
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
      // login(ws, [](client::WebSocket* aWS) { return new JsonPrinter{aWS}; });
    start(ws);
    static_cast<EventTarget&>(window).set_("session", new Session{});
}

// ----------------------------------------------------------------------

class GetHello : public OnMessage<client::HelloFromServer>
{
 public:
    inline GetHello(client::WebSocket* aWS, TransferTo aTransferTo)
        : OnMessage<Message>{aWS}, mTransferTo{aTransferTo} {}
      // using Message = client::LoginData;

 protected:
    virtual void process_message(Message* aMessage);

 private:
    TransferTo mTransferTo;

    // inline void transfer_to()
    //     {
    //         LoginStep<client::LoggedInData>::transfer_to(this->pass_transfer_to());
    //     }

}; // class LoggedIn

// ----------------------------------------------------------------------

void GetHello::process_message(Message* aMessage)
{
    auto server_version = aMessage->get_hello();
    if (eq("acmacs-api-server-v1", server_version)) {
        transfer<Login>(mTransferTo);

    }
    else {
        window.alert(concat("Unsupported server version: ", server_version));
    }

} // GetHello::process_message

// ----------------------------------------------------------------------

void start(client::WebSocket* aWS)
{
    auto* get_hello = new GetHello(aWS, [](client::WebSocket* ws) { return new JsonPrinter{ws}; });
    get_hello->set_onmessage();

} // start

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
