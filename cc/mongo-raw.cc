#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <set>
#include <iterator> // make_ostream_joiner

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/stream.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/iterator.hh"
#include "acmacs-base/float.hh"

// ----------------------------------------------------------------------

namespace json_writer
{
    template <typename RW> class writer;
}

template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::map<std::string, bsoncxx::types::value>& map);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const bsoncxx::array::view& array);

#include "acmacs-base/json-writer.hh"

// ----------------------------------------------------------------------

class CommandBase
{
 public:
    virtual inline ~CommandBase() {}

    virtual void process(mongocxx::database& aDb) = 0;
    virtual void args(int /*argc*/, char* const /*argv*/[]) {}

 protected:
    using Filter = bsoncxx::builder::stream::document;
    static constexpr auto Fin = bsoncxx::builder::stream::finalize;

}; // class CommandBase

// ----------------------------------------------------------------------

template <typename Stream> inline void bson_symbol(Stream& out, const char* aSymbol)
{
    out << (std::string{"*"} + aSymbol + "*");
}

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

template <typename Stream = std::ostream> inline Stream& operator << (Stream& out, const bsoncxx::array::element& aE);

inline std::ostream& operator << (std::ostream& out, const bsoncxx::array::view& aArray)
{
    auto first = std::begin(aArray), last = std::end(aArray);
    out << '[';
    for (bool insert_separator = false; first != last; ++first) {
        if (insert_separator)
            out << ", ";
        else
            insert_separator = true;
        out << *first;
    }
    return out << ']';
}


namespace internal
{
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
              bson_symbol(out, "document");
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
    }
}

template <typename Stream> inline Stream& operator << (Stream& out, const bsoncxx::types::value& aV)
{
    internal::bson_value_to_stream(out, aV);
    return out;
}

template <typename Stream> inline Stream& operator << (Stream& out, const bsoncxx::document::element& aE)
{
    if (aE)
        out << aE.get_value();
    else
        bson_symbol(out, "*invalid*");
    return out;
}

template <typename Stream> inline Stream& operator << (Stream& out, const bsoncxx::array::element& aE)
{
    if (aE)
        out << aE.get_value();
    else
        bson_symbol(out, "*invalid*");
    return out;
}

// ----------------------------------------------------------------------

template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>& out, const bsoncxx::types::value& aV)
{
    internal::bson_value_to_stream(out, aV);
    return out;
}

// $$ template to json-writer.hh
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const bsoncxx::array::view& array)
{
    aWriter << json_writer::start_array;
    for (const auto& e: array)
        aWriter << e;
    return aWriter << json_writer::end_array;
}

// $$ template to json-writer.hh
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::map<std::string, bsoncxx::types::value>& map)
{
    aWriter << json_writer::start_object;
    for (const auto& e: map)
        aWriter << json_writer::key(e.first) << e.second;
    return aWriter << json_writer::end_object;
}

// ----------------------------------------------------------------------

