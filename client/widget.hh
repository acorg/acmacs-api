#pragma once

#include "toolkit-basic.hh"

// ----------------------------------------------------------------------

class Widget
{
 public:
    virtual ~Widget();

    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void reset() = 0;

}; // class Widget

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
