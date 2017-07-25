#include "mongo-access.hh"
#include "bson-to-json.hh"

// ----------------------------------------------------------------------

template <typename Writer> static std::string json_w(DocumentFindResults& aResults, std::string key)
{
    Writer writer;
    if (!key.empty())
        writer << json_writer::start_object << json_writer::key(key);
    writer << json_writer::start_array;
    for (auto& record: aResults.cursor()) {
        writer << record;
        aResults.increment_count();
    }
    writer << json_writer::end_array;
    if (!key.empty())
        writer << json_writer::end_object;
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

std::string DocumentFindResults::json(bool pretty, std::string key)
{
    return pretty ? json_w<json_writer::pretty>(*this, key) : json_w<json_writer::compact>(*this, key);

} // DocumentFindResults::json

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