// inline std::string bson_value_to_string(const bsoncxx::types::value& aV)
// {
//     std::ostringstream out;
//     switch (aV.type()) {
//       case bsoncxx::type::k_double:
//           out << aV.get_double().value;
//           break;
//       case bsoncxx::type::k_utf8:
//           out << "\"" << aV.get_utf8().value.to_string() << '"';
//           break;
//       case bsoncxx::type::k_document:
//           out << "document()";
//           break;
//       case bsoncxx::type::k_array:
//           out << '[';
//           {
//               bool first = true;
//               for (const auto& array_element: aV.get_array().value) {
//                   if (!first)
//                       out << ", ";
//                   else
//                       first = false;
//                   out << bson_value_to_string(array_element.get_value());
//               }
//           }
//           out << ']';
//           break;
//       case bsoncxx::type::k_binary:
//           out << "binary(size: " << aV.get_binary().size << ")";
//           break;
//       case bsoncxx::type::k_undefined:
//           out << "undefined";
//           break;
//       case bsoncxx::type::k_oid:
//           out << "ObjectId:\"" << aV.get_oid().value.to_string() << '"';
//           break;
//       case bsoncxx::type::k_bool:
//           out << aV.get_bool().value;
//           break;
//       case bsoncxx::type::k_date:
//           out << "date(?)";
//           break;
//       case bsoncxx::type::k_null:
//           out << "null";
//           break;
//       case bsoncxx::type::k_regex:
//           out << "regex(?)";
//           break;
//       case bsoncxx::type::k_dbpointer:
//           out << "dbpointer(?)";
//           break;
//       case bsoncxx::type::k_code:
//           out << "code(?)";
//           break;
//       case bsoncxx::type::k_symbol:
//           out << "symbol(?)";
//           break;
//       case bsoncxx::type::k_codewscope:
//           out << "codewscope(?)";
//           break;
//       case bsoncxx::type::k_int32:
//           out << aV.get_int32().value;
//           break;
//       case bsoncxx::type::k_timestamp:
//           out << "timestamp(?)";
//           break;
//       case bsoncxx::type::k_int64:
//           out << aV.get_int64().value;
//           break;
//       case bsoncxx::type::k_decimal128:
//           out << "decimal128(?)"; // aV.get_decimal128().value;
//           break;
//       case bsoncxx::type::k_maxkey:
//           out << "maxkey(?)";
//           break;
//       case bsoncxx::type::k_minkey:
//           out << "minkey(?)";
//           break;
//     }
//     return out.str();
// }

// inline std::string element_to_string(const bsoncxx::v_noabi::document::element& aE)
// {
//     return aE ? bson_value_to_string(aE.get_value()) : std::string{"*invalid*"};
// }

// inline std::string element_to_string(const bsoncxx::v_noabi::array::element& aE)
// {
//     return aE ? bson_value_to_string(aE.get_value()) : std::string{"*invalid*"};
// }

// ----------------------------------------------------------------------

class DocumentFindResults
{
 public:
    using value_type = bsoncxx::v_noabi::types::value;

    inline DocumentFindResults() {}
    inline DocumentFindResults(mongocxx::v_noabi::cursor&& aCursor, const std::vector<std::string>& aExcludeFields )
        : mExcludeFields(std::begin(aExcludeFields), std::end(aExcludeFields)) { build(std::move(aCursor)); }

    void build(mongocxx::v_noabi::cursor&& aCursor)
        {
            for (auto record: aCursor) {
                std::map<std::string, value_type> aRecord;
                for (auto field: record) {
                    const auto field_name = field.key().to_string();
                    if (mExcludeFields.find(field_name) == mExcludeFields.end()) {
                        mFields.insert(field_name);
                        aRecord.emplace(field.key().to_string(), field.get_value());
                    }
                }
                mRecords.push_back(aRecord);
            }
        }

    inline std::string csv() const
        {
            std::ostringstream result;
            std::transform(std::begin(mFields), std::end(mFields), polyfill::make_ostream_joiner(result, ","), &DocumentFindResults::string_to_csv);
            result << '\n';
            auto put_record = [this](const std::map<std::string, value_type>& aRec) {
                std::ostringstream out_record;;
                auto put_field = [&aRec](std::string aField) -> std::string {
                    auto f = aRec.find(aField);
                    return f == aRec.end() ? std::string{} : DocumentFindResults::value_type_to_csv(f->second);
                };
                std::transform(std::begin(this->mFields), std::end(this->mFields), polyfill::make_ostream_joiner(out_record, ","), put_field);
                return out_record.str();
            };
            std::transform(std::begin(mRecords), std::end(mRecords), polyfill::make_ostream_joiner(result, "\n"), put_record);
            return result.str();
        }

    inline std::string json() const
        {
            const size_t indent = 1;
            json_writer::writer<rapidjson::PrettyWriter<rapidjson::StringBuffer>> aWriter("DocumentFindResults");
            aWriter.SetIndent(' ', static_cast<unsigned int>(indent));
            aWriter << mRecords;
            std::string result = aWriter;
            const std::string ind(indent - 1, ' ');
            result.insert(1, ind + "\"_\": \"-*- js-indent-level: " + std::to_string(indent) + " -*-\",");
            return result;
        }

