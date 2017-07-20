#pragma once

#include <cheerp/client.h>
#include <cheerp/clientlib.h>

#include "asm.hh"

// ----------------------------------------------------------------------

inline client::String* operator ""_S(const char* src, size_t) { return new client::String{src}; }

// ----------------------------------------------------------------------

inline client::Object* stringify_replacer(client::String* key, client::Object* value)
{
    if (key == "i0"_S)
        value = client::make_undefined();
    return value;
}

inline client::String* concat(client::String* first, client::Object* second)
{
    if (!is_string(second))
        second = client::JSON.stringify(second, cheerp::Callback(&stringify_replacer));
    return first->concat(static_cast<client::String*>(second));
}

inline client::String* concat(client::String* first, client::String* second)
{
    return first->concat(second);
}

inline client::String* concat(client::String* first, const char* second)
{
    return first->concat(second);
}

inline client::String* concat(const char* first, client::Object* second)
{
    return concat(new client::String{first}, second);
}

inline client::String* concat(const char* first, client::String* second)
{
    return concat(new client::String{first}, second);
}

template <typename First, typename Second, typename ... Args> inline client::String* concat(First first, Second second, Args ... rest)
{
    return concat(concat(first, second), rest ...);
}

//inline client::String operator "" _s(const char* src, size_t) { return {src}; }
// inline client::String* operator + (client::String&& s1, client::String* s2) { return s1.concat(s2); }
// inline client::String* operator + (client::String& s1, client::String* s2) { return s1.concat(s2); }
// inline client::String* operator + (const char* s1, client::String& s2) { return client::String{s1}.concat(s2); }
// inline bool operator == (client::String&& s1, client::String* s2) { return &s1 == s2; }
// inline bool operator == (client::String* s1, client::String&& s2) { return s1 == &s2; }

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
