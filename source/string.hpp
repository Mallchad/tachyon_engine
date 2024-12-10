
#pragma once

#include "code_helpers.h"

namespace tyon
{
    struct string
    {
        char* data = nullptr;
        u32 size = 0;
        u32 reserve = 0;
        string* owner = nullptr;
        bool owning = false;
        bool null_terminated = true;
        bool immutable = false;
        bool slice = false;
        /// String will be permanantly flagged as tainted if copied / read from
        /// null terminated storage. Null terminated strings cannot be formally
        /// proved for validity or safety so any strings tainted by interaction
        /// with them should be treated as dangerous / unreliable
        bool null_tainted = false;

        CONSTRUCTOR string() = default;

        COPY_CONSTRUCTOR string( const char* target );

        /* Measure the size of a cstring null terminator exclusive
         * NOTE: This is error prone and will not behave correctly if it a null
         * terminator is misisng.
         * The runtime of this function is undefined and could go on forever */
        u32
        FUNCTION compile_cstring_size( cstring target );
    };
}
