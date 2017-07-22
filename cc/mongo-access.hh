#pragma once

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
//#include <mongocxx/exception/exception.hpp>
#include <mongocxx/exception/query_exception.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/time.hh"

#include "bson-to-json.hh"

// ----------------------------------------------------------------------

class MongodbAccess
{
 public:
    inline MongodbAccess(mongocxx::database& aDb) : mDb(aDb) {}
    inline MongodbAccess(const MongodbAccess& aSrc) : mDb(aSrc.mDb) {}
    virtual inline ~MongodbAccess() {}

    using bson_doc = bsoncxx::builder::stream::document;
    static constexpr const auto bson_finalize = bsoncxx::builder::stream::finalize;
    static constexpr const auto bson_open_document = bsoncxx::builder::stream::open_document;
    static constexpr const auto bson_close_document = bsoncxx::builder::stream::close_document;
    static constexpr const auto bson_open_array = bsoncxx::builder::stream::open_array;
    static constexpr const auto bson_close_array = bsoncxx::builder::stream::close_array;
    static constexpr const auto bson_null = bsoncxx::types::b_null{};

    using doc_value = bsoncxx::document::value;
    using doc_view = bsoncxx::document::view;
    using cursor = mongocxx::cursor;
    using find_options = mongocxx::options::find;

      // ----------------------------------------------------------------------

    class exclude
    {
     public:
        inline exclude(std::initializer_list<std::string>&& fields)
            {
                auto proj_doc = bson_doc{};
                std::for_each(std::begin(fields), std::end(fields), [&proj_doc](const auto& field) { proj_doc << field << false; });
                mOptions.projection(proj_doc << bson_finalize);
            }

        template <typename ... Args> inline exclude(Args&& ...) : exclude({Args::value...}) {}

        inline operator const find_options& () const { return mOptions; }

     private:
        find_options mOptions;

    }; // class exclude

    class include_exclude
    {
     public:
        inline include_exclude(std::initializer_list<std::string>&& include, std::initializer_list<std::string>&& exclude)
            {
                auto proj_doc = bson_doc{};
                std::for_each(std::begin(include), std::end(include), [&proj_doc](const auto& field) { proj_doc << field << true; });
                std::for_each(std::begin(exclude), std::end(exclude), [&proj_doc](const auto& field) { proj_doc << field << false; });
                mOptions.projection(proj_doc << bson_finalize);
            }

        inline operator const find_options& () const { return mOptions; }

     private:
        find_options mOptions;

    }; // class include_exclude

      // ----------------------------------------------------------------------

    inline auto find(const char* aCollection, doc_value&& aFilter, const find_options& aOptions = find_options{})
        {
            return mDb[aCollection].find(std::move(aFilter), aOptions);
        }

    inline auto find(const char* aCollection)
        {
            return mDb[aCollection].find({});
        }

    inline auto find_one(const char* aCollection, doc_value&& aFilter, const find_options& aOptions = find_options{})
        {
            return mDb[aCollection].find_one(std::move(aFilter), aOptions);
        }

    inline auto insert_one(const char* aCollection, doc_value&& aDoc)
        {
            return mDb[aCollection].insert_one(std::move(aDoc));
        }

    inline auto update_one(const char* aCollection, doc_value&& aFilter, doc_value&& aDoc)
        {
            return mDb[aCollection].update_one(std::move(aFilter), std::move(aDoc));
        }

      // ----------------------------------------------------------------------

    inline std::string time_now() const
        {
            return time_format_gm(std::chrono::system_clock::now(), "%F %T");
        }

    inline std::string time_in_seconds(std::int32_t aSeconds) const
        {
            return time_format_gm(std::chrono::system_clock::now() + std::chrono::seconds{aSeconds}, "%F %T");
        }

      // ----------------------------------------------------------------------

    inline mongocxx::database& db() { return mDb; }

 private:
    mongocxx::database& mDb;

}; // class MongodbAccess

// ----------------------------------------------------------------------

