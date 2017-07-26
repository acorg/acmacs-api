#pragma once

#include <iostream>
#include <memory>

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
//#include <mongocxx/exception/exception.hpp>
#include <mongocxx/exception/query_exception.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/time.hh"

#include "bson.hh"

// ----------------------------------------------------------------------

class MongodbAccess
{
 public:
    inline MongodbAccess(mongocxx::database& aDb) : mDb(aDb) {}
    inline MongodbAccess(const MongodbAccess& aSrc) : mDb(aSrc.mDb) {}
    virtual inline ~MongodbAccess() {}

    using bld_doc = bsoncxx::builder::basic::document;
    using bld_array = bsoncxx::builder::basic::array;

    using bson_value = bsoncxx::document::value;
    using bson_view = bsoncxx::document::view;
    static constexpr const auto bson_null = bsoncxx::types::b_null{};

    using cursor = mongocxx::cursor;
    using mongo_find = mongocxx::options::find;

    using key_context = bsoncxx::builder::stream::key_context<bsoncxx::builder::stream::closed_context>;

      // ----------------------------------------------------------------------

    class find_options
    {
     public:
        inline find_options() = default;
        inline find_options(find_options&&) = default;

        inline find_options& exclude(std::initializer_list<std::string>&& fields)
            {
                std::for_each(std::begin(fields), std::end(fields), [this](const auto& field) { bson_append(mProjection, field, false); });
                return *this;
            }

        template <typename ... Args> inline find_options& exclude(Args ... args) { return exclude({args ...}); }

        inline find_options& include(std::initializer_list<std::string>&& fields)
            {
                std::for_each(std::begin(fields), std::end(fields), [this](const auto& field) { bson_append(mProjection, field, true); });
                return *this;
            }

        template <typename ... Args> inline find_options& include(Args ... args) { return include({args ...}); }

        inline find_options& sort(std::string field, int order = 1)
            {
                bson_append(mSort, field, order);
                return *this;
            }

        template <typename ... Args> inline find_options& sort(std::string field, int order, Args ... args)
            {
                sort(field, order);
                return sort(args ...);
            }

        inline find_options& skip(std::int32_t skip) { mOptions.skip(skip); return *this; }
        inline find_options& limit(std::int32_t limit) { mOptions.limit(limit); return *this; }

        inline operator mongo_find& ()
            {
                const auto projection = mProjection.view();
                if (!projection.empty())
                    mOptions.projection(projection);
                const auto sort = mSort.view();
                if (!sort.empty())
                    mOptions.sort(sort);
                return mOptions;
            }

     private:
        mongo_find mOptions;
        bld_doc mProjection;
        bld_doc mSort;

    }; // class find_options

    class exclude : public find_options
    {
     public:
        template <typename ... Args> inline exclude(Args ... args) { find_options::exclude(args ...); }
    };

    class include : public find_options
    {
     public:
        template <typename ... Args> inline include(Args ... args) { find_options::include(args ...); }
    };

    class sort : public find_options
    {
     public:
        template <typename ... Args> inline sort(Args ... args) { find_options::sort(args ...); }
    };

    class skip_limit : public find_options
    {
     public:
        inline skip_limit(std::int32_t skip, std::int32_t limit) { find_options::skip(skip); find_options::limit(limit); }
    };

      // ----------------------------------------------------------------------

    inline auto find(const char* aCollection, bson_view aFilter, const mongo_find& aOptions = mongo_find{})
        {
            return mDb[aCollection].find(aFilter, aOptions);
        }

    inline auto find(const char* aCollection)
        {
            return mDb[aCollection].find({});
        }

    inline auto distinct(const char* aCollection, const char* aField, bson_view aFilter)
        {
            return mDb[aCollection].distinct(std::string{aField}, aFilter);
        }

    inline auto find_one(const char* aCollection, bson_view aFilter, const mongo_find& aOptions = mongo_find{})
        {
            return mDb[aCollection].find_one(aFilter, aOptions);
        }

