#pragma once

#include "acmacs-webserver/server.hh"

#include "mongo-access.hh"
#include "session.hh"

// ----------------------------------------------------------------------

class CommandFactory;

class AcmacsAPIServer : public WsppWebsocketLocationHandler
{
 public:
    inline AcmacsAPIServer(mongocxx::pool& aPool, CommandFactory& aCommandFactory)
        : WsppWebsocketLocationHandler{}, mPool{aPool}, mCommandFactory{aCommandFactory}, mSession{db()}, mCommandNumber{0} {}
    inline AcmacsAPIServer(const AcmacsAPIServer& aSrc)
        : WsppWebsocketLocationHandler{aSrc}, mPool{aSrc.mPool}, mCommandFactory{aSrc.mCommandFactory}, mSession{aSrc.mSession}, mCommandNumber{0} {}

 protected:
    inline auto connection()
        {
            if (!mConnection)
                mConnection = mPool.acquire();
            return mConnection;
        }

    inline mongocxx::database db(const char* aName)
        {
            return (*connection())[aName];
        }

    inline mongocxx::database& db()
        {
            if (!mAcmacsWebDb)
                mAcmacsWebDb = db("acmacs_web");
            return mAcmacsWebDb;
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
    mongocxx::database mAcmacsWebDb;
    CommandFactory& mCommandFactory;
    std::shared_ptr<mongocxx::client> mConnection;
    Session mSession;
    std::atomic<size_t> mCommandNumber;

    inline Session& session() { return mSession; }

    friend class Command;

}; // class AcmacsAPIServer

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
