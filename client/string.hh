#pragma once

#include <cheerp/client.h>
#include <cheerp/clientlib.h>

#include "asm.hh"

// ----------------------------------------------------------------------

using client::String;

inline String* operator ""_S(const char* src, size_t) { return new String{src}; }

inline bool eq(String* s1, const char* s2) { return s1 == new String{s2}; }
inline bool eq(const char* s1, String* s2) { return s2 == new String{s1}; }

// ----------------------------------------------------------------------

inline client::Object* stringify_replacer(String* key, client::Object* value)
{
    if (key == "i0"_S)
        value = client::make_undefined();
    return value;
}

inline String* stringify(client::Object* value, size_t indent = 0)
{
    return client::JSON.stringify(value, cheerp::Callback(&stringify_replacer), client::make_number(indent));
}

// ----------------------------------------------------------------------

inline String* to_String(client::Object* value)
{
    return is_string(value) ? static_cast<String*>(value) : stringify(value);
}

inline String* to_String(String* value)
{
    return value;
}

inline String* to_String(const char* value)
{
    return new String{value};
}

template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>> inline String* to_String(T value)
{
    return new String{value};
}

// ----------------------------------------------------------------------

      // (... && ) - c++17
// template <typename ... Args> inline std::enable_if_t<(... && std::is_same<Args, String*>::value), client::Array*> to_Array_String(Args ... args)
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

template <typename ... Args> inline String* concat(String* s1, Args&& ... args)
{
    return s1->concat(std::forward<Args>(args) ...);
}

template <typename ... Args> inline String* concat(const char* s1, Args&& ... args)
{
    return concat(new String{s1}, std::forward<Args>(args) ...);
}

inline String* concat_space(String* s1) { return s1; }
template <typename S1, typename S2, typename ... Args> inline String* concat_space(S1 s1, S2 s2, Args&& ... args)
{
    return concat_space(concat(s1, " ", s2), std::forward<Args>(args) ...);
}

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

inline String* make_json(client::Object* src)
{
    return stringify(src);
}

template <typename ... Args> inline String* make_json(Args ... args)
{
    return stringify(make_object(args ...));
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
