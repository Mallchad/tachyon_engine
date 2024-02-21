#!/bin/env lua

require("lib.shim_lib")

gmodule_name = "Build Script"
log_verbose = false

local build_universal_arguments = "-std=c++20 -fuse-ld=mold -I../tracy -lX11 -lGL *.cpp -ldl -Iexternal/include -Wpedantic -Wall -Wno-unused-value "

local build_command = "clang++ -g -O0 " ..build_universal_arguments
local build_command_shipping_test = "clang++ -g -O2 -march=x86-64 -Weverything -Wno-c++98-compat -Wno-implicit-int-float-conversion -Wno-sign-conversion " ..build_universal_arguments
local tracy_arguments = "build/TracyClient.o -DTRACY_ENABLE -lpthread -ldl "  -- Tracy

local build_debug_extras = ""

local build_extras = ""
if arg[1] == "tracy" or arg[2] == "tracy" then
   log( "Enabled tracy profiler, linking client to build" )
   build_extras = tracy_arguments .. build_extras
end

local success, exit, code = 1, 1, 1
local build_command_string = build_command .. build_extras

local tracy_obj_exists = file_exists("build/TracyClient.o")
if (tracy_obj_exists ~= true) then
   log( "No TracyClient.o obj file, building the unit" )
   safe_execute(" clang++ -c -g -O2 -DTRACY_ENABLE -std=c++20 -march=x86-64 ../tracy/TracyClient.cpp -o build/TracyClient.o" )
else
   log( "TracyClient.o obj file exists, continuing to build" )
end

if (arg[1] == "debug" or arg[2] == "debug") then

   build_command_string = build_command_string .. build_extras .. build_debug_extras
   log( "Building a debug build (does nothing of note)" )

elseif (arg[1] == "shipping_test" or arg[2] == "shipping_test") then

   build_command_string = build_command_shipping_test .. build_extras
   log( "Building optimized shipping build, checking all warnings" )
else
   log( "Argument for build config empty or not supported, can be [debug] or [shipping_test]" )
   os.exit(1)
end

log( "Running command: \n ".. build_command_string )
log("-------------------------------------------------------------------------------------\n")
success, exit, code = safe_execute( build_command_shipping_test .. build_extras )

os.exit(code)
