#pragma once

#include <cheerp/clientlib.h>

#include "asm.hh"

// ----------------------------------------------------------------------

namespace client
{
    struct Argv : public Object
    {
        Array* get_S();
        Array* get_U();        // user
        Array* get_P();        // password

        inline String* to_String(Array& value, String* defaul)
            {
                if (is_undefined(value) || value.get_length() == 0)
                    return defaul; // static_cast<String*>(make_undefined());
                return static_cast<String*>(value[0]);
            }

        inline String* session() { return to_String(*get_S(), static_cast<String*>(make_undefined())); }
        inline String* user() { return to_String(*get_U(), ""_S); }
        inline String* password() { return to_String(*get_P(), ""_S); }
    };

    extern Argv* ARGV;

} // namespace client

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
