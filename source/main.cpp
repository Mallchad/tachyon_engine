
#include "main_include.h"

int main()
{
    using namespace std::chrono_literals;
    globals global = {};
    global.program_epoch = get_time();

    globals::primary_database = &global;
    global.kill_program = false;

    #ifdef TRACY_ENABLE
    std::cout << "Tracy Client has been enabled for profiling \n";
    #endif

    lua_State* lua;
    // Trick to allow jump to cleanup
    try
    {
        lua = luaL_newstate();
        renderer main_renderer;
        input main_input( main_renderer.platform );

        // Initialize lua related things
        // Make C libraries available to Lua
        luaL_openlibs( lua );
        log( "lua", "Lua VM Initialized" );

        // Testing lua stuff
        table options;
        options.table = "options";
        options.lua = lua;
        lua_createtable( lua, 30, 200 );
        lua_setglobal( lua, options.table );

        while ( global.kill_program == false )
        {
            auto epoch_elapsed = std::chrono::steady_clock::now() - global.program_epoch;
            float epoch_elapsed_float = std::chrono::duration_cast<
                std::chrono::duration<ffloat>>( epoch_elapsed ).count();

            main_renderer.frame_update( );
            main_input.frame_update( epoch_elapsed_float );

            // DO NOT REMOVE, can lock computer if it runs too fast
            std::this_thread::sleep_for(100us);
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
    lua_close( lua );
    std::cout << "Exiting gracefully" << std::endl;

    return EXIT_SUCCESS;
}
