#pragma once

#include <iostream>
#include <string>

#include "acmacs-base/to-json.hh"

// ----------------------------------------------------------------------

class SessionId
{
 public:
    inline SessionId() = default;
    inline SessionId(std::string aId) : mId{aId} {}

    inline SessionId& operator = (std::string aId) { mId = aId; return *this; }
    inline operator bool () const { return !mId.empty(); }
    inline void reset() { mId.clear(); }
    inline std::string id() const { return mId; }

 private:
    std::string mId;

}; // class SessionId

inline std::ostream& operator << (std::ostream& out, const SessionId& aId)
{
    out << aId.id();
    return out;
}

inline std::string operator + (std::string left, const SessionId& aId)
{
    return left + aId.id();
}

namespace to_json
{
    template <> inline std::string value(SessionId&& aValue)
    {
        return value(aValue.id());
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
