#include <iostream>
#include <sstream>
#include <string>

#include <functional>
#include <curl/curl.h>

#include "acmacs-base/range.hh"
#include "acmacs-base/string.hh"

#include "acmacs-c2.hh"
#include "session-id.hh"

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

from_json::object AcmacsC2::command(const SessionId& aSession, std::string aCommand)
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
    aCommand = embed_session_in_command(aSession, aCommand);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, aCommand.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(aCommand.size()));

    if (CURLcode res = curl_easy_perform(curl); res != CURLE_OK)
        throw Error(std::string{"curl_easy_perform failed: "} + curl_easy_strerror(res));

    from_json::object doc{response};
    try {
        std::string msg;
        for (const auto& entry: doc.get_array("E")) {
            if (!msg.empty())
                msg += "\n       ";
            msg += from_json::get(entry, "code", std::string{}) + ": " + from_json::get(entry, "description", std::string{});
        }
        throw Error{msg};
    }
    catch (from_json::rapidjson_assert&) {
          // no "E" - no error
    }

    return doc;

} // AcmacsC2::command

// ----------------------------------------------------------------------

size_t AcmacsC2::response_receiver(const char* contents, size_t memb_size, size_t nmemb, AcmacsC2* self)
{
    const auto size = memb_size * nmemb;
    self->response.append(contents, size);
    return size;

} // AcmacsC2::response_receiver

// ----------------------------------------------------------------------

std::string AcmacsC2::embed_session_in_command(const SessionId& aSession, std::string source)
{
    std::string result;
    if (source.find("\"S\"") == std::string::npos) {
        result = source.substr(0, source.size() - 1) + ",\"S\":\"" + aSession + "\"}";
    }
    else {
        result = source;
    }
    return result;

} // AcmacsC2::embed_session_in_command

// ----------------------------------------------------------------------

std::string AcmacsC2::ace_uncompressed(const SessionId& aSession, std::string aObjectId, size_t aMaxNumberOfProjections)
{
    const auto projections = "[" + string::join(",", acmacs::incrementer<size_t>::begin(0), acmacs::incrementer<size_t>::end(aMaxNumberOfProjections)) + "]";
    auto result = command(aSession, std::string{R"({"C":"chart_export","format":"ace_uncompressed","pretty":false,"id":")"} + aObjectId + R"(","projection":)" + projections + "}");
      // "chart_json" is a string with embedded json
    return result.get_string("chart_json");

} // AcmacsC2::ace_uncompressed

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
