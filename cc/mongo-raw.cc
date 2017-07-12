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

#include "bson-to-json.hh"

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

    inline std::string json() const
        {
            return json_writer::json(mRecords, "DocumentFindResults", 1);
        }

 private:
    std::set<std::string> mExcludeFields;
    std::set<std::string> mFields;
    std::vector<std::map<std::string, value_type>> mRecords;

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
            // std::cout << results.csv() << std::endl;
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
