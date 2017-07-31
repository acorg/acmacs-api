#pragma once

#include <cheerp/clientlib.h>

#include "command.hh"

// ----------------------------------------------------------------------

namespace client
{
    struct Session : public Object
    {
        String* get_id();
        void set_id(String*);
        String* get_user();
        void set_user(String*);
        String* get_display_name();
        void set_display_name(String*);
    };

    extern Session* session;

} // namespace client

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
