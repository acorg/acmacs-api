#include <iostream>

#include "acmacs-webserver/print.hh"

#include "command.hh"
#include "mongo-acmacs-c2-access.hh"

// ----------------------------------------------------------------------

Command::Command(from_json::object&& aSrc, MongoAcmacsC2Access& aMongoAccess, ClientConnection& aClientConnection, size_t aCommandNumber)
    : from_json::object{std::move(aSrc)}, mDb{aMongoAccess.client()["acmacs_web"]}, mClientConnection{aClientConnection}, mCommandNumber{aCommandNumber}
        //$ mSession{aMongoAccess.client()["acmacs_web"]},
{
    set_command_start();

} // Command::Command

// ----------------------------------------------------------------------

void Command::send(std::string aMessage, send_message_type aMessageType)
{
    auto message = to_json::object_prepend(aMessage, "C", command_name(), "CN", command_number(), "D", command_id(), "CT", command_duration());
    if (const auto to_add = add_to_response(); !to_add.empty())
        message = to_json::object_append(message, "add_to_response", to_json::raw(to_add));
    mClientConnection.send(message, aMessageType);
      // std::cerr << "Command::send: " << aMessage.substr(0, 100) << std::endl;

} // Command::send

// ----------------------------------------------------------------------

void Command::send_error(std::string aMessage)
{
    const auto message = to_json::object("C", command_name(), "CN", command_number(), "D", command_id(), "E", aMessage);
    print_cerr("send ERROR: ", message);
    mClientConnection.send(message, send_message_type::text);

} // Command::send_error

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
