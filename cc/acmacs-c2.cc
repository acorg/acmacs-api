#include <iostream>
#include <vector>
#include <string>
#include <getopt.h>
#include <curl/curl.h>

// #pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#endif
// #pragma GCC diagnostic pop

// ----------------------------------------------------------------------

struct Args
{
    std::string session;
    std::vector<std::string> commands;
};

static void parse_command_line(int argc, char* const argv[], Args& aArgs);
static size_t response(char* contents, size_t size, size_t nmemb, void* userp);

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    const char* postthis = R"({"C":"version"})";
    int exit_code = 0;
    try {
        Args args;
        parse_command_line(argc, argv, args);

          // https://curl.haxx.se/libcurl/c/postinmemory.html
        if (CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT); res != CURLE_OK)
            throw std::runtime_error(std::string{"curl_global_init failed: "} + curl_easy_strerror(res));
        if (CURL* curl = curl_easy_init(); curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://acmacs-web.antigenic-cartography.org/api"); // https://localhost:1168/api https://acmacs-web.antigenic-cartography.org/api
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &response);
              // curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(strlen(postthis))); // optional, curl can do strlen itself
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

            // struct curl_slist *chunk = curl_slist_append(nullptr, "Transfer-Encoding: chunked");
            // if (CURLcode res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk); res != CURLE_OK)
            //     throw std::runtime_error(std::string{"curl_easy_setopt failed: "} + curl_easy_strerror(res));

            if (CURLcode res = curl_easy_perform(curl); res != CURLE_OK)
                throw std::runtime_error(std::string{"curl_easy_perform failed: "} + curl_easy_strerror(res));
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << std::endl;
        exit_code = 1;
    }
    return exit_code;

} // main

// ----------------------------------------------------------------------

size_t response(char* contents, size_t size, size_t nmemb, void* /*userp*/)
{
    const auto real_size = size * nmemb;
    std::cout << "Response " << real_size << " [" << std::string{contents, real_size} << ']' << std::endl;
    return real_size;

} // response

// ----------------------------------------------------------------------

void parse_command_line(int argc, char* const argv[], Args& aArgs)
{
    const char* const short_opts = "s:h";
    const option long_opts[] = {
        {"session", required_argument, nullptr, 's'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, no_argument, nullptr, 0}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
        switch (opt) {
          case 's':
              aArgs.session = optarg;
              break;
          case 'h':
              std::cerr << "Usage: " << argv[0] << " [--session|-s <session-id>] <command-json> ... " << std::endl;
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
