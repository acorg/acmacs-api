#pragma once

#include "application.hh"

// ----------------------------------------------------------------------

class ApplicationOne : public Application
{
 public:
    inline ApplicationOne() : Application{} {}

    void run();

 private:

    // void make_session();

    // void on_message(client::RawMessage* aMessage); // parsed json from the server
    // void on_error(String* aData); // cannot parse response from the server or json contains "E"
    // void on_close(client::CloseEvent* aEvent);

    // void on_hello(client::RawMessage* aMessage); // ws connection established

    // void make_connection();
    // void on_raw_message_event(client::MessageEvent* aEvent);

}; // class ApplicationOne

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
