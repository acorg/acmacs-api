#pragma once

#include <cheerp/client.h>
#include <cheerp/clientlib.h>

#include "asm.hh"
#include "string.hh"

// ----------------------------------------------------------------------

template <typename MessageType> class OnMessage;

class OnMessageBase
{
 public:
    using TransferTo = OnMessageBase* (*)(client::WebSocket*);

    inline OnMessageBase(client::WebSocket* aWS) : mWS{aWS} {}
    OnMessageBase(const OnMessageBase&) = default;
    inline virtual ~OnMessageBase() {}

    inline void operator()(client::MessageEvent* aEvent)
        {
            auto data = static_cast<client::String*>(aEvent->get_data());
              //console.log("WaitingForHello::on_message", data);
            process_raw_message(client::JSON.parse(data));
        }

 protected:
    virtual void process_raw_message(client::Object* aMessage) = 0;

    inline void send(client::Object* aData)
        {
            auto* str = to_string(aData);
            console_log("Send: ", str);
            mWS->send(str);
        }

    inline void send(const char* aData)
        {
            mWS->send(new client::String{aData});
        }

    template <typename NewHandler, typename ... Args> inline void transfer(Args ... args)
        {
            mWS->set_onmessage(cheerp::Callback(NewHandler{mWS, args ...}));
        }

    template <typename NewHandler, typename ... Args> inline void transfer_send(client::Object* aMessage, Args ... args)
        {
            transfer<NewHandler>(args ...);
            send(aMessage);
        }

    // inline void transfer_to(OnMessageBase* aHandler)
    //     {
    //         mWS->set_onmessage(cheerp::Callback([&aHandler](client::MessageEvent* aEvent) { (*aHandler)(aEvent); }));
    //     }

    inline void transfer_to(TransferTo aHandlerMaker)
        {
            auto* handler = (*aHandlerMaker)(mWS);
            mWS->set_onmessage(cheerp::Callback([&handler](client::MessageEvent* aEvent) { (*handler)(aEvent); }));
        }

 private:
    client::WebSocket* mWS;

};

// ----------------------------------------------------------------------

template <typename MessageType> class OnMessage : public OnMessageBase
{
 public:
    using Message = MessageType;

    using OnMessageBase::OnMessageBase;


    inline void operator()(client::MessageEvent* aEvent)
        {
            auto data = static_cast<client::String*>(aEvent->get_data());
              //console.log("WaitingForHello::on_message", data);
            process_message(static_cast<MessageType*>(client::JSON.parse(data)));
        }

 protected:
    virtual inline void process_raw_message(client::Object* aMessage)
        {
              //!! handle aMessage["E"]
            process_message(static_cast<MessageType*>(aMessage));
        }

    virtual void process_message(MessageType* aMessage) = 0;

}; // OnMessage<>

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
