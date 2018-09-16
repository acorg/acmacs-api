#pragma once

#include <string>

#include "acmacs-base/rjson.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
      // singleton, do not instatiate!
    class Curl
    {
     public:
        class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

        rjson::value post(std::string url, std::string data, bool verbose);

     private:
        Curl();
        ~Curl();

        // using CURL = void;
        // CURL* curl_ = nullptr;

        friend Curl& curl();

    }; // class Curl

      // singleton access
    Curl& curl();

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
