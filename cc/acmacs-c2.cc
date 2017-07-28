#include <iostream>
#include <vector>
#include <string>
#include <getopt.h>

#include <functional>
#include <curl/curl.h>

// #pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#endif
// #pragma GCC diagnostic pop

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
    CURL* curl;
    std::string response;

    static size_t response_receiver(const char* contents, size_t size, size_t nmemb, AcmacsC2* self);

}; // class AcmacsC2

// ----------------------------------------------------------------------

inline AcmacsC2::AcmacsC2()
    : acmacs_uri{"https://localhost:1168/api"}, mVerbose{false}, curl{nullptr}
{
    if (CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT); res != CURLE_OK)
        throw Error{std::string{"curl_global_init failed: "} + curl_easy_strerror(res)};
    curl = curl_easy_init();
    if (!curl)
        throw Error{"curl_easy_init failed"};

} // AcmacsC2::AcmacsC2

// ----------------------------------------------------------------------

inline AcmacsC2::~AcmacsC2()
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

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &AcmacsC2::response_receiver);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

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
    self->response.assign(contents, size);
    return size;

} // AcmacsC2::response_receiver

// ----------------------------------------------------------------------

struct Args
{
    inline Args() : acmacs_uri{"https://localhost:1168/api"}, verbose{false} {}
    std::string acmacs_uri;
    std::string session;
    std::vector<std::string> commands;
    bool verbose;
};

static void parse_command_line(int argc, char* const argv[], Args& aArgs);
// static size_t response(char* contents, size_t size, size_t nmemb, void* userp);

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    // const char* postthis = R"({"C":"version"})";
    int exit_code = 0;
    try {
        Args args;
        parse_command_line(argc, argv, args);

        AcmacsC2 acmacs;
        acmacs.uri(args.acmacs_uri);
        acmacs.verbose(args.verbose);
        for (const auto& command: args.commands) {
            std::cout << "==> " << command << std::endl;
            const auto response = acmacs.command(command);
            std::cout << "<== " << response << std::endl;
        }
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << std::endl;
        exit_code = 1;
    }
    return exit_code;

} // main

// ----------------------------------------------------------------------

// size_t response(char* contents, size_t size, size_t nmemb, void* /*userp*/)
// {
//     const auto real_size = size * nmemb;
//     std::cout << "Response " << real_size << " [" << std::string{contents, real_size} << ']' << std::endl;
//     return real_size;

// } // response

// ----------------------------------------------------------------------

void parse_command_line(int argc, char* const argv[], Args& aArgs)
{
    const char* const short_opts = "a:s:vh";
    const option long_opts[] = {
        {"acmacs", required_argument, nullptr, 'a'},
        {"session", required_argument, nullptr, 's'},
        {"verbose", no_argument, nullptr, 'v'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, no_argument, nullptr, 0}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
        switch (opt) {
          case 'a':
              aArgs.acmacs_uri = optarg;
              break;
          case 's':
              aArgs.session = optarg;
              break;
          case 'v':
              aArgs.verbose = true;
              break;
          case 'h':
              std::cerr << "Usage: " << argv[0] << " [--session|-s <session-id>] [--acmacs|-a <acmacs-uri: https://localhost:1168/api https://acmacs-web.antigenic-cartography.org/api> <command-json> ... " << std::endl;
              aArgs.commands.clear();
              return;
          default:
              break;
        }
    }
    argc -= optind;
    argv += optind;

    if (aArgs.session.empty())
        throw std::runtime_error{"Please pass session"};
    if (argc < 1)
        throw std::runtime_error{"command(s) expected in the command line"};

    std::copy(argv, argv + argc, std::back_inserter(aArgs.commands));

} // parse_command_line

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
