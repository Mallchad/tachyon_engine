
#pragma once

/// Runtime assertion that raises an exception when false
// Aims to improve over the built-in assert by not adhereing to DEBUG/NDEBUG macros
// and being more transparently configurable.
void
FUNCTION assertion( bool expression, cstring message );

/// Tags a code location with unimplimented codepaths and errors out where appropriate
void
FUNCTION unimplimented( cstring reason );
