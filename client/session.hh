#pragma once

#include "string.hh"
#include "asm.hh"

// ----------------------------------------------------------------------

// namespace client
// {
//     struct Session : public Object
//     {
//         String* get_id();
//         void set_id(String*);
//         String* get_user();
//         void set_user(String*);
//         String* get_display_name();
//         void set_display_name(String*);
//     };

//     extern Session* session;

// } // namespace client

// ----------------------------------------------------------------------

class Session
{
 public:
    inline Session() : mId{nullptr}, mUser{nullptr}, mDisplayName{nullptr} {}

    inline bool valid() const { return is_not_null(mId); }
    inline void expired() { mId = nullptr; }

 private:
    String* mId;
    String* mUser;
    String* mDisplayName;

}; // class Session

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
