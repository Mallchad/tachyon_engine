#!/bin/env lua

local build_universal_arguments = " -std=c++20 -fuse-ld=mold -I../tracy -lX11 -lGL *.cpp -ldl -Iexternal/include -Wpedantic -Wall -Wno-reserved-id-macro"

local build_command = "clang++ -g -O0 " ..build_universal_arguments
local build_command_shipping_test = "clang++ -g -O2 -Weverything -Wno-c++98-compat -Wno-implicit-int-float-conversion -Wno-sign-conversion " ..build_universal_arguments
local tracy_arguments = "../tracy/TracyClient.cpp -DTRACY_ENABLE -lpthread -ldl "  -- Tracy

local build_debug_extras = ""

local build_extras = ""
if arg[1] == "tracy" or arg[2] == "tracy" then
   print( "Enabled tracy profiler, linking client to build" )
   build_extras = tracy_arguments .. build_extras
end

-- random things may need in future
-- -lGL -lGLX -lGLX_nvidia -lGLEW -lglfw

local success, exit, code = 1, 1, 1
local build_command_string = build_command .. build_extras

if arg[1] == "debug" or arg[1] == nil then
   build_command_string = build_command_string .. build_extras
   print( "Building a debug build (does nothing of note)" )

elseif arg[1] == "shipping_test" or arg[2] == "shipping_test" then
   build_command_string = build_command_shipping_test .. build_extras

   print( "Building optimized shipping build, checking all warnings" )
else
   print( "Argument not supported" )
end

print( "Running command: \n ".. build_command_string )
print("-------------------------------------------------------------------------------------\n")
success, exit, code = os.execute( build_command_shipping_test .. build_extras )

os.exit(code)