    inline auto insert_one(const char* aCollection, bson_view aDoc)
        {
            return mDb[aCollection].insert_one(aDoc);
        }

    inline auto update_one(const char* aCollection, bson_view aFilter, bson_view aDoc)
        {
            return mDb[aCollection].update_one(aFilter, aDoc);
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

    static inline bson_value field_null_or_absent(std::string aField)
        {
            return bson_object("$or", bson_array(
                                       bson_object(aField, bson_object("$exists", false)),
                                       bson_object(aField, bson_object("$eq", bson_null))
                                                          ));
        }

      // ----------------------------------------------------------------------

    inline mongocxx::database& db() { return mDb; }

 private:
    mongocxx::database& mDb;

}; // class MongodbAccess

// ----------------------------------------------------------------------

class DocumentFindResults : public MongodbAccess
{
 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    inline DocumentFindResults(mongocxx::database& aDb) : MongodbAccess{aDb}, mCount{0} {}
    inline DocumentFindResults(mongocxx::database& aDb, const char* aCollection) : MongodbAccess{aDb}, mCount{0} { build(aCollection); }
    inline DocumentFindResults(mongocxx::database& aDb, const char* aCollection, bson_view aFilter, const mongo_find& aOptions = mongo_find{})
        : MongodbAccess{aDb}, mCount{0} { build(aCollection, aFilter, aOptions); }
    inline DocumentFindResults(mongocxx::database& aDb, const char* aCollection, const bson_value& aFilter, const mongo_find& aOptions = mongo_find{})
        : DocumentFindResults(aDb, aCollection, aFilter.view(), aOptions) {}

    std::string json(bool pretty = true, std::string key = std::string{});

    inline size_t count() const { return mCount; }
    inline void increment_count() { ++mCount; }
    inline auto& cursor() { return *mCursor.get(); }

 private:
    std::unique_ptr<mongocxx::cursor> mCursor;
    size_t mCount;

    void build(const char* aCollection, bson_view aFilter, const mongo_find& aOptions = mongo_find{});
    void build(const char* aCollection);

}; // class DocumentFindResults

// ----------------------------------------------------------------------

class StoredInMongodb : public MongodbAccess
{
 public:
    inline StoredInMongodb(mongocxx::database& aDb, const char* aCollection) : MongodbAccess{aDb}, mCollection{aCollection} {}

 protected:
    using bld_key_context = bsoncxx::builder::stream::key_context<bsoncxx::builder::stream::key_context<bsoncxx::builder::stream::closed_context>>;

    using MongodbAccess::find;
    inline auto find(bson_view aFilter, const mongo_find& aOptions = mongo_find{}) { return find(mCollection, aFilter, aOptions); }
    inline auto find() { return find(mCollection); }
    using MongodbAccess::find_one;
    inline auto find_one(bson_view aFilter, const mongo_find& aOptions = mongo_find{}) { return find_one(mCollection, aFilter, aOptions); }
    inline auto insert_one(bson_view aDoc) { return MongodbAccess::insert_one(mCollection, aDoc); }
    inline auto update_one(bson_view aFilter, bson_view aDoc) { return MongodbAccess::update_one(mCollection, aFilter, aDoc); }

    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

      // returns new _id
      // throws Error
    inline std::string create()
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
        }

      // throws Error
    inline void update(std::string aId)
        {
            auto doc_set = bld_doc{};
            add_fields_for_updating(doc_set);
            auto result = update_one(bson_object("_id", bsoncxx::oid{aId}), bson_object("$set", doc_set.extract()));
            if (!result)
                throw Error{"unacknowledged write during doc updating"};
        }

    virtual inline void add_fields_for_creation(bld_doc& aDoc)
        {
            bson_append(aDoc, "_m", time_now());
        }

    virtual inline void add_fields_for_updating(bld_doc& aDoc)
        {
            bson_append(aDoc, "_m", time_now());
        }

 private:
    const char* mCollection;

}; // class StoredInMongodb


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
