#pragma once

#include <cheerp/clientlib.h>

// ----------------------------------------------------------------------

namespace client
{
    bool is_string(client::Object*);

    client::Object* make_undefined(); // don't know how to make undefined in cheerp 1.3 otherwise
    bool is_undefined(client::Object*);
    bool is_undefined(client::Object&);
    bool is_undefined_or_null(client::Object*);
    bool is_undefined_or_null(client::Object&);
    bool is_not_null(client::Object*); // and not undefined
    bool is_not_null(client::Object&); // and not undefined

    client::Object* make_number(int);
    client::Object* make_number(unsigned);
    client::Object* make_number(long);
    client::Object* make_number(unsigned long);

    client::String* make_cnonce();
    client::String* json_syntax_highlight(client::String*);

    template <typename ... Args> void console_log(Args ...);
    template <typename ... Args> inline void console_log(const char* first, Args ... rest) { console_log(new client::String{first}, rest ...); }
    template <typename ... Args> void console_error(Args ...);
    template <typename ... Args> inline void console_error(const char* first, Args ... rest) { console_error(new client::String{first}, rest ...); }

    Array* object_keys(Object*);
    Array* object_keys(Object&);

} // namespace client

// ----------------------------------------------------------------------

void make_asm_definitions();

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
