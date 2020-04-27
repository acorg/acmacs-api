#pragma once

#include <string>

#include "acmacs-base/rjson-v2.hh"

// ----------------------------------------------------------------------

class SessionId;

class AcmacsC2
{
 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    AcmacsC2() = default;

    void uri(std::string aUri) { acmacs_uri = aUri; }

    rjson::value command(const SessionId& aSession, std::string aCommand);
    void verbose(bool aVerbose) { mVerbose = aVerbose; }

    std::string ace_uncompressed(const SessionId& aSession, std::string aObjectId, size_t aMaxNumberOfProjections = static_cast<size_t>(-1));

 private:
    std::string acmacs_uri{"https://localhost:1168/api"};
    bool mVerbose = false;

    std::string embed_session_in_command(const SessionId& aSession, std::string source);

}; // class AcmacsC2

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
