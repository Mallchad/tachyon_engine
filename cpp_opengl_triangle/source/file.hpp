
#include <code_helpers.h>
#include <cstdio>
#include <iostream>
#include <utility>

#include "math.hpp"

byte_buffer
FUNCTION intern_file( fpath target )
{
    byte_buffer out;
    FILE* tmp = nullptr;
    fuint32 tmp_filesize = 0;
    tmp = fopen( target, "r" );
    if (tmp == nullptr)
    {
        std::cout << "Failed to open file: " << target << "\n";
        return out;
    }
    fseek( tmp, 0, SEEK_END );
    tmp_filesize = ftell( tmp );
    // Return to beginning
    fseek( tmp, 0, SEEK_SET );

    out.resize( tmp_filesize );
    fread( out.data(), sizeof(fbyte), out.size(), tmp );
    fclose( tmp );
    std::cout << "Internalized file at path: " << target << "\n";

    return out;
}

/// Tests the memory layout to see if it is little endian or big endian
// Returns true if little endian
fhowdit
FUNCTION test_little_endian()
{
    fuint16 full_bits = 1;
    fuint8 first_bits = *reinterpret_cast<fuint8*>( &full_bits );
    bool little_endian = static_cast<bool>( first_bits );

    std::cout << (little_endian ? "Platform tested for endianess, came back as little endian" :
                  "Platform tested for endianess, came back as big endian")<< "\n";
    return little_endian;
}

/// Returns a buffer of vertecies stored in an STL file
// NOTE In future may be interlaced with normals
std::vector<ffloat3>
FUNCTION read_stl_file( fpath target )
{
    /** STL Format
        Presumed little endian
        80 Bytes  - Header
        4 Bytes   - Number of triangles
            84th Byte - First triangle
        12 Bytes  - float3 Triangle Normal
            96th Byte - First Vertex
        12 Bytes  - float3 Vertex 1
        12 Bytes  - float3 Vertex 2
        12 Bytes  - float3 Vertex 3
        2 Bytes   - Unused attribute width
            50 Byte Stride from one Triangle to Next
    */

    byte_buffer file;
    std::vector<ffloat3> out;
    fuint32 triangle_count;
    constexpr fint32 triangle_count_byte = 80;
    constexpr fint32 first_vertex_byte = 96;
    constexpr fint32 triangle_stride = (sizeof(ffloat) * 12) + sizeof(fuint16);

    file = intern_file( target );
    triangle_count = *reinterpret_cast<fint32*>( triangle_count_byte + file.data() );

    out.resize( triangle_count * 3 );

    ffloat3* x_memory = 0;
    for (int i_triangle=0; i_triangle < triangle_count; ++i_triangle)
    {
        // 96 is an offset to the first vertex data
        x_memory = reinterpret_cast<ffloat3*>( first_vertex_byte + file.data() +
                                               (i_triangle * triangle_stride) );
        out[ 0+ i_triangle *3 ] = *(0+ x_memory);
        out[ 1+ i_triangle *3 ] = *(1+ x_memory);
        out[ 2+ i_triangle *3 ] = *(2+ x_memory);
    }

    return out;
}
