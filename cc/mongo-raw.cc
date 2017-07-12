#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <set>
#include <iterator> // make_ostream_joiner

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
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
    using document = bsoncxx::builder::stream::document;
    static constexpr auto finalize = bsoncxx::builder::stream::finalize;

    inline auto projection_to_exclude_fields(std::initializer_list<std::string>&& fields)
        {
            auto proj_doc = document{};
            for (auto field: fields)
                proj_doc << field << false;
            return proj_doc << finalize;
        }

}; // class CommandBase

// ----------------------------------------------------------------------

class DocumentFindResults
{
 public:
    using document_view = bsoncxx::document::view;

    inline DocumentFindResults() {}
    inline DocumentFindResults(mongocxx::v_noabi::cursor&& aCursor) { build(std::move(aCursor)); }

    void build(mongocxx::v_noabi::cursor&& aCursor)
        {
            std::copy(std::begin(aCursor), std::end(aCursor), std::back_inserter(mRecords));
        }

    inline std::string json() const
        {
            const size_t indent = 1;
            json_writer::writer<rapidjson::PrettyWriter<rapidjson::StringBuffer>> aWriter("DocumentFindResults");
            aWriter.SetIndent(' ', static_cast<unsigned int>(indent));
            aWriter << json_writer::start_object
                    << json_writer::key("results") << mRecords
                    << json_writer::end_object;
            std::string result = aWriter;
            json_writer::insert_emacs_indent_hint(result, indent);
            return result;
        }

 private:
    std::vector<document_view> mRecords;

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
            auto filter = document{} << "_t" << "acmacs.mongodb_collections.users_groups.User" << finalize;
            auto options = mongocxx::options::find{};
            options.projection(projection_to_exclude_fields({"_id", "_t", "password", "nonce"}));
            DocumentFindResults results{aDb["users_groups"].find(std::move(filter), options)};
            std::cout << results.json() << std::endl;
        }

}; // class CommandUsers

// ----------------------------------------------------------------------

class CommandSessions : public CommandBase
{
 public:
    virtual void process(mongocxx::database& aDb)
        {
            auto filter = document{} << /* "_t" << "acmacs.mongodb_collections.users_groups.User" << */ finalize;
            auto options = mongocxx::options::find{};
              //options.projection(projection_to_exclude_fields({"_id", "_t", "password", "nonce"}));
            DocumentFindResults results{aDb["sessions"].find(std::move(filter), options)};
            std::cout << results.json() << std::endl;
        }

}; // class CommandSessions

// ----------------------------------------------------------------------

static inline std::map<std::string, std::unique_ptr<CommandBase>> make_commands()
{
    std::map<std::string, std::unique_ptr<CommandBase>> commands;
    commands.emplace("collections", std::make_unique<CommandCollections>());
    commands.emplace("users", std::make_unique<CommandUsers>());
    commands.emplace("sessions", std::make_unique<CommandSessions>());
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
