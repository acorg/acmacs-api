#pragma once

#include <string>

#include "acmacs-base/from-json.hh"

// ----------------------------------------------------------------------

class SessionId;

class AcmacsC2
{
 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    AcmacsC2();
    ~AcmacsC2();

    inline void uri(std::string aUri) { acmacs_uri = aUri; }
    // inline void session(std::string aSession) { mSession = aSession; }

    from_json::object command(const SessionId& aSession, std::string aCommand);
    void verbose(bool aVerbose) { mVerbose = aVerbose; }

    std::string ace_uncompressed(const SessionId& aSession, std::string aObjectId, size_t aMaxNumberOfProjections);

 private:
    std::string acmacs_uri;
    // std::string mSession;
    bool mVerbose;
    using CURL = void;
    CURL* curl;
    std::string response;

    static size_t response_receiver(const char* contents, size_t size, size_t nmemb, AcmacsC2* self);
    std::string embed_session_in_command(const SessionId& aSession, std::string source);

}; // class AcmacsC2

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
