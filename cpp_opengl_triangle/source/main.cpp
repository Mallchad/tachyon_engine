
#include "main_include.h"

int main()
{
    using namespace std::chrono_literals;
    global_database global = {};
    global_database::primary_database = &global;
    global.kill_program = false;

    #ifdef TRACY_ENABLE
    std::cout << "Tracy Client has been enabled for profiling \n";
    #endif

    // Trick to allow jump to cleanup
    try
    {
        renderer render;
        while ( global.kill_program == false )
        {
            render.frame_update(0.1f);
            FrameMark( "Main Render Thread" );

            // DO NOT REMOVE, can lock computer if it runs too fast
            std::this_thread::sleep_for(1.6ms);
        }
    } catch (...)
    {
        std::cout << "An exception occured, attempting to exit safely" << std::endl;
    }
    std::cout << "Exiting gracefully" << std::endl;

}
