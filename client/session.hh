#pragma once

#include "string.hh"
#include "asm.hh"

// ----------------------------------------------------------------------

class Session
{
 public:
    inline Session() : mId{nullptr}, mUser{nullptr}, mDisplayName{nullptr} {}

    inline bool valid() const { return is_not_null(mId); }
    inline void expired() { mId = nullptr; }

    inline void id(String* aId) { mId = aId; }
    inline String* id() const { return mId; }
    inline void user(String* aUser) { mUser = aUser; }
    inline String* user() const { return mUser; }
    inline void display_name(String* aDisplayName) { mDisplayName = aDisplayName; }
    inline String* display_name() const { return mDisplayName; }

 private:
    String* mId;
    String* mUser;
    String* mDisplayName;

}; // class Session

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
