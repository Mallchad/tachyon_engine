
#pragma once

#include <type_traits>

#include "code_helpers.h"

/** To check for implimented functions place the INTERFACE_DEFINE_FUNCTION macro
 * at the bottom of the .h file. It will not work properly if it is put in a
 * .cpp file, although it will work it will just throw undefined references and
 * linker error which defeats the point of the macro
 *
 */

#define INTERFACE_DEFINE_FUNCTION(interface_function_name) \
    static_assert( std::is_member_function_pointer< \
                   decltype( &interface_function_name )>());

#define INTERFACE_IMPLEMENT_RENDERER(interface_class_name)  \
    \
    INTERFACE_DEFINE_FUNCTION(interface_class_name::create_context) \
    INTERFACE_DEFINE_FUNCTION(interface_class_name::draw_test_triangle) \
    INTERFACE_DEFINE_FUNCTION(interface_class_name::draw_test_circle)   \
    INTERFACE_DEFINE_FUNCTION(interface_class_name::draw_test_rectangle) \
    INTERFACE_DEFINE_FUNCTION(interface_class_name::draw_test_signfield) \
    INTERFACE_DEFINE_FUNCTION(interface_class_name::refresh) \
