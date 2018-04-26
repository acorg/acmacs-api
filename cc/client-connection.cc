#include "client-connection.hh"
#include "session.hh"

// ----------------------------------------------------------------------

// ClientConnection::~ClientConnection()
// {
//       // print_cerr("~ClientConnection ", this);

// } // ClientConnection::~ClientConnection

// ----------------------------------------------------------------------

void ClientConnection::make_session(mongocxx::database& aDb)
{
    if (!mSession)
        mSession = std::make_shared<Session>(aDb);

} // ClientConnection::make_session

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
