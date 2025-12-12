
#include "main_include.h"

int main( int argc, char** argv )
{
    using namespace std::chrono_literals;
    library_context _library; library_context_init( &_library); g_library = &_library;
    globals _global = {}; global = &_global;
    global->program_epoch = get_time();


    {
        // TODO: Remove before flight
        // Random dumb tests
        uid id = uuid_generate();
        TYON_LOGF( "Generated UUID as presented {}", id );
        char* str = memory_allocate_raw( 20 );
        platform::uuid_unparse( raw_pointer(&id.uuid), str );
        TYON_LOGF( "Expected {}", str );

        test_allocators();
    }

    // Change directory to binary
    std::filesystem::current_path( file_self_directory() );
    TYON_LOGF( "Current working directory: {}",  std::filesystem::current_path() );

    globals::primary_database = &_global;
    global->kill_program = false;

    // Argument processing
    TYON_LOG( "Processing Command Line Arguments" );
    for (int i=0; i < argc; ++i)
    {
        fstring x_arg = argv[i];
        TYON_LOGF( "arg {}: '{}'", i, x_arg );
        if (x_arg == "--no-main-loop"s )
        { global->kill_program = true; }
        else if (x_arg == "--vulkan")
        { global->render_backend = e_render_backend::vulkan; }
        else if (x_arg == "--opengl")
        { global->render_backend = e_render_backend::opengl; }
        else if (x_arg == "--graphics-llvmpipe")
        { global-> graphics_llvmpipe = true; }
    }


    TYON_LOG( "Running executable: ", file_self_filename() );

    #ifdef TRACY_ENABLE
    std::cout << "Tracy Client has been enabled for profiling \n";
    #endif

    lua_State* lua;
    // Trick to allow jump to cleanup
    try
    {
        lua = luaL_newstate();
        global->lua_state = lua;

        render_init();

        // Initialize lua related things
        // Make C libraries available to Lua
        luaL_openlibs( lua );
        TYON_BASE_LOG( "lua", "Lua VM Initialized" );
        if (luaL_dofile( lua, "/mnt/tmp/repos/tachyon_engine/content/options.lua" ))
        { throw 1; };

        // Test Lua Stuff
        lua_table options;
        options.name = "options";
        options.lua = lua;
        options["foo"] = 42002.f;
        if (luaL_dofile( lua, "/mnt/tmp/repos/tachyon_engine/content/early_start.lua" ))
        { throw 1; };

        while ( global->kill_program == false )
        {
            auto epoch_elapsed = std::chrono::steady_clock::now() - global->program_epoch;
            float epoch_elapsed_float = std::chrono::duration_cast<
                std::chrono::duration<f32>>( epoch_elapsed ).count();

            // Draw tick last
            x11_tick();
            render_tick();
            // main_renderer.frame_update( );
            // main_input.frame_update( epoch_elapsed_float );

            // DO NOT REMOVE, can lock computer if it runs too fast
            std::this_thread::sleep_for(16ms);
            FrameMark;
        }
    } catch (std::exception& error)
    {
        std::cout << "An exception occured, attempting to exit safely\n" <<
                  "Exception: " << error.what( ) << std::endl;
    } catch (const char* error)
    {
        std::cout << "An exception occured, attempting to exit safely\n" <<
                  "Exception: " << error << std::endl;

    }  catch (...)
    {
        std::cout << "An undocumented exception occured, attempting to exit safely\n" << std::endl;
    }

    // Cleanup
    vulkan_destroy();
    lua_close( lua );
    std::cout << "Exiting gracefully" << std::endl;

    return EXIT_SUCCESS;
}
