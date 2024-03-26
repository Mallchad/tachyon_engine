
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

    // Trick to allow jump to cleanup
    try
    {
        renderer main_renderer;
        input main_input( main_renderer.platform );

        while ( global.kill_program == false )
        {
            auto epoch_elapsed = std::chrono::steady_clock::now() - global.program_epoch;
            float epoch_elapsed_float = std::chrono::duration_cast<
                std::chrono::duration<ffloat>>( epoch_elapsed ).count();

            main_renderer.frame_update( epoch_elapsed_float );
            main_input.frame_update( epoch_elapsed_float );

            FrameMark( "Main Render Thread" );

            // DO NOT REMOVE, can lock computer if it runs too fast
            std::this_thread::sleep_for(100us);
        }
    } catch (...)
    {
        std::cout << "An exception occured, attempting to exit safely" << std::endl;
    }
    std::cout << "Exiting gracefully" << std::endl;

}
