#include "command-info.hh"
#include "command-factory.hh"

#include "acmacs-base/json-writer.hh"

// ----------------------------------------------------------------------

void Command_version::run()
{
    send(json_object("version", "acmacs-d-20170724"));

} // Command_version::run

// ----------------------------------------------------------------------

const char* Command_version::description()
{
    return "sends server version";

} // Command_version::description

// ----------------------------------------------------------------------

void Command_list_commands::run()
{
    json_writer::compact writer{"list_commands"};
    writer << json_writer::start_array;
    for (const auto& command: CommandFactory::sFactory->commands()) {
        writer << json_writer::start_object
               << "name" << command.first
               << "description" << command.second.description()
               << json_writer::end_object;
    }
    writer << json_writer::end_array;
    send(json_object("commands", json_raw{writer << json_writer::finalize}));

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
