#pragma once

#include "application.hh"

// ----------------------------------------------------------------------

class ApplicationOne : public Application
{
 public:
    ApplicationOne();

    void run();
    void logged_in() override;

 protected:
    void reset() override;

 private:
    Handler* mHandler;
    client::HTMLElement* h_display_name;

    void ask_logout();

}; // class ApplicationOne

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
