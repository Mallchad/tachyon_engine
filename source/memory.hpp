
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

template <typename t_any>
struct ptr
{
    using t_self = ptr<t_any>;
    t_any* data = 0x0;
    /** This denotes the arena the memory belongs to. If it belongs to an arena
    the arena can arbitrarily relocate the data pointer, so don't copy it to a
    raw pointer. Pointers managed this was are trivially serializable 0x0 arena means system memory */
    t_any* arena = 0x0;
    t_any** shared_data = &data;
    bool weakly_shared = false;
    t_any& FORCEINLINE
    operator * () { return (weakly_shared ? **shared_data : *data); }

    template <typename t_other>
    t_self& FORCEINLINE
    operator = (ptr<t_other> rhs)
    {
        this->data = ptr_cast<t_any*>( rhs.data );
        this->shared_data = ptr_cast<t_any**>( &rhs.data );
        this->arena = ptr_cast<t_any*>( arena );
        this->weakly_shared = true;
        return *this;
    }
    t_self& FORCEINLINE
    operator = (t_any* rhs)
    {
        this->data = rhs;
        this->shared_data = 0x0;
        this->arena = 0x0;
        this->weakly_shared = false;
        return *this;
    }
};
