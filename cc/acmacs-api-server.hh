#pragma once

#include <thread>

#include "acmacs-webserver/server.hh"

#include "mongo-access.hh"
#include "session.hh"
#include "send-func.hh"
#include "print.hh"

// ----------------------------------------------------------------------

class CommandFactory;

class AcmacsAPIServer : public WsppWebsocketLocationHandler
{
 public:
    inline AcmacsAPIServer(CommandFactory& aCommandFactory)
        : WsppWebsocketLocationHandler{}, mCommandFactory{aCommandFactory} {}
    inline AcmacsAPIServer(const AcmacsAPIServer& aSrc)
        : WsppWebsocketLocationHandler{aSrc}, mCommandFactory{aSrc.mCommandFactory} {}

    inline Session& session(mongocxx::database aDb)
        {
            if (!mSession) {
                mSession = std::make_unique<Session>(aDb);
            }
            return *mSession;
        }

    inline void send(std::string aMessage, send_message_type aMessageType = send_message_type::text)
        {
            auto op_code = websocketpp::frame::opcode::text;
            switch (aMessageType) {
              case send_message_type::text:
                  op_code = websocketpp::frame::opcode::text;
                  break;
              case send_message_type::binary:
                  op_code = websocketpp::frame::opcode::binary;
                  break;
            }
            print2("SEND: ", aMessage);
            WsppWebsocketLocationHandler::send(aMessage, op_code);
        }

 protected:
    virtual std::shared_ptr<WsppWebsocketLocationHandler> clone() const
        {
            return std::make_shared<AcmacsAPIServer>(*this);
        }

    virtual inline bool use(std::string aLocation) const
        {
            return aLocation == "/api";
        }

    virtual inline void opening(std::string, WsppThread& /*aThread*/)
        {
            send(json_object("hello", "acmacs-api-server-v1"));
        }

    virtual void message(std::string aMessage, WsppThread& aThread);

    virtual void after_close(std::string, WsppThread& /*aThread*/)
        {
              //print1("AcmacsAPIServer after_close");
        }

 private:
    CommandFactory& mCommandFactory;
    mongocxx::database mAcmacsWebDb;
    std::unique_ptr<Session> mSession;

}; // class AcmacsAPIServer

// ----------------------------------------------------------------------

class WsppThreadWithMongoAccess : public WsppThread
{
 public:
    inline WsppThreadWithMongoAccess(Wspp& aWspp, std::string aMongoURI)
        : WsppThread{aWspp}, mMongoURI{aMongoURI} {}

    auto& client() { return mClient; }

 protected:
    virtual void initialize();

 private:
    mongocxx::client mClient;
    std::string mMongoURI;

}; // class WsppThreadWithMongoAccess

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
