#pragma once

#include <cheerp/clientlib.h>

// ----------------------------------------------------------------------

namespace client
{
    bool is_string(client::Object*);

    client::Object* make_undefined(); // don't know how to make undefined in cheerp 1.3 otherwise
    bool is_undefined(client::Object*);
    bool is_undefined(client::Object&);
    client::String* make_cnonce();

    template <typename ... Args> void console_log(Args ...);
    template <typename ... Args> inline void console_log(const char* first, Args ... rest) { console_log(new client::String{first}, rest ...); }

    Array* object_keys(Object*);
    Array* object_keys(Object&);

} // namespace client

// ----------------------------------------------------------------------

void make_asm_definitions();

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
