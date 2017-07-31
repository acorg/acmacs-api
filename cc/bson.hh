#pragma once

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <bsoncxx/builder/basic/document.hpp>
// #include <bsoncxx/json.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/from-json.hh"
#include "acmacs-base/to-json.hh"

// ----------------------------------------------------------------------

namespace to_bson
{
    inline void append(bsoncxx::builder::basic::document&) {}

    template <typename ... Args> inline void append(bsoncxx::builder::basic::document& target, const bsoncxx::document::value& to_merge, Args&& ... args)
    {
        target.append(bsoncxx::builder::concatenate(to_merge.view()));
        append(target, std::forward<Args>(args) ...);
    }

    template <typename Value, typename ... Args> inline void append(bsoncxx::builder::basic::document& target, std::string key, Value value, Args&& ... args)
    {
        target.append(bsoncxx::builder::basic::kvp(key, value));
        append(target, std::forward<Args>(args) ...);
    }

    inline void append(bsoncxx::builder::basic::array&) {}

    template <typename ... Args> inline void append(bsoncxx::builder::basic::array& target, const bsoncxx::document::value& to_append, Args&& ... args)
    {
        target.append(bsoncxx::builder::concatenate(to_append.view()));
        append(target, std::forward<Args>(args) ...);
    }

      // iterator SFINAE: https://stackoverflow.com/questions/12161109/stdenable-if-or-sfinae-for-iterator-or-pointer
    template <typename Iterator, typename = decltype(*std::declval<Iterator&>(), void(), ++std::declval<Iterator&>(), void())>
        inline bsoncxx::array::value array(Iterator first, Iterator last)
    {
        bsoncxx::builder::basic::array array;
        for (; first != last; ++first)
            array.append(*first);
        return array.extract();
    }

    template <typename Iterator, typename UnaryOperation, typename = decltype(*std::declval<Iterator&>(), void(), ++std::declval<Iterator&>(), void())>
        inline bsoncxx::array::value array(Iterator first, Iterator last, UnaryOperation unary_op)
    {
        bsoncxx::builder::basic::array array;
        for (; first != last; ++first)
            array.append(unary_op(*first));
        return array.extract();
    }

    template <typename ... Args> inline bsoncxx::array::value array(Args&& ... args)
    {
        bsoncxx::builder::basic::array array;
        append(array, std::forward<Args>(args) ...);
        return array.extract();
    }

    template <typename ... Args> inline bsoncxx::document::value object(Args&& ... args)
    {
        bsoncxx::builder::basic::document doc;
        append(doc, std::forward<Args>(args) ...);
        return doc.extract();
    }

// ----------------------------------------------------------------------

      // mongo_operator: $in, $all
    inline void in_for_optional_array_of_strings(bsoncxx::builder::basic::document& append_to, const char* key, const char* mongo_operator, std::function<from_json::ConstArray()> getter, std::function<std::string(const rapidjson::Value&)> transformer = &from_json::get_string)
    {
        try {
            const auto array = getter();
            if (!array.Empty())
                to_bson::append(append_to, key, to_bson::object(mongo_operator, to_bson::array(std::begin(array), std::end(array), transformer)));
        }
        catch (from_json::rapidjson_assert&) {
        }
    }

} // namespace to_bson

// ======================================================================

namespace from_bson
{
} // namespace from_bson

// ======================================================================

namespace to_json
{
    inline std::string symbol_(const char* tag) { return value(std::string{"*"} + tag + "*"); }

    // template <> inline std::string value(bsoncxx::document::view&& aView)
    // {
    //     return object(std::begin(aView), std::end(aView), [](const auto& element) { return std::make_tuple(element.key().to_string(), element.get_value()); });
    // }

    // template <> inline std::string value(bsoncxx::types::value&& aV)
    // {
    //     switch (aV.type()) {
    //       case bsoncxx::type::k_double:
    //           return value(aV.get_double().value);
    //       case bsoncxx::type::k_utf8:
    //           return value(aV.get_utf8().value.to_string());
    //       case bsoncxx::type::k_document:
    //           return value(aV.get_document().value);
    //       case bsoncxx::type::k_array:
    //           return value(aV.get_array().value);
    //       case bsoncxx::type::k_binary:
    //           return value(aV.get_binary());
    //       case bsoncxx::type::k_undefined:
    //           return value(to_json::undefined);
    //       case bsoncxx::type::k_oid:
    //           return value(aV.get_oid());
    //       case bsoncxx::type::k_bool:
    //           return value(aV.get_bool().value);
    //       case bsoncxx::type::k_date:
    //           return symbol_("date");
    //       case bsoncxx::type::k_null:
    //           return value(to_json::null);
    //       case bsoncxx::type::k_regex:
    //           return symbol_("regex");
    //       case bsoncxx::type::k_dbpointer:
    //           return symbol_("dbpointer");
    //       case bsoncxx::type::k_code:
    //           return symbol_("code");
    //       case bsoncxx::type::k_symbol:
    //           return symbol_("symbol");
    //       case bsoncxx::type::k_codewscope:
    //           return symbol_("codewscope");
    //       case bsoncxx::type::k_int32:
    //           return value(aV.get_int32().value);
    //       case bsoncxx::type::k_timestamp:
    //           return symbol_("timestamp");
    //       case bsoncxx::type::k_int64:
    //           return value(aV.get_int64().value);
    //       case bsoncxx::type::k_decimal128:
    //           return symbol_("decimal128"); // aV.get_decimal128().value;
    //       case bsoncxx::type::k_maxkey:
    //           return symbol_("maxkey");
    //       case bsoncxx::type::k_minkey:
    //           return symbol_("minkey");
    //     }
    // }

} // namespace to_json

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
