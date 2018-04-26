#include "handler.hh"

// ----------------------------------------------------------------------

Handler::~Handler()
{
    // log("~Handler");

} // Handler::~Handler

// ----------------------------------------------------------------------

void Handler::on_error(String* aMessage)
{
    log_error("Handler::on_error: ", aMessage);

} // Handler::on_error

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
