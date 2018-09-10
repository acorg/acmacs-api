#include <iostream>

#include "acmacs-webserver/print.hh"

#include "command.hh"
#include "mongo-acmacs-c2-access.hh"

// ----------------------------------------------------------------------

Command::Command(rjson::v1::object&& aSrc, MongoAcmacsC2Access& aMongoAccess, ClientConnection& aClientConnection, size_t aCommandNumber)
    : data_{std::move(aSrc)}, mDb{aMongoAccess.client()["acmacs_web"]}, mClientConnection{aClientConnection}, mCommandNumber{aCommandNumber}
        //$ mSession{aMongoAccess.client()["acmacs_web"]},
{
    set_command_start();

} // Command::Command

// ----------------------------------------------------------------------

void Command::send(std::string aMessage, send_message_type aMessageType)
{
    if (aMessageType == send_message_type::text) {
        auto message = to_json::object_prepend(aMessage, "C", command_name(), "CN", command_number(), "D", command_id(), "CT", static_cast<float>(command_duration()));
        try {
            message = to_json::object_append(message, "add_to_response", to_json::raw(add_to_response().to_json()));
        }
        catch (rjson::v1::field_not_found&) {
        }
        mClientConnection.send(message, aMessageType);
        // std::cerr << "Command::send: " << aMessage.substr(0, 100) << std::endl;
    }
    else
        mClientConnection.send(aMessage, aMessageType);

} // Command::send

// ----------------------------------------------------------------------

void Command::send_binary(std::string aName, std::string aData)
{
    const std::string header = to_json::object("name", aName, "C", command_name(), "CN", command_number(), "D", command_id(), "CT", static_cast<float>(command_duration()));
    std::string header_size = std::to_string(header.size());
    header_size.append(4 - header_size.size(), ' ');
    send(header_size + header + aData, send_message_type::binary);

} // Command::send_binary

// ----------------------------------------------------------------------

void Command::send_error(std::string aMessage)
{
    const auto message = to_json::object("C", command_name(), "CN", command_number(), "D", command_id(), "E", aMessage);
    print_send_error(log_send_receive(), command_id(), aMessage);
    mClientConnection.send(message, send_message_type::text);

} // Command::send_error

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
