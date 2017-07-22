#pragma once

#include <cheerp/clientlib.h>

#include "command.hh"
#include "on-message.hh"

// ----------------------------------------------------------------------

namespace client
{
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

      // --------------------------------------------------

    struct HelloFromServer : public ResponseData
    {
        String* get_hello();
    };

    struct LoginSessionData : public CommandData
    {
        inline LoginSessionData(String* aS) : CommandData{"login_session"_S} { set_S(aS); }
        void set_S(String*);
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

} // namespace client

// ----------------------------------------------------------------------

template <typename MessageType> class LoginStep : public OnMessage<MessageType>
{
 public:
    using Message = MessageType;

    inline LoginStep(client::WebSocket* aWS, OnMessageBase::TransferTo aTransferTo) : OnMessage<MessageType>{aWS}, mTransferTo{aTransferTo} {}

 protected:
    inline OnMessageBase::TransferTo pass_transfer_to() { return mTransferTo; }

 private:
    OnMessageBase::TransferTo mTransferTo;
};

// ----------------------------------------------------------------------

class LoggedIn : public LoginStep<client::LoggedInData>
{
 public:
    using Message = client::LoggedInData;
    using LoginStep<Message>::LoginStep;

 protected:
    virtual void process_message(Message* aMessage);

 private:
    inline void transfer_to()
        {
            LoginStep<client::LoggedInData>::transfer_to(this->pass_transfer_to());
        }
};

// ----------------------------------------------------------------------

class LoginNonce : public LoginStep<client::LoginNonceData>
{
 public:
    using Message = client::LoginNonceData;
    using LoginStep<Message>::LoginStep;

 protected:
    virtual void process_message(Message* aMessage);
};

// ----------------------------------------------------------------------

class Login : public LoginStep<client::HelloFromServer>
{
 public:
    using Message = client::HelloFromServer;
    using LoginStep<Message>::LoginStep;

 protected:
    virtual void process_message(Message* aMessage);
};

// ----------------------------------------------------------------------

void login(client::WebSocket* aWS, OnMessageBase::TransferTo aTransferTo);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
