#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <thread>

// ----------------------------------------------------------------------

namespace print_internal
{
    inline void print_impl(const char* file, int line, std::string m1, std::string m2 = std::string{}, std::string m3 = std::string{})
    {
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif
        static std::mutex access;
#pragma GCC diagnostic pop
        std::unique_lock<decltype(access)> lock{access};
        std::cout << std::this_thread::get_id() << ' ' << file << ':' << line << ": " << m1 << m2 << m3 << std::endl;
    }
}

#define print1(m1) print_internal::print_impl(__FILE__, __LINE__, (m1))
#define print2(m1, m2) print_internal::print_impl(__FILE__, __LINE__, (m1), (m2))
#define print3(m1, m2, m3) print_internal::print_impl(__FILE__, __LINE__, (m1), (m2), (m3))

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
