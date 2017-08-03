#pragma once

#include "application.hh"

// ----------------------------------------------------------------------

class ApplicationOne : public Application
{
 public:
    inline ApplicationOne() : Application{}, mHandler{nullptr} {}

    void run();
    void logged_in() override;

 private:
    Handler* mHandler;

}; // class ApplicationOne

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
