
#pragma once

enum class extension_family
{
    no_family,
    EXT_swap_control
};

struct extension
{
    bool is_integer_id;
    int integer_id;
    fstring string_id;
    extension_family enum_family;
};

/// Bookkeeping mechanism for extensions in use
// This works for any extension type at all
class extension_database
{
private:
    std::vector<extension> known;
    std::vector<extension> enabled;

public:
    const fstring type;
    CONSTRUCTOR extension_database( fstring extension_type ) :
        type(extension_type) {}
    CONSTRUCTOR extension_database( const char* extension_type ) :
        type(extension_type) {}
    void
    FUNCTION enable( fstring added, extension_family = extension_family::no_family )
    {
        extension tmp = {
            .is_integer_id = false,
            .integer_id = -1,
            .string_id = added
        };
        enabled.push_back( tmp );
    }
};
