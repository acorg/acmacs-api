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

inline client::String* stringify(client::Object* value, size_t indent = 0)
{
    return client::JSON.stringify(value, cheerp::Callback(&stringify_replacer), client::make_number(indent));
}

// ----------------------------------------------------------------------

inline client::String* to_String(client::Object* value)
{
    return is_string(value) ? static_cast<client::String*>(value) : stringify(value);
}

inline client::String* to_String(client::String* value)
{
    return value;
}

inline client::String* to_String(const char* value)
{
    return new client::String{value};
}

template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>> inline client::String* to_String(T value)
{
    return new client::String{value};
}

// ----------------------------------------------------------------------

      // (... && ) - c++17
// template <typename ... Args> inline std::enable_if_t<(... && std::is_same<Args, client::String*>::value), client::Array*> to_Array_String(Args ... args)
// {
//     return new client::Array(args ...);
// }

// template <typename ... Args> inline std::enable_if_t<(... && std::is_same<Args, const char*>::value), client::Array*> to_Array_String(Args ... args)
// {
//     return new client::Array(to_String(std::forward<Args>(args)) ...);
// }

template <typename ... Args> inline client::Array* to_Array_String(Args ... args)
{
    return new client::Array(to_String(std::forward<Args>(args)) ...);
}

// ----------------------------------------------------------------------

inline client::String* concat(client::String* first, client::Object* second)
{
    // if (!is_string(second))
    //     second = client::JSON.stringify(second, cheerp::Callback(&stringify_replacer));
    // return first->concat(static_cast<client::String*>(second));
    return first->concat(to_String(second));
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

namespace string_internal
{
    inline client::Object* to_object(client::Object* src) { return src; }
    inline client::Object* to_object(const char* src) { return to_String(src); }

    template <typename Key, typename Value> inline void add_to_object(client::Object* target, Key key, Value value)
    {
        target->set_(*to_String(key), to_object(value));
    }

    template <typename Key, typename Value, typename ... Args> inline void add_to_object(client::Object* target, Key key, Value value, Args ... args)
    {
        add_to_object(target, key, value);
        add_to_object(target, args ...);
    }
}

template <typename ... Args> inline client::Object* make_object(Args ... args)
{
    auto* result = new client::Object{};
    string_internal::add_to_object(result, args...);
    return result;
}

inline client::String* make_json(client::Object* src)
{
    return stringify(src);
}

template <typename ... Args> inline client::String* make_json(Args ... args)
{
    return stringify(make_object(args ...));
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
