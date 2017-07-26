#include "command.hh"

// ----------------------------------------------------------------------

void Command::send(std::string aMessage, websocketpp::frame::opcode::value op_code)
{
    mServer.send(json_object_prepend(aMessage, "C", command_name(), "CN", command_number(), "CT", command_duration()), op_code);
    std::cerr << "Command::send: " << aMessage.substr(0, 100) << std::endl;

} // Command::send

// ----------------------------------------------------------------------

void Command::send_error(std::string aMessage)
{
    mServer.send(json_object("C", command_name(), "CN", command_number(), "E", aMessage));

} // Command::send_error

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
