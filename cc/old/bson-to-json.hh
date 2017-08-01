#pragma once

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types/value.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/to-json.hh"

// ----------------------------------------------------------------------

namespace json_writer
{
    template <typename RW> class writer;
}

template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const bsoncxx::array::view& array);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const bsoncxx::document::view& document);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const bsoncxx::document::value& document);

template <typename Stream> Stream& operator << (Stream& out, const bsoncxx::types::value& aV);
template <typename Stream> Stream& operator << (Stream& out, const bsoncxx::array::element& aE);
template <typename Stream> Stream& operator << (Stream& out, const bsoncxx::document::element& aE);

#include "acmacs-base/json-writer.hh"

// ----------------------------------------------------------------------

template <typename Stream> inline Stream& operator << (Stream& out, const bsoncxx::types::b_binary& aV)
{
    out << (std::string{"*binary(size: "} + std::to_string(aV.size) + ")*");
    return out;
}

template <typename Stream> inline Stream& operator << (Stream& out, const bsoncxx::types::b_oid& aV)
{
    out << aV.value.to_string();
    return out;
}

// ----------------------------------------------------------------------

namespace bson_to_internal
{
    template <typename Stream> inline void bson_symbol(Stream& out, const char* aSymbol)
    {
        out << (std::string{"*"} + aSymbol + "*");
    }

    template <typename Stream> inline void bson_value_to_stream(Stream& out, const bsoncxx::types::value& aV)
    {
        switch (aV.type()) {
          case bsoncxx::type::k_double:
              out << aV.get_double().value;
              break;
          case bsoncxx::type::k_utf8:
              out << aV.get_utf8().value.to_string();
              break;
          case bsoncxx::type::k_document:
              out << aV.get_document().value;
              break;
          case bsoncxx::type::k_array:
              out << aV.get_array().value;
              break;
          case bsoncxx::type::k_binary:
              out << aV.get_binary();
              break;
          case bsoncxx::type::k_undefined:
              bson_symbol(out, "undefined");
              break;
          case bsoncxx::type::k_oid:
              out << aV.get_oid();
              break;
          case bsoncxx::type::k_bool:
              out << aV.get_bool().value;
              break;
          case bsoncxx::type::k_date:
              bson_symbol(out, "date");
              break;
          case bsoncxx::type::k_null:
              bson_symbol(out, "null");
              break;
          case bsoncxx::type::k_regex:
              bson_symbol(out, "regex");
              break;
          case bsoncxx::type::k_dbpointer:
              bson_symbol(out, "dbpointer");
              break;
          case bsoncxx::type::k_code:
              bson_symbol(out, "code");
              break;
          case bsoncxx::type::k_symbol:
              bson_symbol(out, "symbol");
              break;
          case bsoncxx::type::k_codewscope:
              bson_symbol(out, "codewscope");
              break;
          case bsoncxx::type::k_int32:
              out << aV.get_int32().value;
              break;
          case bsoncxx::type::k_timestamp:
              bson_symbol(out, "timestamp");
              break;
          case bsoncxx::type::k_int64:
              out << aV.get_int64().value;
              break;
          case bsoncxx::type::k_decimal128:
              bson_symbol(out, "decimal128"); // aV.get_decimal128().value;
              break;
          case bsoncxx::type::k_maxkey:
              bson_symbol(out, "maxkey");
              break;
          case bsoncxx::type::k_minkey:
              bson_symbol(out, "minkey");
              break;
        }
    } // bson_value_to_stream

} // namespace bson_to_internal

// ----------------------------------------------------------------------

template <typename Stream> inline Stream& operator << (Stream& out, const bsoncxx::types::value& aV)
{
    bson_to_internal::bson_value_to_stream(out, aV);
    return out;
}

template <typename Stream> inline Stream& operator << (Stream& out, const bsoncxx::document::element& aE)
{
    if (aE)
        out << aE.get_value();
    else
        bson_to_internal::bson_symbol(out, "*invalid*");
    return out;
}

template <typename Stream> inline Stream& operator << (Stream& out, const bsoncxx::array::element& aE)
{
    if (aE)
        out << aE.get_value();
    else
        bson_to_internal::bson_symbol(out, "*invalid*");
    return out;
}

// ----------------------------------------------------------------------

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const bsoncxx::array::view& array)
{
    return json_writer::write_list(aWriter, array);
}

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const bsoncxx::document::view& document)
{
    aWriter << json_writer::start_object;
    for (const auto& e: document)
        aWriter << json_writer::key(e.key().to_string()) << e.get_value();
    return aWriter << json_writer::end_object;
}

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const bsoncxx::document::value& document)
{
    return aWriter << document.view();
}

// ----------------------------------------------------------------------

namespace to_json
{
    template <> inline std::string value(bsoncxx::document::value&& value)
    {
        json_writer::compact writer;
        return writer << value.view() << json_writer::finalize;
    }

} // namespace to_json

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
