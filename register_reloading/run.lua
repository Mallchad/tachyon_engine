local compile_command = "clang++ register_reloading.cpp tracy/TracyClient.cpp -Itracy/ /usr/lib/libpthread.so -D TRACY_ENABLE=1 /usr/lib/libdl.so"

os.execute(compile_command);
