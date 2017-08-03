#pragma once

#include <cheerp/clientlib.h>

// ----------------------------------------------------------------------

namespace client
{
    struct CommandData : public Object
    {
        inline CommandData(String* aCmd) { set_C(aCmd) ; }

        void set_C(String*);
        void set_D(String*);    // command id
        String* get_C();
        String* get_D();
    };

      // --------------------------------------------------

    struct RawMessage : public Object
    {
        String* get_hello();
        String* get_C();
        String* get_D();
        String* get_E();

    }; // struct RawMessage

      // --------------------------------------------------

    struct ResponseData : public RawMessage
    {
        String* get_CN();       // number of commands processed
        String* get_CT();       // time of the command processing
        String* get_CI();       // index of the command part (if there are multiple responses for the command)
    };

} // namespace client

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
