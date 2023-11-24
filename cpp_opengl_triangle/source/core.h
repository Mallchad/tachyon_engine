#pragma once

#include "code_helpers.h"
#include "global.h"

#include <string>

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

template<id_type t_kind = id_type::untyped>
class internal_id
{
    fint32 m_value = -1;
public:
    CONSTRUCTOR internal_id() = default;

    explicit COPY_CONSTRUCTOR
    internal_id( internal_id<t_kind>& other ) : m_value(other) {}
    CONSTRUCTOR
    internal_id( fint32 value ) : m_value(value) {}
    operator int() const { return m_value; }
    fuint32
    FUNCTION fint32() const { return m_value; }

};

/// Object for handling locating ID's to references of objects
// Default ID class, strongly reccomend against using it and creating your
// own ttype with enums
typedef internal_id<> fid;

/// std::byte was initially tried for buffer objects but it has the nasty side
/// effect of introducing lots of reinterpret_casts everywhere which is really
/// nasty. An alternative would be a thin templated wrapper type with an
/// explicit cast which auto-disallows arithmatic operations
typedef uint8_t fbyte;
typedef std::vector<fbyte> byte_buffer;
