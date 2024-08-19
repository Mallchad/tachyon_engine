
#include "memory.hpp"

void
FUNCTION raw_copy( void* dest, const void* src, u64 bytes)
{
    memcpy( dest, src, bytes );
}

void*
FUNCTION raw_allocate( u64 bytes )
{
    static fbyte* stub = ptr_cast<fbyte*>( malloc( 1'073'741'824 ) );
    static u64 stub_bump = 0;
    void* result = malloc( bytes );
    if (result == nullptr)
    {
        result = stub + stub_bump;
        stub_bump += bytes;
    }
    // Zero and prefault
    memset( result, 0, bytes );
    return result;
}
