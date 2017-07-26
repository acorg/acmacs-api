#include <iostream>

#include "command.hh"

// ----------------------------------------------------------------------

void Command::send(std::string aMessage, send_message_type aMessageType)
{
    mSendFunc(json_object_prepend(aMessage, "C", command_name(), "CN", command_number(), "CT", command_duration()), aMessageType);
      // std::cerr << "Command::send: " << aMessage.substr(0, 100) << std::endl;

} // Command::send

// ----------------------------------------------------------------------

void Command::send_error(std::string aMessage)
{
    mSendFunc(json_object("C", command_name(), "CN", command_number(), "E", aMessage), send_message_type::text);

} // Command::send_error

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
