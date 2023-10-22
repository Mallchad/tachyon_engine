
// Standard Lirary
#include <iostream>
#include <bits/this_thread_sleep.h>
#include <chrono>

// Project Local
#include "global.h"
#include "renderer.h"
#include "../tracy/Tracy.hpp"

#include <GL/gl.h>

int main() 
{
    using namespace std::chrono_literals;
    global_database& global = global_database::primary_database;
    global.kill_program = false;
    // Forward Declarations


    // Trick to allow jump to cleanup
    try
    {

        renderer render = {};
        while ( global.kill_program == false )
        {
            // render.draw_test_triangle();
            // render.draw_test_rectangle(render.mrectangle_color);
            // render.draw_test_circle(render.mcircle_color);
            render.draw_test_signfield(render.msignfield_color);
            render.refresh();
            (void)FrameMark("Main Render Thread");

            // DO NOT REMOVE, can lock computer if it runs too fast
            std::this_thread::sleep_for(1.6ms);
        }
    } catch (...) {}

    }
