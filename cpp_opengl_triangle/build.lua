#!/bin/env lua

build_command = "clang++ -g -O0 -Wpedantic -std=c++20 -lX11 -lGL *.cpp *.h"
build_command_release = "clang++ -g -O2 -march=native -Weverything -Wno-c++98-compat -Wno-implicit-int-float-conversion -Wno-sign-conversion -analyze -std=c++20 -lX11 -lGL *.cpp *.h"
-- -lGL -lGLX -lGLX_nvidia -lGLEW -lglfw
if arg[1] == "debug" or arg[1] == nil then
   os.execute(build_command)
elseif arg[1] == "shipping" then
   print("Building optimized shipping build, checking all warnings")
   os.execute(build_command_release)
end
