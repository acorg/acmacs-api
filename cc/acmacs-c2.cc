#include <iostream>
#include <sstream>
#include <string>

#include <functional>
#include <curl/curl.h>

#include "acmacs-base/range.hh"
#include "acmacs-base/stream.hh"
#include "acmacs-base/rapidjson.hh"

#include "acmacs-c2.hh"

// #pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#endif
// #pragma GCC diagnostic pop

// ----------------------------------------------------------------------

AcmacsC2::AcmacsC2()
    : acmacs_uri{"https://localhost:1168/api"}, mVerbose{false}, curl{nullptr}
{
    if (CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT); res != CURLE_OK)
        throw Error{std::string{"curl_global_init failed: "} + curl_easy_strerror(res)};
    curl = curl_easy_init();
    if (!curl)
        throw Error{"curl_easy_init failed"};

} // AcmacsC2::AcmacsC2

// ----------------------------------------------------------------------

AcmacsC2::~AcmacsC2()
{
    curl_easy_cleanup(curl);
    curl_global_cleanup();

} // AcmacsC2::~AcmacsC2

// ----------------------------------------------------------------------

std::string AcmacsC2::command(std::string aCommand)
{
    if (!curl)
        throw Error{"curl not initialized"};

      // https://curl.haxx.se/libcurl/c/postinmemory.html
    curl_easy_setopt(curl, CURLOPT_VERBOSE, mVerbose ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_URL, acmacs_uri.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    response.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &AcmacsC2::response_receiver); // response_receiver may be called multiple times for a single response
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    if (mVerbose)
        std::cerr << "==> " << aCommand << std::endl;
    aCommand = embed_session_in_command(aCommand);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, aCommand.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(aCommand.size()));

    if (CURLcode res = curl_easy_perform(curl); res != CURLE_OK)
        throw Error(std::string{"curl_easy_perform failed: "} + curl_easy_strerror(res));

    return response;

} // AcmacsC2::command

// ----------------------------------------------------------------------

size_t AcmacsC2::response_receiver(const char* contents, size_t memb_size, size_t nmemb, AcmacsC2* self)
{
    const auto size = memb_size * nmemb;
    self->response.append(contents, size);
    return size;

} // AcmacsC2::response_receiver

// ----------------------------------------------------------------------

std::string AcmacsC2::embed_session_in_command(std::string source)
{
    std::string result;
    if (source.find("\"S\"") == std::string::npos) {
        result = source.substr(0, source.size() - 1) + ",\"S\":\"" + mSession + "\"}";
    }
    else {
        result = source;
    }
    return result;

} // AcmacsC2::embed_session_in_command

// ----------------------------------------------------------------------

std::string AcmacsC2::ace_uncompressed(std::string aObjectId, size_t aMaxNumberOfProjections)
{
    std::vector<size_t> projections{Range<size_t>::begin(aMaxNumberOfProjections), Range<size_t>::end()};
    std::ostringstream os;
    os << projections;
    std::string result = command(std::string{R"({"C":"chart_export","format":"ace_uncompressed","pretty":false,"id":")"} + aObjectId + R"(","projection":)" + os.str() + "}");
      // "chart_json" is a string with embedded json
    json_importer::Object doc{result};
    return doc.get_string("chart_json");

} // AcmacsC2::ace_uncompressed

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
