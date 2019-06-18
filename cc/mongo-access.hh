#pragma once

#include <iostream>
#include <memory>

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/exception/query_exception.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/time.hh"
#include "acmacs-webserver/print.hh"

#include "bson.hh"

// ----------------------------------------------------------------------

class MongodbAccess
{
 public:
    MongodbAccess(mongocxx::database aDb) : mDb(aDb) { if (!mDb) throw std::runtime_error("MongodbAccess::MongodbAccess: invalid mDb"); }
    MongodbAccess(const MongodbAccess& aSrc) : mDb(aSrc.mDb) {}
    virtual ~MongodbAccess() {}

    using bld_doc = bsoncxx::builder::basic::document;
    using bld_array = bsoncxx::builder::basic::array;

    using bson_value = bsoncxx::document::value;
    using bson_view = bsoncxx::document::view;
    static constexpr const auto bson_null = bsoncxx::types::b_null{};

    using cursor = mongocxx::cursor;
    using mongo_find = mongocxx::options::find;

      // ----------------------------------------------------------------------

    class find_options
    {
     public:
        find_options() = default;
        find_options(find_options&&) = default;

        find_options& exclude(std::initializer_list<std::string>&& fields)
            {
                std::for_each(std::begin(fields), std::end(fields), [this](const auto& field) { to_bson::append(mProjection, field, false); });
                return *this;
            }

        template <typename ... Args> find_options& exclude(Args ... args) { return exclude({args ...}); }

        find_options& include(std::initializer_list<std::string>&& fields)
            {
                std::for_each(std::begin(fields), std::end(fields), [this](const auto& field) { to_bson::append(mProjection, field, true); });
                return *this;
            }

        template <typename ... Args> find_options& include(Args ... args) { return include({args ...}); }

        find_options& sort(std::string field, int order = 1)
            {
                to_bson::append(mSort, field, order);
                return *this;
            }

        template <typename ... Args> find_options& sort(std::string field, int order, Args ... args)
            {
                sort(field, order);
                return sort(args ...);
            }

        find_options& skip(std::int32_t skip) { mOptions.skip(skip); return *this; }
        find_options& limit(std::int32_t limit) { mOptions.limit(limit); return *this; }

        operator mongo_find& ()
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
        template <typename ... Args> exclude(Args ... args) { find_options::exclude(args ...); }
    };

    class include : public find_options
    {
     public:
        template <typename ... Args> include(Args ... args) { find_options::include(args ...); }
    };

    class sort : public find_options
    {
     public:
        template <typename ... Args> sort(Args ... args) { find_options::sort(args ...); }
    };

    class skip_limit : public find_options
    {
     public:
        skip_limit(std::int32_t skip, std::int32_t limit) { find_options::skip(skip); find_options::limit(limit); }
    };

      // ----------------------------------------------------------------------

    mongocxx::collection collection(const char* aCollection)
        {
            return mDb.collection(aCollection);
        }

    auto find(const char* aCollection, bson_view aFilter, const mongo_find& aOptions = mongo_find{})
        {
            return collection(aCollection).find(aFilter, aOptions);
        }

    auto find(const char* aCollection)
        {
            return collection(aCollection).find({});
        }

    auto distinct(const char* aCollection, const char* aField, bson_view aFilter)
        {
            return collection(aCollection).distinct(std::string{aField}, aFilter);
        }

    auto find_one(const char* aCollection, bson_view aFilter, const mongo_find& aOptions = mongo_find{})
        {
            return collection(aCollection).find_one(aFilter, aOptions);
        }

    auto insert_one(const char* aCollection, bson_view aDoc)
        {
            return collection(aCollection).insert_one(aDoc);
        }

    auto update_one(const char* aCollection, bson_view aFilter, bson_view aDoc)
        {
            return collection(aCollection).update_one(aFilter, aDoc);
        }

    auto remove(const char* aCollection, bson_view aFilter)
        {
            return collection(aCollection).delete_one(aFilter);
        }

