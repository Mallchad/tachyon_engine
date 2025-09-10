#pragma once

/// Try to find a file by name by searching through a vector of provided directories
// This can be provided as an initializer list of strings { "/foo/bar" }
fpath
FUNCTION linux_search_file( fpath target, std::vector<fpath> search_paths );


/*** Read a file into the internal storage of the program
 This effectively reads a file and then returns a byte buffer repsenting the read file
 in a binary format. No attempt is made at formatting it.

 Returns a zero length byte_buffer if it failed
*/
byte_buffer
FUNCTION compat_file_load_binary( const fpath target );

/// Deprecreated. Use the new function load_file_binary
byte_buffer
FUNCTION intern_file( fpath target );

/// vertex_only - Basic normal_less layout
// vertex_and_normal - Include a full set of triangle vertecies and its assocaited normal
// fullspec - UNIMPLIMENTED triangles, normals, and random variable length embeddable
//  attribute data
enum class stl_format { vertex_only, vertex_and_normal, fullspec };

/// Returns a buffer of vertecies stored in an STL file
// Vertex and normal version (default overload)
fmesh
FUNCTION read_stl_file( fpath target );

std::vector<v3>
FUNCTION read_stl_file( fpath target, stl_format format );

fstring
FUNCTION linux_load_text_file( fpath target, std::vector<fpath> search_paths );
