#include <numeric>

#include <curl/curl.h>

#include "acmacs-api/curl.hh"

// ----------------------------------------------------------------------

acmacs::Curl& acmacs::curl()
{
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
// #pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif
    static Curl curl_;
#pragma GCC diagnostic pop

    return curl_;

} // acmacs::curl

// ----------------------------------------------------------------------

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#endif

// ----------------------------------------------------------------------

acmacs::Curl::Curl()
{
    if (CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT); res != CURLE_OK)
        throw Error{std::string{"curl_global_init failed: "} + curl_easy_strerror(res)};

} // acmacs::Curl::Curl

// ----------------------------------------------------------------------

acmacs::Curl::~Curl()
{
    curl_global_cleanup();

} // acmacs::Curl::~Curl

// ----------------------------------------------------------------------

static size_t response_receiver(const char* contents, size_t memb_size, size_t nmemb, std::string* target)
{
    const auto size = memb_size * nmemb;
    target->append(contents, size);
    return size;
}

// ----------------------------------------------------------------------

rjson::value acmacs::Curl::post(std::string url, std::string data, bool verbose)
{
    auto curl = curl_easy_init();
    if (!curl)
        throw Error{"curl_easy_init failed"};

    std::string response;

    try {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose ? 1L : 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &response_receiver); // response_receiver may be called multiple times for a single response
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        if (verbose)
            std::cerr << "==> " << data << std::endl;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(data.size()));

        if (CURLcode res = curl_easy_perform(curl); res != CURLE_OK)
            throw Error(std::string{"curl_easy_perform failed: "} + url + ": " + std::to_string(res) + ": " + curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }
    catch (...) {
        curl_easy_cleanup(curl);
        throw;
    }

    const auto doc = rjson::parse_string(response);
    if (const auto& errors = doc["E"]; !errors.empty()) {
        std::string msg;
        rjson::for_each(errors, [&msg](const rjson::value& err) {
            if (!msg.empty())
                msg.append("\n       ");
            msg.append(rjson::get_or(err, "code", ""));
            msg.append(": ");
            msg.append(rjson::get_or(err, "description", ""));
        });
        throw Error{msg};
    }

    return doc;

} // acmacs::Curl::post

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
