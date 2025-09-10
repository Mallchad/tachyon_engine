
#include "include_core.h"

fpath
FUNCTION linux_search_file( fpath target, std::vector<fpath> search_paths )
{
    namespace fs = std::filesystem;
// Linux only call
    // Always use the executable parent directory as search reference point
    fpath self_directory = fs::canonical( "/proc/self/exe" );
    self_directory = self_directory.parent_path();

    int matches = 0;
    fpath out_path;
    fpath check_path;
    for (fpath x_path : search_paths)
    {
        check_path = x_path / target;
        std::cout << "[File] Searching for file '" << check_path << "' \n";
        if (fs::exists( check_path ))
        {
            matches = 1;
            out_path = check_path;
        }
    }
    if (matches == 0)
    {
        std::cout << "[File] No viable match found for file search " << target << "\n";
        return "";
    }
    if (matches > 1)
    {
        std::cout << "[File] Multiple candidate matches for file search " << target
                  << " . Unsure how to proceed with operation \n";
        return "";
    }
    std::cout << "[File] Candidate file found in search paths " << out_path << "\n";
    return out_path;
}

byte_buffer
FUNCTION compat_file_load_binary( const fpath target )
{
    using namespace std::filesystem;
    byte_buffer out;
    FILE* tmp = nullptr;
    fuint64 tmp_filesize = 0;

    tmp = fopen( target.c_str(), "r" );
    if (tmp == nullptr)
    {
        std::cout << "[File] Failed to open file: " << target << "\n";
        std::cout << "[File] Failed to open file: " << globals::get_primary()->project_root << "\n";
        return out;
    }
    fseek( tmp, 0, SEEK_END );
    tmp_filesize = ftell( tmp );
    if (tmp_filesize <= 0)
    {
        std::cout << "[File] WARNING, opened file is zero length \n";
        fclose( tmp );
        return out;
    }

    // Return to beginning
    fseek( tmp, 0, SEEK_SET );

    out.resize( tmp_filesize );
    fread( out.data(), sizeof(fbyte), out.size(), tmp );
    fclose( tmp );
    std::cout << "[File] Internalized file at path: " << target << "\n";

    return out;
}

fmesh
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
    fmesh out;
    std::vector<v3>& vertex_buffer = out.vertex_buffer;
    fuint32 triangle_count;
    fbyte* triangle_count_ptr = nullptr;
    constexpr fint32 triangle_count_byte = 80;
    constexpr fint32 first_normal_byte = 84;
    constexpr fint32 first_vertex_byte = 96;
    constexpr fint32 triangle_stride = (sizeof(f32) * 12) + sizeof(fuint16);
    constexpr fint32 triangle_stride_normal = 50;

    v3 normal;
    v3 vertex_1;
    v3 vertex_2;
    v3 vertex_3;
    v3 vertex_1c;
    v3 vertex_2c;
    v3 vertex_3c;
    v3 normal_calculated;
    f32 winding_alignment = 0.0f;
    bool flipped_winding = false;
    bool flipped_message_sent = false;


    file = compat_file_load_binary( target );
    bool file_read_fail = file.size() <= 0;
    if ( file_read_fail ) { return out; }

    // Get the face count first
    fbyte* first_triangle_write = nullptr;
    triangle_count_ptr = triangle_count_byte + file.data();

    std::memcpy( &triangle_count, triangle_count_ptr, sizeof(fuint32));
    log( "File","Loading mesh with triangle count:", triangle_count );

    // Do quick setup with the face count
    out.face_count = triangle_count;
    out.vertex_count = triangle_count * 3; // Sized for 3 vertecies per face, 3 normals per face
    vertex_buffer.resize( (triangle_count * 6) );
    first_triangle_write = (triangle_count * sizeof(v3)) +
        reinterpret_cast<fbyte*>( vertex_buffer.data() );

    fbyte* x_readhead = nullptr;
    fbyte* x_writehead = nullptr;
    while (1)
    {
        for (fuint32 i_triangle=0; i_triangle < triangle_count; ++i_triangle)
        {
            // Copy normal
            x_readhead = (i_triangle * triangle_stride_normal) + first_normal_byte +
            ptr_cast<fbyte*>( file.data() );
            x_writehead = (i_triangle * 72) +
            ptr_cast<fbyte*>( vertex_buffer.data() );
            // Give every vertex a copy of the same normal
            std::memcpy(  0+ x_writehead, x_readhead, sizeof(v3) );
            std::memcpy( 24+ x_writehead, x_readhead, sizeof(v3) );
            std::memcpy( 48+ x_writehead , x_readhead, sizeof(v3) );

            // Copy Vertex
            std::memcpy( 12+ x_writehead, 12+ x_readhead, 12 );
            std::memcpy( 36+ x_writehead, 24+ x_readhead, 12 );
            std::memcpy( 60+ x_writehead, 36+ x_readhead, 12 );

            std::memcpy( &normal,    0+ x_writehead, 12 );
            std::memcpy( &vertex_1, 12+ x_writehead, 12 );
            std::memcpy( &vertex_2, 36+ x_writehead, 12 );
            std::memcpy( &vertex_3, 60+ x_writehead, 12 );

            winding_alignment = 0;
            normal_calculated = normalize( cross( vertex_2-vertex_1, vertex_3-vertex_1 ) );
            winding_alignment = dot( normal, normal_calculated );

            // OpenGL considers counter-clockwise wound triangles from the
            // perspective of the (invisible)interior of the object. Thus,
            // normals aligned with the given pre-computed normal shall be
            // flipped. A greater than zero alignment/dot product is considered
            // closer to being aligned
            if ( winding_alignment > 0 )
            {
                flipped_winding = true;
                std::memcpy( 12+ x_writehead, 12+ x_readhead, 12 );
                std::memcpy( 60+ x_writehead, 24+ x_readhead, 12 );
                std::memcpy( 36+ x_writehead, 36+ x_readhead, 12 );
                // Error Checking
                std::memcpy( &vertex_1c, 12+ x_writehead, 12 );
                std::memcpy( &vertex_2c, 36+ x_writehead, 12 );
                std::memcpy( &vertex_3c, 60+ x_writehead, 12 );

                winding_alignment = 0;
                normal_calculated = normalize( cross( vertex_2c-vertex_1c, vertex_3c-vertex_1c ) );
                winding_alignment = dot( normal, normal_calculated );

                if (winding_alignment > 0)
                { std::cout << "[File] Something went wrong! Normal is still flipped after reversed winding order!\n"; }
            }
            if (flipped_message_sent == false && flipped_winding)
            {
                flipped_message_sent = true;
                std::cout << "[File] STL file appears to have a winding out of phase with the normal, attempting to inverse winding order \n";
            }
        }
        break;
    }

    return out;
}

