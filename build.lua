#!/bin/env lua

require("lib.shim_lib")

gmodule_name = "Build Script"
log_verbose = false

success, exit, code = safe_execute( "xmake build" )

os.exit(code)
