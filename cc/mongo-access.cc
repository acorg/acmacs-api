#include "mongo-access.hh"

// ----------------------------------------------------------------------

template <typename Writer> static std::string json_w(const DocumentFindResults& aResults, std::string key)
{
    Writer writer{"DocumentFindResults"};
    if (!key.empty()) {
        writer << json_writer::start_object
               << json_writer::key(key) << aResults.records()
               << json_writer::end_object;
    }
    else {
        writer << json_writer::start_array;
        for (const auto& record: aResults.records())
            writer << record;
        writer << json_writer::end_array;
    }
    return writer << json_writer::finalize;
}

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

void DocumentFindResults::build(const char* aCollection, doc_value&& aFilter, const mongo_find& aOptions)
{
    try {
        auto found = find(aCollection, std::move(aFilter), aOptions);
        std::copy(std::begin(found), std::end(found), std::back_inserter(mRecords));
    }
    catch (mongocxx::query_exception& err) {
        throw_error(err);
    }

} // DocumentFindResults::build

// ----------------------------------------------------------------------

void DocumentFindResults::build(const char* aCollection)
{
    try {
        auto found = find(aCollection);
        std::copy(std::begin(found), std::end(found), std::back_inserter(mRecords));
    }
    catch (mongocxx::query_exception& err) {
        throw_error(err);
    }

} // DocumentFindResults::build

// ----------------------------------------------------------------------

std::string DocumentFindResults::json(bool pretty, std::string key) const
{
    return pretty ? json_w<json_writer::pretty>(*this, key) : json_w<json_writer::compact>(*this, key);

} // DocumentFindResults::json

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
