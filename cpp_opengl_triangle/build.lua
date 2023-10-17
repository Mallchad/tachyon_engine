#!/bin/env lua

build_command = "clang++ -g -std=c++20 -lX11 -lGL *.cpp *.h"
-- -lGL -lGLX -lGLX_nvidia -lGLEW -lglfw
os.execute(build_command)
