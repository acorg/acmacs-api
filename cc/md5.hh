#pragma once

#include <sstream>
#include <iomanip>
#include <string>

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wreserved-id-macro"
#endif

#include <openssl/md5.h>

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

inline std::string md5(std::string aSource)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char *>(aSource.c_str()), aSource.size(), digest);
    std::ostringstream os;
    os << std::hex << std::setfill('0') << std::nouppercase;
    for(auto c: digest)
        os << std::setw(2) << static_cast<long long>(c);
      // std::cerr << "md5 " << aSource << " --> " << os.str() << std::endl;
    return os.str();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
