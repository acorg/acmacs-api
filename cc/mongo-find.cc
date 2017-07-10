#include <iostream>
#include <string>

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/stream.hh"

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <object-id>" << std::endl;
        return 1;
    }

    mongocxx::instance inst{};
    mongocxx::pool pool{mongocxx::uri{}};
    auto conn = pool.acquire(); // shared_ptr<mongocxx::client>
    auto db = (*conn)["acmacs_web"]; // mongocxx::database

    auto collection_cursor = db.list_collections();
    std::vector<std::string> collections;
    std::transform(std::begin(collection_cursor), std::end(collection_cursor), std::back_inserter(collections), [](const auto& doc) { return doc["name"].get_utf8().value.to_string(); });
    // std::cout << collections << std::endl;

    for (const auto& collection_name: collections) {
        auto value_optional = db[collection_name].find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid{argv[1]} << bsoncxx::builder::stream::finalize);
        if (value_optional) {
            auto field_t = (value_optional->view())["_t"];
            auto field_table = (value_optional->view())["table"];
            if (field_t && field_t.get_utf8().value == bsoncxx::stdx::string_view{"acmacs.mongodb_collections.chart.Table"} && field_table) {
                auto field_binary = field_table.get_binary();
                std::cout << "Table " << field_binary.size << std::string{reinterpret_cast<const char*>(field_binary.bytes), 5} << /* bsoncxx::to_json(field_table.get_document().view()) <<  */std::endl;
            }
            else
                std::cout << collection_name << ": " << bsoncxx::to_json(*value_optional) << std::endl;
            break;
        }
    }
}


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
