#pragma once

#include <thread>

#include "acmacs-webserver/server.hh"
#include "acmacs-api/client-connection.hh"
#include "acmacs-api/mongo-acmacs-c2-access.hh"

// ----------------------------------------------------------------------

class WsppThreadWithMongoAccess : public WsppThread
{
 public:
    WsppThreadWithMongoAccess(Wspp& aWspp, std::string aMongoURI, AcmacsC2& aAcmacsC2)
        : WsppThread{aWspp}, mongo_access_(aMongoURI, aAcmacsC2) {}

    auto& mongo_access() { return mongo_access_; }

 protected:
    void initialize() override;

 private:
    MongoAcmacsC2Access mongo_access_;

}; // class WsppThreadWithMongoAccess

// ----------------------------------------------------------------------

class CommandFactory;

class WebsocketConnection : public ClientConnection, public WsppWebsocketLocationHandler
{
 public:
    WebsocketConnection(CommandFactory& aCommandFactory)
        : ClientConnection{}, WsppWebsocketLocationHandler{}, mCommandFactory{aCommandFactory} {}
    WebsocketConnection(const WebsocketConnection& aSrc)
        : ClientConnection{aSrc}, WsppWebsocketLocationHandler{aSrc}, mCommandFactory{aSrc.mCommandFactory} {}

    void send(std::string aMessage, send_message_type aMessageType = send_message_type::text) override;
    std::ostream& log_send_receive() override { return WsppWebsocketLocationHandler::log_send_receive(); }

 protected:
    bool use(std::string aLocation) const override { return ClientConnection::use(aLocation); }
    void message(std::string aMessage, WsppThread& aThread) override;

    std::shared_ptr<WsppWebsocketLocationHandler> clone() const override
        {
            return std::make_shared<WebsocketConnection>(*this);
        }

    void opening(std::string, WsppThread& /*aThread*/) override;

    void after_close(std::string, WsppThread& /*aThread*/) override
        {
              //print_cerr("ClientConnection after_close");
        }

 private:
    CommandFactory& mCommandFactory;

}; // class WebsocketConnection

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
