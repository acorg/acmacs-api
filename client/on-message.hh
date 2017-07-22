#pragma once

#include <functional>

#include <cheerp/client.h>
#include <cheerp/clientlib.h>

#include "weak-tables-off.hh"
#include "asm.hh"
#include "string.hh"

// ----------------------------------------------------------------------

template <typename MessageType> class OnMessage;

class OnMessageBase
{
 public:
    using TransferTo = std::function<OnMessageBase* (client::WebSocket*)>;

    inline OnMessageBase(client::WebSocket* aWS) : mWS{aWS} {}
    OnMessageBase(const OnMessageBase&) = default;
    inline virtual ~OnMessageBase() {}

      // called upon transferring to this handler.
      // Could be used to send message.
    virtual void upon_transfer()
        {
        }

    inline void operator()(client::MessageEvent* aEvent)
        {
            auto data = static_cast<client::String*>(aEvent->get_data());
              //console.log("WaitingForHello::on_message", data);
            process_raw_message(client::JSON.parse(data));
        }

 protected:
    virtual void process_raw_message(client::Object* aMessage) = 0;

    inline void send(client::String* aData)
        {
            console_log("Send: ", aData);
            mWS->send(aData);
        }

    inline void send(client::Object* aData)
        {
            send(to_string(aData));
        }

    template <typename ... Args> inline void send(Args ... args)
        {
            send(make_json(args...));
        }

    inline void send(const char* aData)
        {
            mWS->send(new client::String{aData});
        }

    template <typename NewHandler> inline void transfer(NewHandler&& aHandler)
        {
            mWS->set_onmessage(cheerp::Callback(aHandler));
            aHandler.upon_transfer();
        }

    template <typename NewHandler, typename ... Args> inline void transfer(Args ... args)
        {
            transfer(NewHandler{mWS, args ...});
        }

    template <typename NewHandler, typename ... Args> inline void transfer_send(client::Object* aMessage, Args ... args)
        {
            transfer<NewHandler>(args ...);
            send(aMessage);
        }

    inline void transfer_to(TransferTo aHandlerMaker)
        {
            OnMessageBase& handler = *aHandlerMaker(mWS);
            mWS->set_onmessage(cheerp::Callback([&handler](client::MessageEvent* aEvent) { handler(aEvent); }));
            handler.upon_transfer();
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
