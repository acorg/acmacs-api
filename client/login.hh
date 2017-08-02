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

// template <typename MessageType> class LoginStep : public OnMessage<MessageType>
// {
//  public:
//     using Message = MessageType;

//     inline LoginStep(client::WebSocket* aWS, OnMessageBase::TransferTo aTransferTo) : OnMessage<MessageType>{aWS}, mTransferTo{aTransferTo} {}

//  protected:
//     inline OnMessageBase::TransferTo pass_transfer_to() { return mTransferTo; }

//  private:
//     OnMessageBase::TransferTo mTransferTo;

// }; // class LoginStep<>

// ----------------------------------------------------------------------

// class LoggedIn : public LoginStep<client::LoginData>
// {
//  public:
//     using Message = client::LoginData;
//     using LoginStep<Message>::LoginStep;

//  protected:
//     virtual void process_message(Message* aMessage);

//  private:
//     inline void transfer_to()
//         {
//             LoginStep<client::LoginData>::transfer_to(this->pass_transfer_to());
//         }

// }; // class LoggedIn

// // ----------------------------------------------------------------------

// class LoginNonce : public LoginStep<client::LoginData>
// {
//  public:
//     using Message = client::LoginData;
//     using LoginStep<Message>::LoginStep;

//     inline LoginNonce(client::WebSocket* aWS, String* aUser, String* aPassword, OnMessageBase::TransferTo aTransferTo) : LoginStep<Message>{aWS, aTransferTo}, mUser{aUser}, mPassword{aPassword} {}

//  protected:
//     virtual void process_message(Message* aMessage);

//  private:
//     String* mUser;
//     String* mPassword;

// }; // class LoginNonce

// ----------------------------------------------------------------------

class LoginWidget;

class Login : public OnMessage<client::LoginData>
{
 public:
    inline Login(client::WebSocket* aWS, OnMessageBase::TransferTo aTransferTo)
        : OnMessage<Message>{aWS}, mTransferTo{aTransferTo}, mWidget{nullptr}
        {
              // client::console_log("Login");
        }
      //inline Login(const Login&) = default;
      // inline ~Login() { client::console_log("~Login", mNo); }

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
