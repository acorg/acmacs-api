#include <cheerp/client.h>
#include <cheerp/clientlib.h>

#include <map>
#include <vector>

#include "string.hh"
#include "argv.hh"

// ----------------------------------------------------------------------

namespace client
{

    String* md5(String*);

      // ----------------------------------------------------------------------

    struct Session : public Object
    {
        String* get_id();
        void set_id(String*);
        String* get_user();
        void set_user(String*);
        String* get_display_name();
        void set_display_name(String*);
    };

    extern Session* session;

      // ----------------------------------------------------------------------

    struct HelloFromServer : public Object
    {
        String* get_hello();
    };

    struct EchoMessage : public Object {};

    struct CommandData : public Object
    {
        inline CommandData(String* aCmd) { set_C(aCmd) ; }
        void set_C(String*);
    };

    struct UsersData : public CommandData
    {
        inline UsersData() : CommandData{"users"_S} {}
    };

    // struct LoginData : public CommandData
    // {
    //     inline LoginData(String* aS) : CommandData{"login"_S} { set_S(aS); }
    //     void set_S(String*);
    //     void set_U(String*);
    //     void set_P(String*);
    // };

    struct LoginSessionData : public CommandData
    {
        inline LoginSessionData(String* aS) : CommandData{"login_session"_S} { set_S(aS); }
        void set_S(String*);
    };

    struct ResponseData : public Object
    {
        String* get_R();
        String* get_E();
    };

    struct GetNonceCommandData : public CommandData
    {
        inline GetNonceCommandData(String* aUser) : CommandData{"login_nonce"_S} { set_user(aUser); }
        void set_user(String*);
    };

    struct LoginPasswordCommandData : public CommandData
    {
        inline LoginPasswordCommandData(String* cnonce, String* digest) : CommandData{"login_digest"_S} { set_cnonce(cnonce); set_digest(digest); }
        void set_cnonce(String*);
        void set_digest(String*);
    };

    struct LoginNonceData : public ResponseData
    {
        String* get_login_nonce();
    };

    struct LoggedInData : public ResponseData
    {
        String* get_S();
        String* get_user();
        String* get_display_name();
    };
}

// ----------------------------------------------------------------------

using namespace client;

void webMain();
static void on_load();
// static void on_message(MessageEvent* aEvent);

// ----------------------------------------------------------------------

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

    inline void send(Object* aData)
        {
            if (!is_string(aData))
                aData = JSON.stringify(aData, cheerp::Callback(&stringify_replacer));
            console_log("Send: ", aData);
            mWS->send(aData);
        }

    inline void send(const char* aData)
        {
            mWS->send(new String{aData});
        }

    template <typename NewHandler> inline void transfer()
        {
            mWS->set_onmessage(cheerp::Callback(NewHandler{mWS}));
        }

    template <typename NewHandler> inline void transfer_send(Object* aMessage)
        {
            transfer<NewHandler>();
            send(aMessage);
        }

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
            console_log("EchoResponder: ", aMessage);
        }
};

class LoggedIn : public OnMessage<LoggedInData>
{
 public:
    using OnMessage::OnMessage;

 protected:
    virtual void process_message(LoggedInData* aMessage)
        {
            if (!is_undefined(aMessage->get_E()) || aMessage->get_R() != "session"_S) {
                window.alert(concat("Login failed: ", aMessage->get_E()));
            }
            else {
                session->set_id(aMessage->get_S());
                session->set_user(aMessage->get_user());
                session->set_display_name(aMessage->get_display_name());
                console_log("Logged in: ", session->get_user(), session->get_display_name());
            }
            send("{\"C\":\"echo\"}");
            transfer<EchoResponder>();
        }
};

class LoginNonce : public OnMessage<LoginNonceData>
{
 public:
    using OnMessage::OnMessage;

 protected:
    virtual void process_message(LoginNonceData* aMessage)
        {
            if (is_undefined(aMessage->get_E())) {
                auto* snonce = aMessage->get_login_nonce();
                auto* cnonce = make_cnonce();
                auto* digest_password = md5(concat(ARGV->user(), ";acmacs-web;", ARGV->password()));
                auto* digest = md5(concat(snonce, ";", cnonce, ";", digest_password));
                transfer_send<LoggedIn>(new LoginPasswordCommandData{cnonce, digest});
            }
            else {
                window.alert(concat("Login failed: ", aMessage->get_E()));
            }
        }
};

class Login : public OnMessage<HelloFromServer>
{
 public:
    using OnMessage::OnMessage;

 protected:
    virtual void process_message(HelloFromServer* aMessage)
        {
            auto server_version = aMessage->get_hello();
            if ("acmacs-api-server-v1"_S == server_version) {
                login();
                  // transfer_send<EchoResponder>(new UsersData{});
            }
            else {
                window.alert(concat("Unsupported server version: ", server_version));
            }
        }

    void login()
        {
            if (!is_undefined(ARGV->session())) {
                transfer_send<LoggedIn>(new LoginSessionData{ARGV->session()});
            }
            else if (!is_undefined(ARGV->user())) {
                transfer_send<LoginNonce>(new GetNonceCommandData{ARGV->user()});
            }
            else {
                window.alert("Cannot login: no cridentials");
            }
        }

    // virtual void process_message(HelloFromServer* aMessage)
    //     {
    //         auto server_version = aMessage->get_hello();
    //         if ("acmacs-api-server-v1"_S == server_version) {
    //             transfer<EchoResponder>();
    //               // send(R"({"C": "echo", "V": "Login"})");
    //             send(R"({"C": "users"})");
    //         }
    //         else {
    //             window.alert(concat("Unsupported server version: ", server_version));
    //         }
    //     }
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
    // ws->set_onmessage(cheerp::Callback(on_message));
    ws->set_onmessage(cheerp::Callback(Login(ws)));
    console.log("ARGV", JSON.stringify(ARGV));
      //console.log("ARGV", JSON.stringify(ARGV), static_cast<String*>((*ARGV[String{"S"}])[0]));
    console.log("ARGV[S]", JSON.stringify(ARGV->get_S()));

    static_cast<EventTarget&>(window).set_("session", new Session{});
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