      // ----------------------------------------------------------------------

    std::string time_now() const
        {
            return acmacs::time_format_gm(std::chrono::system_clock::now(), "%F %T");
        }

    std::string time_in_seconds(std::int32_t aSeconds) const
        {
            return acmacs::time_format_gm(std::chrono::system_clock::now() + std::chrono::seconds{aSeconds}, "%F %T");
        }

      // ----------------------------------------------------------------------

    static inline bson_value field_null_or_absent(std::string aField)
        {
            return to_bson::object("$or", to_bson::array(
                                       to_bson::object(aField, to_bson::object("$exists", false)),
                                       to_bson::object(aField, to_bson::object("$eq", bson_null))
                                                          ));
        }

 private:
    mongocxx::database mDb;

}; // class MongodbAccess

// ----------------------------------------------------------------------

class DocumentFindResults : public MongodbAccess
{
 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    DocumentFindResults(mongocxx::database aDb) : MongodbAccess{aDb}, mCount{0} {}
    DocumentFindResults(mongocxx::database aDb, const char* aCollection) : MongodbAccess{aDb}, mCount{0} { build(aCollection); }
    DocumentFindResults(mongocxx::database aDb, const char* aCollection, bson_view aFilter, const mongo_find& aOptions = mongo_find{})
        : MongodbAccess{aDb}, mCount{0} { build(aCollection, aFilter, aOptions); }
    DocumentFindResults(mongocxx::database aDb, const char* aCollection, const bson_value& aFilter, const mongo_find& aOptions = mongo_find{})
        : DocumentFindResults(aDb, aCollection, aFilter.view(), aOptions) {}

    std::string json(std::string key) { return to_json::v1::object(key, *this); }
    std::string json();

    size_t count() const { return mCount; }
    void increment_count() { ++mCount; }
    auto& cursor() { return *mCursor.get(); }

 private:
    std::unique_ptr<mongocxx::cursor> mCursor;
    size_t mCount;

    void build(const char* aCollection, bson_view aFilter, const mongo_find& aOptions = mongo_find{});
    void build(const char* aCollection);

}; // class DocumentFindResults

namespace to_json::v1
{
    template <> inline std::string value(DocumentFindResults& aResults)
    {
        std::string target;
        for (auto& record: aResults.cursor()) {
            target = internal::array_append(target, record);
            aResults.increment_count();
        }
        return target;
    }

} // namespace to_json

inline std::string DocumentFindResults::json() { return to_json::v1::value(*this); }

// ----------------------------------------------------------------------

class StoredInMongodb : public MongodbAccess
{
 public:
    StoredInMongodb(mongocxx::database aDb, const char* aCollection) : MongodbAccess{aDb}, mCollection{aCollection} {}

 protected:
    using MongodbAccess::find;
    auto find(bson_view aFilter, const mongo_find& aOptions = mongo_find{}) { return find(mCollection, aFilter, aOptions); }
    auto find() { return find(mCollection); }
    using MongodbAccess::find_one;
    auto find_one(bson_view aFilter, const mongo_find& aOptions = mongo_find{}) { return find_one(mCollection, aFilter, aOptions); }
    auto insert_one(bson_view aDoc) { return MongodbAccess::insert_one(mCollection, aDoc); }
    auto update_one(bson_view aFilter, bson_view aDoc) { return MongodbAccess::update_one(mCollection, aFilter, aDoc); }
    auto remove(bson_view aFilter) { return MongodbAccess::remove(mCollection, aFilter); }

    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

      // returns new _id
      // throws Error
    std::string create();

      // throws Error
    void update(std::string aId);
    void remove(std::string aId);

    virtual void add_fields_for_creation(bld_doc& aDoc)
        {
            to_bson::append(aDoc, "_m", time_now());
        }

    virtual void add_fields_for_updating(bld_doc& aDoc)
        {
            to_bson::append(aDoc, "_m", time_now());
        }

 private:
    const char* mCollection;

}; // class StoredInMongodb


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
