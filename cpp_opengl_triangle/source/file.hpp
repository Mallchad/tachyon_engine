
#include <code_helpers.h>
#include <cstdio>
#include <iostream>
#include <utility>

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

fhowdit
test_little_endian()
{
    fuint16 full_bits = 1;
    fuint8 first_bits = *reinterpret_cast<fuint8*>( &full_bits );
    bool little_endian = static_cast<bool>( first_bits );

    std::cout << (little_endian ? "Platform tested for endianess, came back as little endian" :
                  "Platform tested for endianess, came back as big endian")<< "\n";
    return little_endian;
}
