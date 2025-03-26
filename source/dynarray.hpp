
#pragma once

#include <core.hpp>
#include <memory.hpp>

namespace tyon
{
    template <typename T>
    struct dynarray
    {
        ptr<T> data = nullptr;
        u64 size = 0;
        u64 head = 0;
        u64 head_size = 0;
        bool fast_bounded = true;

        /// Pushes an new before the 'head' index and decrements the 'head'
        bool
        FUNCTION push_head( T item )
        {
            if (fast_bounded && (head <= 0)) { return false; }
            data[ --head ] = item;
            ++head_size;
            return true;
        }

        /// Pushes a new item at off the tail (head + head_size) and increments 'head_size'
        bool
        FUNCTION push_tail( T item )
        {
            if (fast_bounded && (head + head_size >= size)) { return false; }
            data[ head + (++head_size) ] = item;
            ++head_size;
            return true;
        }

        T
        FUNCTION pop_head()
        {
            if (fast_bounded && (head_size = 0)) { return false; }
            T result = data[ head++ ];
            --head_size;
            return result;
        }

        T
        FUNCTION pop_tail( T item )
        {
            if (fast_bounded && (head_size = 0)) { return false; }
            T result = data[ head + head_size - 1 ];
            --head_size;
            return result;
        }

        T&
        FUNCTION get_tail()
        { return data[ head+head_size -1 ]; }

        T*
        get_tail_address()
        { return &(get_tail()); }

        /// Set memory of array to read only and employ data integrity checks
        PROCEDURE protect()
        {
            // Crash
            *(char*)0 = 1;
        }

    };
}