class DocumentFindResults : public MongodbAccess
{
 private:
    template <typename Writer> inline std::string json_w(std::string key) const
        {
            Writer writer{"DocumentFindResults"};
            writer << json_writer::start_object
                    << json_writer::key(key) << mRecords
                    << json_writer::end_object;
            return writer << json_writer::finalize;
        }

 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    inline DocumentFindResults(mongocxx::database& aDb) : MongodbAccess{aDb} {}
    inline DocumentFindResults(mongocxx::database& aDb, const char* aCollection) : MongodbAccess{aDb} { build(aCollection); }
    inline DocumentFindResults(mongocxx::database& aDb, const char* aCollection, doc_value&& aFilter, const find_options& aOptions = find_options{})
        : MongodbAccess{aDb} { build(aCollection, std::move(aFilter), aOptions); }

    inline void build(const char* aCollection, doc_value&& aFilter, const find_options& aOptions = find_options{})
        {
            try {
                auto found = find(aCollection, std::move(aFilter), aOptions);
                std::copy(std::begin(found), std::end(found), std::back_inserter(mRecords));
            }
            catch (mongocxx::query_exception& err) {
                throw_error(err);
            }
        }

    inline void build(const char* aCollection)
        {
            try {
                auto found = find(aCollection);
                std::copy(std::begin(found), std::end(found), std::back_inserter(mRecords));
            }
            catch (mongocxx::query_exception& err) {
                throw_error(err);
            }
        }

    inline std::string json(bool pretty = true, std::string key = "results") const
        {
            return pretty ? json_w<json_writer::pretty>(key) : json_w<json_writer::compact>(key);
        }

 private:
    std::vector<doc_view> mRecords;

    [[noreturn]] inline void throw_error(mongocxx::query_exception& err)
        {
            const std::string what = err.what();
            if (what.substr(0, 26) == "No suitable servers found:")
                throw Error{"Acmacs mongodb server is down"};
            else
                throw Error{what};
        }

}; // class DocumentFindResults

// ----------------------------------------------------------------------

class StoredInMongodb : public MongodbAccess
{
 public:
    inline StoredInMongodb(mongocxx::database& aDb, const char* aCollection) : MongodbAccess{aDb}, mCollection{aCollection} {}

 protected:
    using bson_key_context = bsoncxx::builder::stream::key_context<bsoncxx::builder::stream::key_context<bsoncxx::builder::stream::closed_context>>;

    using MongodbAccess::find;
    inline auto find(doc_value&& aFilter, const find_options& aOptions = find_options{}) { return find(mCollection, std::move(aFilter), aOptions); }
    inline auto find() { return find(mCollection); }
    using MongodbAccess::find_one;
    inline auto find_one(doc_value&& aFilter, const find_options& aOptions = find_options{}) { return find_one(mCollection, std::move(aFilter), aOptions); }
    inline auto insert_one(doc_value&& aDoc) { return MongodbAccess::insert_one(mCollection, std::move(aDoc)); }
    inline auto update_one(doc_value&& aFilter, doc_value&& aDoc) { return MongodbAccess::update_one(mCollection, std::move(aFilter), std::move(aDoc)); }

    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

      // returns new _id
      // throws Error
    inline std::string create()
        {
            auto doc = bson_doc{};
            add_fields_for_creation(doc);
            try {
                auto result = insert_one(doc << bson_finalize);
                if (!result)
                    throw Error{"unacknowledged write during doc insertion"};
                if (result->inserted_id().type() != bsoncxx::type::k_oid)
                    throw Error{"cannot insert doc: inserted id was not an OID type"};
                return result->inserted_id().get_oid().value.to_string();
            }
            catch (const mongocxx::exception& err) {
                throw Error{std::string{"cannot insert doc: "} + err.what()};
            }
        }

      // throws Error
    inline void update(std::string aId)
        {
            auto doc_set = bson_doc{};
            add_fields_for_updating(doc_set);
            auto result = update_one(bson_doc{} << "_id" << bsoncxx::oid{aId} << bson_finalize,
                                     bson_doc{} << "$set" << bson_open_document << bsoncxx::builder::concatenate(doc_set.view()) << bson_close_document << bson_finalize);
            if (!result)
                throw Error{"unacknowledged write during doc updating"};
        }

    virtual inline void add_fields_for_creation(bson_doc& aDoc)
        {
            aDoc << "_m" << time_now();
        }

    virtual inline void add_fields_for_updating(bson_doc& aDoc)
        {
            aDoc << "_m" << time_now();
        }

 private:
    const char* mCollection;

}; // class StoredInMongodb


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
