#pragma once

#include <memory>

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
                std::for_each(std::begin(fields), std::end(fields), [this](const auto& field) { mProjection << field << false; });
                return *this;
            }

        template <typename ... Args> inline find_options& exclude(Args ... args) { return exclude({args ...}); }

        inline find_options& include(std::initializer_list<std::string>&& fields)
            {
                std::for_each(std::begin(fields), std::end(fields), [this](const auto& field) { mProjection << field << true; });
                return *this;
            }

        template <typename ... Args> inline find_options& include(Args ... args) { return include({args ...}); }

        inline find_options& sort(std::string field, int order = 1)
            {
                mSort << field << order;
                return *this;
            }

        template <typename ... Args> inline find_options& sort(std::string field, int order, Args ... args)
            {
                sort(field, order);
                return sort(args ...);
            }

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
        bson_doc mProjection;
        bson_doc mSort;

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

      // ----------------------------------------------------------------------

    inline auto find(const char* aCollection, doc_value&& aFilter, const mongo_find& aOptions = mongo_find{})
        {
            return mDb[aCollection].find(std::move(aFilter), aOptions);
        }

    inline auto find(const char* aCollection)
        {
            return mDb[aCollection].find({});
        }

    inline auto find_one(const char* aCollection, doc_value&& aFilter, const mongo_find& aOptions = mongo_find{})
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

    static inline bson_doc field_null_or_absent(std::string aField)
        {
            auto doc = bson_doc{};
            doc << "$or" << bson_open_array
                << bson_open_document << aField << bson_open_document << "$exists" << false << bson_close_document << bson_close_document
                << bson_open_document << aField << bson_open_document << "$eq" << bson_null << bson_close_document << bson_close_document
                << bson_close_array;
            return doc;
        }

      // ----------------------------------------------------------------------

    inline mongocxx::database& db() { return mDb; }

 private:
    mongocxx::database& mDb;

}; // class MongodbAccess

// ----------------------------------------------------------------------

inline auto operator <= (MongodbAccess::bson_doc&& left, MongodbAccess::bson_doc&& right)
{
    return left << bsoncxx::builder::concatenate(right.view());
}

inline auto operator <= (MongodbAccess::key_context&& left, MongodbAccess::bson_doc&& right)
{
    return left << bsoncxx::builder::concatenate(right.view());
}

inline auto operator <= (MongodbAccess::key_context&& left, decltype(MongodbAccess::bson_finalize))
{
    return left << MongodbAccess::bson_finalize;
}

// ----------------------------------------------------------------------

class DocumentFindResults : public MongodbAccess
{
 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    inline DocumentFindResults(mongocxx::database& aDb) : MongodbAccess{aDb}, mCount{0} {}
    inline DocumentFindResults(mongocxx::database& aDb, const char* aCollection) : MongodbAccess{aDb}, mCount{0} { build(aCollection); }
    inline DocumentFindResults(mongocxx::database& aDb, const char* aCollection, doc_value&& aFilter, const mongo_find& aOptions = mongo_find{})
        : MongodbAccess{aDb}, mCount{0} { build(aCollection, std::move(aFilter), aOptions); }

    std::string json(bool pretty = true, std::string key = std::string{});

    inline size_t count() const { return mCount; }
    inline void increment_count() { ++mCount; }
    inline auto& cursor() { return *mCursor.get(); }

 private:
    std::unique_ptr<mongocxx::cursor> mCursor;
    size_t mCount;

    void build(const char* aCollection, doc_value&& aFilter, const mongo_find& aOptions = mongo_find{});
    void build(const char* aCollection);

}; // class DocumentFindResults

// ----------------------------------------------------------------------

class StoredInMongodb : public MongodbAccess
{
 public:
    inline StoredInMongodb(mongocxx::database& aDb, const char* aCollection) : MongodbAccess{aDb}, mCollection{aCollection} {}

 protected:
    using bson_key_context = bsoncxx::builder::stream::key_context<bsoncxx::builder::stream::key_context<bsoncxx::builder::stream::closed_context>>;

    using MongodbAccess::find;
    inline auto find(doc_value&& aFilter, const mongo_find& aOptions = mongo_find{}) { return find(mCollection, std::move(aFilter), aOptions); }
    inline auto find() { return find(mCollection); }
    using MongodbAccess::find_one;
    inline auto find_one(doc_value&& aFilter, const mongo_find& aOptions = mongo_find{}) { return find_one(mCollection, std::move(aFilter), aOptions); }
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
