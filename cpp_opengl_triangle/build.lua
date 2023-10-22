#!/bin/env lua

build_universal_arguments = " -std=c++20 -I../tracy -lX11 -lGL *.cpp ../tracy/TracyClient.cpp -lpthread -ldl"

build_command = "clang++ -g -O" ..build_universal_arguments
build_command_shipping_test = "clang++ -g -O2 -march=native -Weverything -Wno-c++98-compat -Wno-implicit-int-float-conversion -Wno-sign-conversion" ..build_universal_arguments
tracy_arguments = " -DTRACY_ENABLE"             -- Tracy

build_extras = ""
if arg[1] == "tracy" or arg[2] == "tracy" then
   build_extras = tracy_arguments
end

-- random things may need in future
-- -lGL -lGLX -lGLX_nvidia -lGLEW -lglfw

success, exit, code = 1, 1, 1

if arg[1] == "debug" or arg[1] == nil then
   success, exit, code= os.execute( build_command .. build_extras )
elseif arg[1] == "shipping_test" then
   print( "Building optimized shipping build, checking all warnings" )
   success, exit, code = os.execute( build_command_shipping_test .. build_extras )
else
   print( "Argument not supported" )
end

os.exit(code)
