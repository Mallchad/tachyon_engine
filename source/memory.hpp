
#pragma once

#include <core.hpp>
#include <memory.h>

// Removes an object from scope, essentially calling it's destructor if sensible
template <typename t_movable>
constexpr void
FUNCTION drop( t_movable doomed )
{
    auto _ = std::move( doomed );
}

void
FUNCTION raw_copy( void* dest, const void* src, u64 bytes);

template <typename t_src>
void
FUNCTION typed_copy( void* dest, const t_src* src, u64 bytes)
{
    using t_target = decltype( *src );
    memcpy( dest, src, bytes* sizeof(t_target) );
}

template <typename t_dest>
void
FUNCTION typed_copy_dest( t_dest* dest, const void* src, u64 bytes)
{
    using t_target = decltype( *dest );
    memcpy( dest, src, bytes* sizeof(t_target) );
}

/* Default typed allocation
 * Tries to gurantee physical allocation of memory. Allocates based on the return type */
template <typename t_sizing>
t_sizing*
FUNCTION allocate( u64 bytes )
{
    static fbyte* stub = ptr_cast<fbyte*>( malloc( 1'073'741'824 ) );
    static u64 stub_bump = 0;

    u64 allocation = (sizeof(t_sizing) * bytes);
    t_sizing* result = ptr_cast<t_sizing*>( malloc( allocation ));
    if (result == nullptr)
    {
        result = ptr_cast<t_sizing*>( stub ) + stub_bump;
        stub_bump += allocation;
    }
    // Zero and prefault
    memset( result, 0, allocation );
    return result;
}

/* Default allocation in raw bytes
 * Tries to gurantee physical allocation of memory. Allocates based on the return type */
void*
FUNCTION raw_allocate( u64 bytes );