std::vector<v3>
FUNCTION read_stl_file( fpath target, stl_format format )
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

    ERROR_GUARD( (format != stl_format::fullspec), "[File] Fullspec STL format is not impliemnted" );
    ERROR_GUARD( (format != stl_format::vertex_and_normal),
        "[File] Wrong overload for vertex_and_normal_" );

    byte_buffer file;
    std::vector<v3> out;
    fuint32 triangle_count;
    fbyte* triangle_count_ptr = nullptr;
    constexpr fint32 triangle_count_byte = 80;
    constexpr fint32 first_normal_byte = 84;
    constexpr fint32 first_vertex_byte = 96;
    constexpr fint32 triangle_stride = (sizeof(f32) * 12) + sizeof(fuint16);
    constexpr fint32 triangle_stride_normal = 50;


    file = compat_file_load_binary( target );
    bool file_read_fail = file.size() <= 0;
    if ( file_read_fail ) { return out; }

    if (format == stl_format::vertex_only)
    {
        triangle_count = *reinterpret_cast<fint32*>( triangle_count_byte + file.data() );

        out.resize( triangle_count * 3 );

        v3* x_memory = nullptr;
        for (fuint32 i_triangle=0; i_triangle < triangle_count; ++i_triangle)
        {
            if ( (2 + i_triangle * 3) > out.size() )
            {
                std::cout << "[File] ERRROR buffer overflow whilst reading file \n";
                out.resize(0);\
                return out;
            }
            // 96 is an offset to the first vertex data
            x_memory = reinterpret_cast<v3*>( first_vertex_byte + file.data() +
                                                   (i_triangle * triangle_stride) );
            out[ 0+ i_triangle *3 ] = *(0+ x_memory);
            out[ 1+ i_triangle *3 ] = *(1+ x_memory);
            out[ 2+ i_triangle *3 ] = *(2+ x_memory);

        }
    }
    else { std::cout << "[File] Invalid STL format parameter\n"; }

    return out;
}

fstring
FUNCTION linux_load_text_file( fpath target, std::vector<fpath> search_paths )
{
    fpath target_path;
    byte_buffer loaded_file;
    fstring out;

    target_path = linux_search_file( target, search_paths );
    if (target_path.empty()) return "";

    loaded_file = compat_file_load_binary( target_path );
    out.resize( loaded_file.size() );
    std::memcpy( out.data(), loaded_file.data(), loaded_file.size() );

    return out;
}
