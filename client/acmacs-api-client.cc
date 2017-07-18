#include <cheerp/client.h>
#include <cheerp/clientlib.h>

#include <map>
#include <vector>

// ----------------------------------------------------------------------

namespace client
{
    struct Argv : public Object
    {
        String* get_S();
    };
    extern Argv* ARGV;
    Array* object_keys(Object*);
    Array* object_keys(Object&);

      // ----------------------------------------------------------------------

    struct HelloFromServer : public Object
    {
        String* get_hello();
    };

    struct EchoMessage : public Object {};
}

// ----------------------------------------------------------------------

using namespace client;

void webMain();
static void on_load();
// static void on_message(MessageEvent* aEvent);

// ----------------------------------------------------------------------

inline String* operator ""_S(const char* src, size_t) { return new String{src}; }

//inline String operator "" _s(const char* src, size_t) { return {src}; }
// inline String* operator + (String&& s1, String* s2) { return s1.concat(s2); }
// inline String* operator + (String& s1, String* s2) { return s1.concat(s2); }
// inline String* operator + (const char* s1, String& s2) { return String{s1}.concat(s2); }
// inline bool operator == (String&& s1, String* s2) { return &s1 == s2; }
// inline bool operator == (String* s1, String&& s2) { return s1 == &s2; }

// ----------------------------------------------------------------------

template <typename MessageType> class OnMessage
{
 public:
    inline OnMessage(WebSocket* aWS) : mWS{aWS} {}
    OnMessage(const OnMessage&) = default;
    inline virtual ~OnMessage() {}

    inline void operator()(MessageEvent* aEvent)
        {
            auto data = static_cast<String*>(aEvent->get_data());
              //console.log("WaitingForHello::on_message", data);
            process_message(static_cast<MessageType*>(JSON.parse(data)));
        }

 protected:
    virtual void process_message(MessageType* aMessage) = 0;
    inline void send(Object* aData) { mWS->send(aData); }
    inline void send(const char* aData) { mWS->send(new String{aData}); }

    template <typename NewHandler> inline void transfer() { mWS->set_onmessage(cheerp::Callback(NewHandler{mWS})); }

 private:
    WebSocket* mWS;
};

class EchoResponder : public OnMessage<EchoMessage>
{
 public:
    using OnMessage::OnMessage;

 protected:
    virtual void process_message(EchoMessage* aMessage)
        {
            console.log("EchoResponder: "_S->concat(JSON.stringify(aMessage)));
        }
};

class WaitingForHello : public OnMessage<HelloFromServer>
{
 public:
    using OnMessage::OnMessage;

 protected:
    virtual void process_message(HelloFromServer* aMessage)
        {
            auto server_version = aMessage->get_hello();
            if ("acmacs-api-server-v1"_S == server_version) {
                transfer<EchoResponder>();
                  // send(R"({"C": "echo", "V": "WaitingForHello"})");
                send(R"({"C": "users"})");
            }
            else {
                window.alert("Unsupported server version: "_S->concat(server_version));
            }
        }
};

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
    // ws->set_onmessage(cheerp::Callback(on_message));
    ws->set_onmessage(cheerp::Callback(WaitingForHello(ws)));
    console.log("ARGV", JSON.stringify(ARGV));
      //console.log("ARGV", JSON.stringify(ARGV), static_cast<String*>((*ARGV[String{"S"}])[0]));
    console.log("ARGV[S]", static_cast<String*>(ARGV->get_S()));

}

// void on_message(MessageEvent* aEvent)
// {
//     console.log("on_message", aEvent->get_origin());
//     console.log(JSON.stringify(aEvent->get_data()));
//     auto message_hello = static_cast<HelloFromServer*>(JSON.parse(static_cast<String*>(aEvent->get_data())));
//     console.log(String{"hello: "}.concat(message_hello->get_hello()));
//       //console.log("keys", JSON.stringify(jopa(message)));
//     // auto version = static_cast<String*>(get_value(message, "hello"));
//     // console.log(version, version == new String{"acmacs-api-server-v1"});

//     // for (const auto& arg: ARGV) {
//     //     console.log("ARGV"); //, arg.first);
//     // }

// } // on_message

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
