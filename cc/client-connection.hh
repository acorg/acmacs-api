#pragma once

#include <functional>

#include "acmacs-base/to-json.hh"

#include "mongo-access.hh"

// ----------------------------------------------------------------------

enum class send_message_type { text, binary };

// ----------------------------------------------------------------------

class Session;
class WsppThread;

class ClientConnection
{
 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    inline ClientConnection() = default;
    inline ClientConnection(const ClientConnection&) = default;
    virtual ~ClientConnection();

    virtual void send(std::string aMessage, send_message_type aMessageType = send_message_type::text) = 0;

    inline Session& session()
        {
            if (!mSession)
                throw Error{"no session"};
            return *mSession;
        }

    void make_session(mongocxx::database& aDb);

 protected:
    virtual inline bool use(std::string aLocation) const
        {
            return aLocation == "/api";
        }

 private:
    std::shared_ptr<Session> mSession;

}; // class ClientConnection

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
