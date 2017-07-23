#include "command.hh"
#include "session.hh"
#include "acmacs-api-server.hh"

// ----------------------------------------------------------------------

Session& Command::session()
{
    return mServer.session();
}

// ----------------------------------------------------------------------

void Command::send(std::string aMessage, websocketpp::frame::opcode::value op_code)
{
    std::cerr << "Command::send: " << aMessage << std::endl;
    mServer.send(json_object_prepend(aMessage, "C", command_name(), "CN", command_number(), "CT", command_duration()), op_code);

} // Command::send

// ----------------------------------------------------------------------

void Command::send_error(std::string aMessage)
{
    mServer.send(json_object("C", command_name(), "CN", command_number(), "E", aMessage));

} // Command::send_error

// ----------------------------------------------------------------------

mongocxx::database Command::db()
{
    return mServer.db();

} // Command::db

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
