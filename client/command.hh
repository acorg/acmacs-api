#pragma once

#include <cheerp/clientlib.h>

// ----------------------------------------------------------------------

namespace client
{
    struct CommandData : public Object
    {
        inline CommandData(String* aCmd) { set_C(aCmd) ; }
        void set_C(String*);
    };

      // --------------------------------------------------

    struct ResponseData : public Object
    {
        String* get_R();
        String* get_E();
    };

} // namespace client

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
