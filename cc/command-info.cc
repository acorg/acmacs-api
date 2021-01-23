#include "acmacs-base/json-writer.hh"
#include "acmacs-api/command-info.hh"
#include "acmacs-api/command-factory.hh"

// ----------------------------------------------------------------------

void Command_version::run()
{
    send(to_json::v1::object("version", "acmacs-d-20180426"));

} // Command_version::run

// ----------------------------------------------------------------------

const char* Command_version::description()
{
    return "sends server version";

} // Command_version::description

// ----------------------------------------------------------------------

void Command_list_commands::run()
{
    json_writer::compact writer;
    writer << json_writer::start_array;
    for (const auto& command: CommandFactory::sFactory->commands()) {
        writer << json_writer::start_object
               << "name" << command.first
               << "description" << command.second.description()
               << json_writer::end_object;
    }
    writer << json_writer::end_array;
    send(to_json::v1::object("commands", to_json::v1::raw{writer << json_writer::finalize}));

} // Command_list_commands::run

// ----------------------------------------------------------------------

const char* Command_list_commands::description()
{
    return "lists available commands and their descriptions";

} // Command_list_commands::description

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
