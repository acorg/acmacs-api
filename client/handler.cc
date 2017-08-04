#include "handler.hh"

// ----------------------------------------------------------------------

Handler::~Handler()
{
    // client::console_log("~Handler");

} // Handler::~Handler

// ----------------------------------------------------------------------

void Handler::on_error(String* aMessage)
{
    client::console_error("Handler::on_error: ", aMessage);

} // Handler::on_error

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
