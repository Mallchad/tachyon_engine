
#include "main_include.h"

int main( int argc, char** argv )
{
    using namespace std::chrono_literals;
    library_context _library; library_context_init( &_library); g_library = &_library;
    globals _global = {}; global = &_global;
    global->program_epoch = get_time();

    {
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
        { global->graphics_llvmpipe = true; }
        else if (x_arg == "--debugger-mode")
        { global->debugger_mode = true; }
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
