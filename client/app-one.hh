#pragma once

#include "application.hh"

// ----------------------------------------------------------------------

class ApplicationOne : public Application
{
 public:
    inline ApplicationOne() : Application{} {}

    void run();

 private:


}; // class ApplicationOne

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
