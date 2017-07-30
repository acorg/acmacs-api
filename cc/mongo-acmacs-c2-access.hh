#pragma once

#include "mongo-access.hh"

// ----------------------------------------------------------------------

class AcmacsC2;

class MongoAcmacsC2Access
{
 public:
    inline MongoAcmacsC2Access(std::string aMongoURI, AcmacsC2& aAcmacsC2)
        : mMongoURI{aMongoURI}, mAcmacsC2{aAcmacsC2} {}

    auto& client() { return mClient; }
    auto& acmacs_c2() { return mAcmacsC2; }

    inline void create_client()
        {
            mClient = mongocxx::client{mMongoURI.empty() ? mongocxx::uri{} : mongocxx::uri{mMongoURI}};
        }

 private:
    mongocxx::client mClient;
    std::string mMongoURI;
    AcmacsC2& mAcmacsC2;

}; // class MongoAcmacsC2Access

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
