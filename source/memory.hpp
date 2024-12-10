
#pragma once

#include <memory>

#include "core.hpp"
#include "error.hpp"

// Removes an object from scope, essentially calling it's destructor if sensible
template <typename t_movable>
constexpr void
FUNCTION drop( t_movable doomed )
{
    auto _ = std::move( doomed );
}

template <typename t_ptr>
bool
raw_zero( t_ptr target, i64 count)
{
    using t_any = decltype(*target);
    std::memset( target, 0x0, count * sizeof(t_any) );
    return true;
}

void
FUNCTION raw_copy( void* dest, const void* src, u64 bytes);

template <typename t_src>
void
FUNCTION typed_copy( void* dest, const t_src* src, u64 count)
{
    using t_target = decltype( *src );
    memcpy( dest, src, count* sizeof(t_target) );
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
FUNCTION allocate( u64 count )
{
    static fbyte* stub = ptr_cast<fbyte*>( malloc( 1'073'741'824 ) );
    static u64 stub_bump = 0;

    u64 allocation = (sizeof(t_sizing) * count);
    t_sizing* result = ptr_cast<t_sizing*>( malloc( allocation ));
    if (result == nullptr)
    {
        result = ptr_cast<t_sizing*>( stub ) + stub_bump;
        stub_bump += allocation;
    }
    // Zero and prefault to get real memory
    memset( result, 0, allocation );
    return result;
}

template <typename t_any>
void
swap( t_any& lhs, t_any& rhs )
{
    t_any tmp = lhs;
    lhs = rhs;
    rhs = tmp;
}

/* Default allocation in raw bytes
 * Tries to gurantee physical allocation of memory. Allocates based on the return type */
void*
FUNCTION raw_allocate( u64 bytes );

template <typename t_any>
struct ptr final
{
    using t_self = ptr<t_any>;
    t_any* data = 0x0;
    /** This denotes the arena the memory belongs to. If it belongs to an arena
        the arena can arbitrarily relocate the data pointer, so don't copy it to
        a raw pointer. Pointers managed this was are trivially serializable 0x0
        arena means system memory */
    t_any* arena = 0x0;
    t_self* next = nullptr;
    /** This is the primary manager for the underlying object and will propagate
        changes correctly where required */
    bool manager = false;
    /** When true this will call the destructor of the managed resource when done */
    bool scoped_resource = false;
    /** If another pointer is copied to the current pointer it is considered
        weakly shared, and is at the mercy any reallocaitons and destruction the
        other pointer. If the pointer changed it will be reflected in weakly
        shared pointers */
    bool weakly_shared = false;
    /** If a raw pointer absolutely needs access to a smart pointer, like
        through sysaclls, it will have to be bound in-place, so it shouldn't realloc or
        be destroyed for as long as possible, such pointers are marked as borrowed */
    bool borrowed = false;

    t_any& FORCEINLINE
    operator *() { return (*data); }
    t_any& FORCEINLINE
    operator [](u64 i) { return data[i]; }

    template <typename t_other>
    t_self& FORCEINLINE
    operator =( ptr<t_other>& rhs )
    {
        this->data = ptr_cast<t_any*>( rhs.data );
        this->arena = ptr_cast<t_any*>( arena );
        this->weakly_shared = true;
        int share_cap = 100;

        this->next = rhs.next;
        rhs.next = this;
        propagate_data();
        return *this;
    }

    t_self& FORCEINLINE
    operator =( t_any* rhs )
    {
        this->data = rhs;
        this->next = nullptr;
        this->arena = 0x0;
        this->weakly_shared = false;

        propagate_data();
        return *this;
    }

    /** Destroys scoped resources if necessary
        If the pointer needs to be invalidated for any reason this should be
        done manually before the smart pointer goes out of scope. With unscoped
        resources weak pointers remain valid until the external memory manager
        cleans it up */
    DESTRUCTOR ~ptr()
    {
        if (scoped_resource) { delete data; data = nullptr; propagate_data(); }
    }

    void
    FUNCTION propagate_data()
    {
        if (manager == false) { return; }
        t_self* x_ptr = next;

        int share_cap = 100;
        for (int i=0; i<share_cap; ++i)
        {
            if (x_ptr == nullptr) { break; }
            x_ptr = x_ptr->next;
            x_ptr->data = this->data;
            x_ptr->arena = arena;
        }
    }
};
