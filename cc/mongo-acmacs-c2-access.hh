#pragma once

#include "acmacs-webserver/print.hh"
#include "mongo-access.hh"

// ----------------------------------------------------------------------

class AcmacsC2;

class MongoAcmacsC2Access
{
 public:
    MongoAcmacsC2Access(const std::string& aMongoURI, AcmacsC2& aAcmacsC2)
        : mMongoURI{aMongoURI}, mAcmacsC2{aAcmacsC2} {}

    auto& client()
    {
        if (!mClient)
            create_client();
        return mClient;
    }
    auto& acmacs_c2() { return mAcmacsC2; }

    void create_client()
    {
        try {
            mClient = mongocxx::client(mMongoURI.empty() ? mongocxx::uri{} : mongocxx::uri{mMongoURI});
            if (!mClient)
                print_cerr("MongoAcmacsC2Access::create_client FAILED, URI: ", mMongoURI);
        }
        catch (std::exception& err) {
            print_cerr("MongoAcmacsC2Access::create_client FAILED: ", err.what(), " URI: ", mMongoURI);
        }
    }

 private:
    mongocxx::client mClient;
    const std::string mMongoURI;
    AcmacsC2& mAcmacsC2;

}; // class MongoAcmacsC2Access

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
