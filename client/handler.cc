#include "handler.hh"

// ----------------------------------------------------------------------

Handler::~Handler()
{

} // Handler::~Handler

// ----------------------------------------------------------------------

void Handler::on_error(String* aMessage)
{
    console_error("Handler::on_error: ", aMessage);

} // Handler::on_error

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
