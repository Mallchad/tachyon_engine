#pragma once

// Passthrough Includes
#include "code_helpers.h"
#include "time.hpp"

#include <compare>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <type_traits>

#include <csignal>

// Project Specific Includes
// Silence warnings in external projects
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

// For faster compile times
#define SPDLOG_COMPILED_LIB 1
#include <spdlog/spdlog.h>
#include <spdlog/fwd.h>
using namespace std::string_literals;

#pragma clang diagnostic pop
// End Passthrough Includes

enum class id_type
{
    untyped,
    display,
    window,
    graphics_context,
    vertex_attribute,
    buffer,
    uniform,
    mesh,
    shader,
    shader_program
};

#ifndef CLING_INTERPRETER_H
    #define print( message ) log( message, __FILE__ )
#endif
template<typename... t_streamable>
void
FUNCTION log( const char* category, t_streamable... messages )
{
    std::cout << "[" << category << "]" << std::setfill(' ') << std::setw(6) << " ";
    ((std::cout << messages  << " "), ...) << "\n";
}

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

#if __cplusplus >= 202002L // C++20 (and later) code
    std::strong_ordering
    FUNCTION operator <=> ( internal_id<t_kind> rhs ) const
    { return this->m_value <=> rhs.m_value; }
    std::strong_ordering
    FUNCTION operator <=> ( int rhs ) { return m_value <=> rhs; }
#else
    bool
    FUNCTION operator < ( internal_id<t_kind> rhs ) const
    { return this->m_value < rhs.m_value; }
    bool
    FUNCTION operator <= ( internal_id<t_kind> rhs ) const
    { return this->m_value <= rhs.m_value; }
    bool
    FUNCTION operator > ( internal_id<t_kind> rhs ) const
    { return this->m_value <= rhs.m_value; }
    bool
    FUNCTION operator >= ( internal_id<t_kind> rhs ) const
    { return this->m_value >= rhs.m_value; }
    bool
    FUNCTION operator == ( internal_id<t_kind> rhs ) const
    { return this->m_value == rhs.m_value; }
    bool
    FUNCTION operator != ( internal_id<t_kind> rhs ) const
    { return this->m_value != rhs.m_value; }
#endif
    
    explicit operator
    CONVERSION fint32() const { return m_value; }
    explicit operator
    CONVERSION fuint32() const { return m_value; }

    template<typename t_integral = fint32> t_integral
    FUNCTION cast() const { return static_cast<t_integral>( m_value ); }
};

/// Relatively well defined cast behaviour
// This will primarily do a static cast, but will allow reinterpret cast ONLY
// if it is a pointer to pointer conversion which is relatively sensible.
// Conversions to and from pointers to other values are compeltely nonsensical
template<typename t_return, typename t_target>
constexpr t_return
FUNCTION cast( t_target target )
{ return static_cast<t_return>( target ); }

/// Reinterpret cast that only can return pointer types
// This is deemed to be slightly safer than being able to reinterpret any type
template<typename t_return, typename t_target>
constexpr t_return
FUNCTION ptr_cast( t_target target )
{
    constexpr bool pointer_return = (std::is_pointer<t_return>::value);
    constexpr bool pointer_target = (std::is_pointer<t_target>::value);
    constexpr bool integral_target = (std::is_integral<t_target>::value);

    static_assert( pointer_return , "Return type must be of a pointer type" );
    static_assert( integral_target || pointer_target,
                   "Casting from non-ingetral types to pointers is unsafe" );
    return reinterpret_cast<t_return>( target );
}

/// Reinterpret cast alias
// Very prone to invoking Undefined Behaviour, avoid accessing objects through
// reinterpreted pointers. If looking for a way to cast between pointers use
// ptr_cast
template<typename t_return, typename t_target>
constexpr t_return
FUNCTION ub_cast( t_target target )
{
    return reinterpret_cast<t_return>( target );
}

template<typename t_result, typename t_target>
constexpr auto
binary_cast( t_target& target )
{
    t_result result;
    static_assert( sizeof(t_result) == sizeof(t_target), "Target must have same size as result" );
    std::memcpy( &target, &result, sizeof( target ) );
    return result;
}

// -- Typedefs --
using fstring = std::basic_string<char>;
using fstring_view = std::basic_string_view<char>;
// using cstring = char*;
using fpath = std::filesystem::path;

/// std::byte was initially tried for buffer objects but it has the nasty side
/// effect of introducing lots of reinterpret_casts everywhere which is really
/// nasty and risky. An alternative would be a thin templated wrapper type with
/// an explicit cast which auto-disallows arithmatic operations
using byte_buffer = std::vector<fbyte>;
// An untyped byte buffer
using buffer = std::vector<fbyte>;

/// Object for handling locating ID's to references of objects
// Default ID class is untyped, strongly reccomend against using it and creating your
// own ttype with enums
using fid = internal_id<>;

// Project Specific