 private:
    std::set<std::string> mExcludeFields;
    std::set<std::string> mFields;
    std::vector<std::map<std::string, value_type>> mRecords;

    inline static std::string value_type_to_csv(value_type src)
        {
            std::ostringstream out;
            out << src;
            return "\"" + string::replace(out.str(), "\"", "\"\"") + "\"";
        }

    inline static std::string string_to_csv(std::string src)
        {
            return "\"" + string::replace(src, "\"", "\"\"") + "\"";
        }

}; // class DocumentFindResults

// ----------------------------------------------------------------------

class CommandCollections : public CommandBase
{
 public:
    virtual void process(mongocxx::database& aDb)
        {
            for (auto doc: aDb.list_collections())
                std::cout << doc["name"].get_utf8().value.to_string() << std::endl;
        }

}; // class CommandCollections

// ----------------------------------------------------------------------

class CommandUsers : public CommandBase
{
 public:
    virtual void process(mongocxx::database& aDb)
        {
            DocumentFindResults results{
                aDb["users_groups"].find(Filter{} << "_t" << "acmacs.mongodb_collections.users_groups.User" << Fin),
                {"_id", "_t", "password", "nonce"}
            };
            std::cout << results.csv() << std::endl;
            std::cout << results.json() << std::endl;
        }

}; // class CommandUsers

// ----------------------------------------------------------------------

static inline std::map<std::string, std::unique_ptr<CommandBase>> make_commands()
{
    std::map<std::string, std::unique_ptr<CommandBase>> commands;
    commands.emplace("collections", std::make_unique<CommandCollections>());
    commands.emplace("users", std::make_unique<CommandUsers>());
    return commands;
}

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <command> [<arg> ...]" << std::endl;
        return 1;
    }

    mongocxx::instance inst{};
    mongocxx::pool pool{mongocxx::uri{}};
    auto conn = pool.acquire(); // shared_ptr<mongocxx::client>
    auto db = (*conn)["acmacs_web"]; // mongocxx::database
    auto commands = make_commands();
    auto command = commands.find(argv[1]);
    if (command != commands.end()) {
        command->second->args(argc - 2, argv + 2);
        command->second->process(db);
    }
    else {
        std::cerr << "Unrecognized command: " << argv[1] << std::endl;
        std::cerr << " available commands:\n  ";
        std::transform(commands.begin(), commands.end(), polyfill::make_ostream_joiner(std::cerr, "\n  "), [](const auto& cmd) { return cmd.first; });
        return 2;
    }
    return 0;

    // auto collection_cursor = db.list_collections();
    // std::vector<std::string> collections;
    // std::transform(std::begin(collection_cursor), std::end(collection_cursor), std::back_inserter(collections), [](const auto& doc) { return doc["name"].get_utf8().value.to_string(); });
    // // std::cout << collections << std::endl;

    // for (const auto& collection_name: collections) {
    //     auto value_optional = db[collection_name].find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid{argv[1]} << bsoncxx::builder::stream::finalize);
    //     if (value_optional) {
    //         auto field_t = (value_optional->view())["_t"];
    //         auto field_table = (value_optional->view())["table"];
    //         if (field_t && field_t.get_utf8().value == bsoncxx::stdx::string_view{"acmacs.mongodb_collections.chart.Table"} && field_table) {
    //             auto field_binary = field_table.get_binary();
    //             std::cout << "Table " << field_binary.size << std::string{reinterpret_cast<const char*>(field_binary.bytes), 5} << /* bsoncxx::to_json(field_table.get_document().view()) <<  */std::endl;
    //         }
    //         else
    //             std::cout << collection_name << ": " << bsoncxx::to_json(*value_optional) << std::endl;
    //         break;
    //     }
    // }
}


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
