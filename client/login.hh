#pragma once

#include "command.hh"
#include "on-message.hh"

// ----------------------------------------------------------------------

namespace client
{
    struct LoginSessionData : public CommandData
    {
        inline LoginSessionData(String* aS) : CommandData{"login_session"_S} { set_S(aS); }
        void set_S(String*);
    };

    // struct LoginNonceData : public ResponseData
    // {
    //     String* get_login_nonce();
    // };

    // struct LoggedInData : public ResponseData
    // {
    //     String* get_S();
    //     String* get_user();
    //     String* get_display_name();
    // };

    struct LoginData : public ResponseData
    {
        String* get_login_nonce();
        String* get_S();
        String* get_user();
        String* get_display_name();
    };

} // namespace client

// ----------------------------------------------------------------------

class LoginWidget;

class Login : public OnMessage<client::LoginData>
{
 public:
    inline Login(client::WebSocket* aWS, OnMessageBase::TransferTo aTransferTo)
        : OnMessage<Message>{aWS}, mTransferTo{aTransferTo}, mWidget{nullptr}
        {
            client::console_log("Login");
        }
      //inline Login(const Login&) = default;
    inline ~Login() { client::console_log("~Login"); }

    virtual void upon_transfer();

 protected:
    virtual void process_message(Message* aMessage);
    virtual void process_error(String* aError);

 private:
    TransferTo mTransferTo;
    LoginWidget* mWidget;
    String* mUser;
    String* mPassword;

    friend class LoginWidget;

    void initiate_login(String* aUser, String* aPassword);

}; // class Login

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
