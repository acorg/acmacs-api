#include <iostream>
#include <sstream>
#include <string>

#include <functional>
#include <curl/curl.h>

#include "acmacs-base/range.hh"
#include "acmacs-base/string.hh"

#include "acmacs-api/curl.hh"
#include "acmacs-api/acmacs-c2.hh"
#include "acmacs-api/session-id.hh"

// ----------------------------------------------------------------------

rjson::object AcmacsC2::command(const SessionId& aSession, std::string aCommand)
{
    return acmacs::curl().post(acmacs_uri, embed_session_in_command(aSession, aCommand), mVerbose);

} // AcmacsC2::command

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
    const auto projections = "[" + string::join(",", acmacs::index_iterator(0UL), acmacs::index_iterator(aMaxNumberOfProjections)) + "]";
    auto result = command(aSession, std::string{R"({"C":"chart_export","format":"ace_uncompressed","pretty":false,"id":")"} + aObjectId + R"(","projection":)" + projections + "}");
      // "chart_json" is a string with embedded json, all double-quotes are escaped
    return string::replace(result.get_or_default("chart_json", "* C2 chart_export failed without error *"), "\\\"", "\"");

} // AcmacsC2::ace_uncompressed

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
