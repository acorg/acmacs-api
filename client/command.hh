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
        String* get_C();        // response for command
        String* get_CN();       // number of commands processed
        String* get_CT();       // time of the command processing
        String* get_CI();       // index of the command part (if there are multiple responses for the command)
        String* get_E();
    };

} // namespace client

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
