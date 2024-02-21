#pragma once

// Passthrough Includes
#include "code_helpers.h"

#include <compare>
#include <string>
#include <string_view>
#include <vector>

// Silence warnings in external projects
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

// For faster compile times
#define SPDLOG_COMPILED_LIB 1
#include <spdlog/spdlog.h>
#include <spdlog/fwd.h>
using namespace std::string_literals;

#pragma pop
// End Passthrough Includes

enum class id_type
{
    untyped,
    display,
    window,
    graphics_context,
    vertex_attribute,
    buffer,
    mesh,
    shader,
    shader_program
};

/// Object for handling locating ID's to references of objects
// Default ID class, strongly reccomend against using it and creating your
// own ttype with enums
template<id_type t_kind = id_type::untyped>
class internal_id
{
    fint32 m_value = -1;
public:
    CONSTRUCTOR internal_id() = default;

    CONSTRUCTOR
    internal_id( fint32 value ) : m_value(value) {}

    template<id_type = t_kind>
    std::strong_ordering
    FUNCTION operator <=> ( internal_id<t_kind> rhs ) const
    { return this->m_value <=> rhs.m_value; }
    std::strong_ordering
    FUNCTION operator <=> ( int rhs ) { return m_value <=> rhs; }
    
    explicit operator
    CONVERSION fint32() const { return m_value; }
    explicit operator
    CONVERSION fuint32() const { return m_value; }

    template<typename t_integral = fint32> t_integral
    FUNCTION cast() const { return static_cast<t_integral>( m_value ); }
};

/// Static cast alias
template<typename t_return> t_return
FUNCTION cast( auto target )
{
    return static_cast<t_return>( target );
}

// -- Typedefs --
using fstring = std::basic_string<char>;
using fstring_view = std::basic_string_view<char>;
using cstring = const char*;
using fpath = const char*;

/// std::byte was initially tried for buffer objects but it has the nasty side
/// effect of introducing lots of reinterpret_casts everywhere which is really
/// nasty and risky. An alternative would be a thin templated wrapper type with
/// an explicitf cast which auto-disallows arithmatic operations
using fbyte = uint8_t;
using byte_buffer = std::vector<fbyte>;

/// Object for handling locating ID's to references of objects
// Default ID class is untyped, strongly reccomend against using it and creating your
// own ttype with enums
using fid = internal_id<>;
