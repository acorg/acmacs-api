#pragma once

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <bsoncxx/builder/basic/document.hpp>
// #include <bsoncxx/json.hpp>
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

inline void bson_append(bsoncxx::builder::basic::document&) {}

template <typename ... Args> inline void bson_append(bsoncxx::builder::basic::document& target, const bsoncxx::document::value& to_merge, Args ... args)
{
    target.append(bsoncxx::builder::concatenate(to_merge.view()));
    bson_append(target, args ...);
}

template <typename Value, typename ... Args> inline void bson_append(bsoncxx::builder::basic::document& target, std::string key, Value value, Args ... args)
{
    target.append(bsoncxx::builder::basic::kvp(key, value));
    bson_append(target, args ...);
}

inline void bson_append(bsoncxx::builder::basic::array&) {}

template <typename ... Args> inline void bson_append(bsoncxx::builder::basic::array& target, const bsoncxx::document::value& to_append, Args ... args)
{
    target.append(bsoncxx::builder::concatenate(to_append.view()));
    bson_append(target, args ...);
}

// iterator SFINAE: https://stackoverflow.com/questions/12161109/stdenable-if-or-sfinae-for-iterator-or-pointer
template <typename Iterator, typename = decltype(*std::declval<Iterator&>(), void(), ++std::declval<Iterator&>(), void())>
    inline bsoncxx::array::value bson_array(Iterator first, Iterator last)
{
    bsoncxx::builder::basic::array array;
    for (; first != last; ++first)
        array.append(*first);
    return array.extract();
}

template <typename Iterator, typename UnaryOperation, typename = decltype(*std::declval<Iterator&>(), void(), ++std::declval<Iterator&>(), void())>
    inline bsoncxx::array::value bson_array(Iterator first, Iterator last, UnaryOperation unary_op)
{
    bsoncxx::builder::basic::array array;
    for (; first != last; ++first)
        array.append(unary_op(*first));
    return array.extract();
}

template <typename ... Args> inline bsoncxx::array::value bson_array(Args ... args)
{
    bsoncxx::builder::basic::array array;
    bson_append(array, args ...);
    return array.extract();
}

template <typename ... Args> inline bsoncxx::document::value bson_object(Args ... args)
{
    bsoncxx::builder::basic::document doc;
    bson_append(doc, args ...);
    return doc.extract();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
