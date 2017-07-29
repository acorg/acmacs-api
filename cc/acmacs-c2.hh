#pragma once

#include <string>

// ----------------------------------------------------------------------

class AcmacsC2
{
 public:
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    AcmacsC2();
    ~AcmacsC2();

    inline void uri(std::string aUri) { acmacs_uri = aUri; }

    std::string command(std::string aCommand);
    void verbose(bool aVerbose) { mVerbose = aVerbose; }

 private:
    std::string acmacs_uri;
    bool mVerbose;
    using CURL = void;
    CURL* curl;
    std::string response;

    static size_t response_receiver(const char* contents, size_t size, size_t nmemb, AcmacsC2* self);

}; // class AcmacsC2

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
