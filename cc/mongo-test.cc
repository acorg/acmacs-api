#include <iostream>
#include <string>

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
//#include <bsoncxx/stdx/string_view.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

int main(int, char**)
{
    mongocxx::instance inst{};
    mongocxx::pool pool{mongocxx::uri{}};
    auto conn = pool.acquire();

    auto acmacs_version_collection = (*conn)["acmacs_web"]["acmacs_version"];
    auto acmacs_version_cursor = acmacs_version_collection.find({});
    for (auto& doc : acmacs_version_cursor) {
          // std::cout << bsoncxx::to_json(doc) << std::endl;
        // auto view = doc.view();
        const auto num_keys = std::distance(std::begin(doc), std::end(doc));
        std::cout << "DOC: " << num_keys << std::endl;
        for (auto ele : doc) {
            bsoncxx::stdx::string_view key = ele.key();
            bsoncxx::types::value val = ele.get_value();
            std::string value_s;
            switch (val.type()) {
              case bsoncxx::type::k_oid:
                  value_s = "ObjectId(\"" + val.get_oid().value.to_string() + "\")";
                  break;
              case bsoncxx::type::k_utf8:
                  value_s = "\"" + val.get_utf8().value.to_string() + "\"";
                  break;
              default:
                  value_s = std::to_string(static_cast<int>(val.type())) + "(?)";
                  break;
            }
            std::cout << key << ": " << value_s << std::endl;
        }
    }

    // auto charts_collection = (*conn)["acmacs_web"]["charts"];
    // auto charts_cursor = charts_collection.find({});
    // for (auto& doc : charts_cursor) {
    //     std::cout << bsoncxx::to_json(doc) << std::endl;
    // }

}

// ----------------------------------------------------------------------

// void test_db_from_cxx_driver_doc(mongocxx::client& conn)
// {
//     bsoncxx::builder::stream::document document{};

//     auto collection = conn["testdb"]["testcollection"];
//     document << "hello" << "world";

//     collection.insert_one(document.view());
//     auto cursor = collection.find({});

//     for (auto&& doc : cursor) {
//         std::cout << bsoncxx::to_json(doc) << std::endl;
//     }

// } // test_db_from_cxx_driver_doc

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
