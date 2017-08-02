#pragma once

#include <functional>

#include <cheerp/client.h>
#include <cheerp/clientlib.h>

#include "weak-tables-off.hh"
#include "asm.hh"
#include "string.hh"
#include "session.hh"

// ----------------------------------------------------------------------

template <typename MessageType> class OnMessage;

class OnMessageBase : public client::Object
{
 public:
    using TransferTo = std::function<OnMessageBase* (client::WebSocket*)>;

    inline OnMessageBase(client::WebSocket* aWS) : mWS{aWS}, mCommandId{0} {}
    OnMessageBase(const OnMessageBase&) = default;
    inline virtual ~OnMessageBase() {}

      // called upon transferring to this handler.
      // Could be used to send message.
    virtual inline void upon_transfer()
        {
        }

    inline void operator()(client::MessageEvent* aEvent)
        {
            auto data = static_cast<String*>(aEvent->get_data());
              //console.log("WaitingForHello::on_message", data);
            process_raw_message(client::JSON.parse(data));
        }

    inline void set_onmessage()
        {
            mWS->set_onmessage(cheerp::Callback([this](client::MessageEvent* aEvent) { (*this)(aEvent); }));
        }

 protected:
    virtual void process_raw_message(client::Object* aMessage) = 0;

    // template <typename ... Args> inline void send(Args ... args)
    //     {
    //         send(make_json(args...));
    //     }

    inline void send(String* aData)
        {
            console_log("Send: ", aData);
            mWS->send(aData);
        }

    inline void send(client::CommandData* aCommand)
        {
            aCommand->set_D(to_String(++mCommandId));
            send(to_String(aCommand));
        }

    // inline void send(const char* aData)
    //     {
    //         send(to_String(aData));
    //     }

    template <typename NewHandler> inline void transfer(NewHandler* aHandler)
        {
            aHandler->set_onmessage();
            aHandler->upon_transfer();
        }

    template <typename NewHandler, typename ... Args> inline void transfer(Args&& ... args)
        {
            transfer(new NewHandler{mWS, std::forward<Args>(args) ...});
        }

    template <typename NewHandler, typename ... Args> inline void transfer_send(client::CommandData* aCommand, Args&& ... args)
        {
            transfer<NewHandler>(std::forward<Args>(args) ...);
            send(aCommand);
        }

    inline void transfer_to(TransferTo aHandlerMaker)
        {
            transfer(aHandlerMaker(mWS));
        }

 private:
    client::WebSocket* mWS;
    size_t mCommandId;

}; // class OnMessageBase

// ----------------------------------------------------------------------

template <typename MessageType> class OnMessage : public OnMessageBase
{
 public:
    using Message = MessageType;

    using OnMessageBase::OnMessageBase;

 protected:
    virtual inline void process_raw_message(client::Object* aMessage)
        {
            console_log("raw-message", aMessage);
            auto msg = static_cast<MessageType*>(aMessage);
            auto* err = msg->get_E();
            if (is_undefined_or_null(err)) {
                process_message(msg);
            }
            else {
                process_error(err);
            }
        }

    virtual void process_message(MessageType* aMessage) = 0;

    virtual inline void process_error(String* aError)
        {
            console_error("ERROR:", aError);
        }

}; // OnMessage<>

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
