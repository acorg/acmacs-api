#pragma once

#include <cheerp/clientlib.h>

// ----------------------------------------------------------------------

namespace client
{
    bool is_string(client::Object*);
    bool is_array(client::Object*);
    bool is_array_of_strings(client::Object*);
    String* typeof(client::Object*);
    void debug();

    client::Object* make_undefined(); // don't know how to make undefined in cheerp 1.3 otherwise
    bool is_undefined(client::Object*);
    bool is_undefined(client::Object&);
    bool is_undefined_or_null(const client::Object*);
    bool is_undefined_or_null(const client::Object&);
    bool is_not_null(client::Object*); // and not undefined
    bool is_not_null(client::Object&); // and not undefined
    bool is_defined(client::Object*);  // not null, not undefined
    bool is_not_empty(client::Object*);  // not null, not undefined, length > 0

    client::Object* make_number(int);
    client::Object* make_number(unsigned);
    client::Object* make_number(long);
    client::Object* make_number(unsigned long);

    client::String* make_cnonce();

    client::String* ws_host_port();

    client::String* json_syntax_highlight(client::String*);

    template <typename ... Args> void console_log(Args ...);
      //template <typename ... Args> inline void console_log(const char* first, Args ... rest) { console_log(new client::String{first}, rest ...); }
    template <typename ... Args> void console_error(Args ...);
      //template <typename ... Args> inline void console_error(const char* first, Args ... rest) { console_error(new client::String{first}, rest ...); }
    template <typename ... Args> void console_warning(Args ...);
      //template <typename ... Args> inline void console_warning(const char* first, Args ... rest) { console_warning(new client::String{first}, rest ...); }

    Array* object_keys(Object*);
    Array* object_keys(Object&);

      // localStorage if available
    Storage* app_local_storage();

} // namespace client

// ----------------------------------------------------------------------

template <typename ... Args> inline void log(const char* first, Args ... rest) { client::console_log(new client::String{first}, rest ...); }
template <typename ... Args> inline void log_error(const char* first, Args ... rest) { client::console_error(new client::String{first}, rest ...); }
template <typename ... Args> inline void log_warning(const char* first, Args ... rest) { client::console_warning(new client::String{first}, rest ...); }

// ----------------------------------------------------------------------

void make_asm_definitions();

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
