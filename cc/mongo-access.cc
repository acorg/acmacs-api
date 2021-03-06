#include "acmacs-api/mongo-access.hh"

// ----------------------------------------------------------------------

[[noreturn]] inline static void throw_error(mongocxx::query_exception& err)
{
    const std::string what = err.what();
    if (what.substr(0, 26) == "No suitable servers found:")
        throw DocumentFindResults::Error{"Acmacs mongodb server is down"};
    else
        throw DocumentFindResults::Error{what};
}

// ----------------------------------------------------------------------

void DocumentFindResults::build(const char* aCollection, bson_view aFilter, const mongo_find& aOptions)
{
    try {
        mCursor = std::make_unique<mongocxx::cursor>(find(aCollection, aFilter, aOptions));
    }
    catch (mongocxx::query_exception& err) {
        throw_error(err);
    }

} // DocumentFindResults::build

// ----------------------------------------------------------------------

void DocumentFindResults::build(const char* aCollection)
{
    try {
        mCursor = std::make_unique<mongocxx::cursor>(find(aCollection));
    }
    catch (mongocxx::query_exception& err) {
        throw_error(err);
    }

} // DocumentFindResults::build

// ----------------------------------------------------------------------

std::string StoredInMongodb::create()
{
    auto doc_bld = bld_doc{};
    add_fields_for_creation(doc_bld);
    try {
        auto result = insert_one(doc_bld.extract().view());
        if (!result)
            throw Error{"unacknowledged write during doc insertion"};
        if (result->inserted_id().type() != bsoncxx::type::k_oid)
            throw Error{"cannot insert doc: inserted id was not an OID type"};
        return result->inserted_id().get_oid().value.to_string();
    }
    catch (const mongocxx::exception& err) {
        throw Error{std::string{"cannot insert doc: "} + err.what()};
    }

} // StoredInMongodb::create

// ----------------------------------------------------------------------

void StoredInMongodb::update(std::string aId)
{
    auto doc_set = bld_doc{};
    add_fields_for_updating(doc_set);
    auto result = update_one(to_bson::object("_id", bsoncxx::oid{aId}), to_bson::object("$set", doc_set.extract()));
    if (!result)
        throw Error{"unacknowledged write during doc updating"};

} // StoredInMongodb::update

// ----------------------------------------------------------------------

void StoredInMongodb::remove(std::string aId)
{
    auto result = remove(to_bson::object("_id", bsoncxx::oid{aId}));
    if (!result)
        throw Error{"unacknowledged write during doc removing"};

} // StoredInMongodb::remove

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
