#pragma once

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <bsoncxx/builder/basic/document.hpp>
// #include <bsoncxx/json.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/rapidjson.hh"

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

      // mongo_operator: $in, $all
inline void bson_in_for_optional_array_of_strings(bsoncxx::builder::basic::document& append_to, const char* key, const char* mongo_operator, std::function<json_importer::ConstArray()> getter, std::function<std::string(const rapidjson::Value&)> transformer = &json_importer::get_string)
{
    try {
        const auto array = getter();
        if (!array.Empty())
            bson_append(append_to, key, bson_object(mongo_operator, bson_array(std::begin(array), std::end(array), transformer)));
    }
    catch (RapidjsonAssert&) {
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
