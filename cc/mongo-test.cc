#include <iostream>

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdocumentation"
#pragma GCC diagnostic ignored "-Wdocumentation-deprecated-sync"
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wshadow-field-in-constructor"
#endif
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

static void test_db_from_cxx_driver_doc(mongocxx::client& conn);

int main(int, char**)
{
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{}};

    auto collection = conn["acmacs_web"]["acmacs_version"];
    auto cursor = collection.find({});
    for (auto& doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << std::endl;
    }
}

// ----------------------------------------------------------------------

void test_db_from_cxx_driver_doc(mongocxx::client& conn)
{
    bsoncxx::builder::stream::document document{};

    auto collection = conn["testdb"]["testcollection"];
    document << "hello" << "world";

    collection.insert_one(document.view());
    auto cursor = collection.find({});

    for (auto&& doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << std::endl;
    }

} // test_db_from_cxx_driver_doc

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
