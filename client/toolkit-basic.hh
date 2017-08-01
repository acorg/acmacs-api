#pragma once

#include <cheerp/clientlib.h>

#include "string.hh"

// ----------------------------------------------------------------------

namespace toolkit
{
    inline void add_class(client::HTMLElement* aElement, const char* aClass)
    {
        auto* present = aElement->get_className();
        if (present->indexOf(aClass) == -1)
            aElement->set_className(concat(present, " ", aClass));
    }

    inline void remove_class(client::HTMLElement* aElement, const char* aClass)
    {
        aElement->set_className(aElement->get_className()->replace(aClass, ""));
    }

} // namespace toolkit

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
