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

template <typename Element, typename std::enable_if<std::is_base_of<bsoncxx::v_noabi::document::element, Element>::value>::type* = nullptr>
    std::ostream& operator << (std::ostream& out, const Element& aE);

template <typename Element, typename std::enable_if<std::is_base_of<bsoncxx::v_noabi::document::element, Element>::value>::type* = nullptr>
    inline std::string element_to_string(const Element& aE)
{
    std::ostringstream out;
    if (aE) {
        switch (aE.type()) {
          case bsoncxx::type::k_double:
              out << aE.get_double().value;
              break;
          case bsoncxx::type::k_utf8:
              out << "\"" << aE.get_utf8().value.to_string() << '"';
              break;
          case bsoncxx::type::k_document:
              out << "document()";
              break;
          case bsoncxx::type::k_array:
              out << '[';
              {
                  bool first = true;
                  for (const auto& array_element: aE.get_array().value) {
                      if (!first)
                          out << ", ";
                      else
                          first = false;
                      out << array_element;
                  }
              }
              out << ']';
              break;
          case bsoncxx::type::k_binary:
              out << "binary(size: " << aE.get_binary().size << ")";
              break;
          case bsoncxx::type::k_undefined:
              out << "undefined";
              break;
          case bsoncxx::type::k_oid:
              out << "ObjectId:\"" << aE.get_oid().value.to_string() << '"';
              break;
          case bsoncxx::type::k_bool:
              out << aE.get_bool().value;
              break;
          case bsoncxx::type::k_date:
              out << "date(?)";
              break;
          case bsoncxx::type::k_null:
              out << "null";
              break;
          case bsoncxx::type::k_regex:
              out << "regex(?)";
              break;
          case bsoncxx::type::k_dbpointer:
              out << "dbpointer(?)";
              break;
          case bsoncxx::type::k_code:
              out << "code(?)";
              break;
          case bsoncxx::type::k_symbol:
              out << "symbol(?)";
              break;
          case bsoncxx::type::k_codewscope:
              out << "codewscope(?)";
              break;
          case bsoncxx::type::k_int32:
              out << aE.get_int32().value;
              break;
          case bsoncxx::type::k_timestamp:
              out << "timestamp(?)";
              break;
          case bsoncxx::type::k_int64:
              out << aE.get_int64().value;
              break;
          case bsoncxx::type::k_decimal128:
              out << "decimal128(?)"; // aE.get_decimal128().value;
              break;
          case bsoncxx::type::k_maxkey:
              out << "maxkey(?)";
              break;
          case bsoncxx::type::k_minkey:
              out << "minkey(?)";
              break;
        }
    }
    else {
        out << "*invalid*";
    }
    return out.str();
}

template <typename Element, typename std::enable_if<std::is_base_of<bsoncxx::v_noabi::document::element, Element>::value>::type*>
    inline std::ostream& operator << (std::ostream& out, const Element& aE)
{
    return out << element_to_string(aE);
}

class DocumentFindResults
{
 public:
    inline DocumentFindResults() {}
    inline DocumentFindResults(mongocxx::v_noabi::cursor&& aCursor) { build(std::move(aCursor)); }

    void build(mongocxx::v_noabi::cursor&& aCursor)
        {
            for (auto record: aCursor) {
                std::map<std::string, std::string> aRecord;
                for (auto field: record) {
                    mFields.insert(field.key().to_string());
                    aRecord.emplace(field.key().to_string(), element_to_string(field));
                }
                mRecords.push_back(aRecord);
            }
        }

    inline std::string csv() const
        {
            std::ostringstream result;
            std::transform(std::begin(mFields), std::end(mFields), std::make_ostream_joiner(result, ","), &DocumentFindResults::to_csv);
            result << '\n';
            auto put_record = [this](const std::map<std::string, std::string>& aRec) {
                std::ostringstream out_record;;
                auto put_field = [&aRec](std::string aField) -> std::string {
                    auto f = aRec.find(aField);
                    return f == aRec.end() ? std::string{} : DocumentFindResults::to_csv(f->second);
                };
                std::transform(std::begin(this->mFields), std::end(this->mFields), std::make_ostream_joiner(out_record, ","), put_field);
                return out_record.str();
            };
            std::transform(std::begin(mRecords), std::end(mRecords), std::make_ostream_joiner(result, "\n"), put_record);
            return result.str();
        }

 private:
    std::set<std::string> mFields;
    std::vector<std::map<std::string, std::string>> mRecords;

    inline static std::string to_csv(std::string src)
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
            DocumentFindResults results{aDb["users_groups"].find(Filter{} << "_t" << "acmacs.mongodb_collections.users_groups.User" << Fin)};
            std::cout << results.csv() << std::endl;
            // for (auto doc: aDb["users_groups"].find(Filter{} << "_t" << "acmacs.mongodb_collections.users_groups.User" << Fin)) {
            //     // std::cout << bsoncxx::to_json(doc) << std::endl;
            //     std::cout << doc["name"] << ' ' << doc["recent_logins"] << std::endl;
            // }
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
        std::transform(commands.begin(), commands.end(), std::make_ostream_joiner(std::cerr, "\n  "), [](const auto& cmd) { return cmd.first; });
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
