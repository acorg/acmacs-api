#pragma once

#include <thread>

#include "acmacs-webserver/server.hh"

#include "client-connection.hh"
#include "mongo-acmacs-c2-access.hh"

// ----------------------------------------------------------------------

class WsppThreadWithMongoAccess : public WsppThread, public MongoAcmacsC2Access
{
 public:
    inline WsppThreadWithMongoAccess(Wspp& aWspp, std::string aMongoURI, AcmacsC2& aAcmacsC2)
        : WsppThread{aWspp}, MongoAcmacsC2Access{aMongoURI, aAcmacsC2} {}

 protected:
    virtual void initialize();

}; // class WsppThreadWithMongoAccess

// ----------------------------------------------------------------------

class CommandFactory;

class BrowserConnection : public ClientConnection, public WsppWebsocketLocationHandler
{
 public:
    inline BrowserConnection(CommandFactory& aCommandFactory)
        : ClientConnection{}, WsppWebsocketLocationHandler{}, mCommandFactory{aCommandFactory} {}
    inline BrowserConnection(const BrowserConnection& aSrc)
        : ClientConnection{aSrc}, WsppWebsocketLocationHandler{aSrc}, mCommandFactory{aSrc.mCommandFactory} {}

    virtual void send(std::string aMessage, send_message_type aMessageType = send_message_type::text);

 protected:
    virtual inline bool use(std::string aLocation) const { return ClientConnection::use(aLocation); }
    virtual void message(std::string aMessage, WsppThread& aThread);

    virtual std::shared_ptr<WsppWebsocketLocationHandler> clone() const
        {
            return std::make_shared<BrowserConnection>(*this);
        }

    virtual inline void opening(std::string, WsppThread& /*aThread*/)
        {
            send(to_json::object("hello", "acmacs-api-server-v1"));
        }

    virtual void after_close(std::string, WsppThread& /*aThread*/)
        {
              //print_cerr("ClientConnection after_close");
        }

 private:
    CommandFactory& mCommandFactory;

}; // class BrowserConnection

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
