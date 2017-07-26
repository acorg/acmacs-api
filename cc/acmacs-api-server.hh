#pragma once

#include <thread>

#include "acmacs-webserver/server.hh"

#include "mongo-access.hh"
#include "session.hh"
#include "send-func.hh"

// ----------------------------------------------------------------------

class CommandFactory;

class AcmacsAPIServer : public WsppWebsocketLocationHandler
{
 public:
    inline AcmacsAPIServer(mongocxx::pool& aPool, CommandFactory& aCommandFactory)
        : WsppWebsocketLocationHandler{}, mPool{aPool}, mCommandFactory{aCommandFactory} {}
    inline AcmacsAPIServer(const AcmacsAPIServer& aSrc)
        : WsppWebsocketLocationHandler{aSrc}, mPool{aSrc.mPool}, mCommandFactory{aSrc.mCommandFactory} {}

    inline Session& session()
        {
            if (!mSession)
                mSession = std::make_unique<Session>(db());
            return *mSession;
        }

    inline mongocxx::database& db()
        {
            if (!mAcmacsWebDb)
                mAcmacsWebDb = db("acmacs_web");
            std::cerr << std::this_thread::get_id() << " acmacs_web db" << std::endl;
            return mAcmacsWebDb;
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
            std::cout << std::this_thread::get_id() << " SEND: " << aMessage << std::endl;
            WsppWebsocketLocationHandler::send(aMessage, op_code);
        }

 protected:
    inline auto connection()
        {
            std::cerr << std::this_thread::get_id() << " mongo connection" << std::endl;
            return mPool.acquire();
        }

    inline mongocxx::database db(const char* aName)
        {
            return (*connection())[aName];
        }

    virtual std::shared_ptr<WsppWebsocketLocationHandler> clone() const
        {
            return std::make_shared<AcmacsAPIServer>(*this);
        }

    virtual inline bool use(std::string aLocation) const
        {
            return aLocation == "/api";
        }

    virtual inline void opening(std::string)
        {
            send(json_object("hello", "acmacs-api-server-v1"));
        }

    virtual void message(std::string aMessage);

    virtual void after_close(std::string)
        {
              //std::cout << std::this_thread::get_id() << " MyWS after_close" << std::endl;
        }

 private:
    mongocxx::pool& mPool;
    CommandFactory& mCommandFactory;
    mongocxx::database mAcmacsWebDb;
    std::unique_ptr<Session> mSession;

      // friend class Command;
    // friend class WsppThreadWithMongoAccess;

}; // class AcmacsAPIServer

// ----------------------------------------------------------------------

// class WsppThreadWithMongoAccess : public WsppThread
// {
//  public:
//     static inline WsppThread* make(Wspp& aWspp, AcmacsAPIServer& aAPIServer) { return new WsppThreadWithMongoAccess{aWspp, aAPIServer}; }

//  protected:
//     inline WsppThreadWithMongoAccess(Wspp& aWspp, AcmacsAPIServer& aAPIServer)
//         : WsppThread{aWspp}, mAPIServer{aAPIServer} {}

//     virtual void initialize();

//  private:
//     AcmacsAPIServer& mAPIServer;
//     mongocxx::database mAcmacsWebDb;

// }; // class WsppThreadWithMongoAccess

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
