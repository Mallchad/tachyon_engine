#pragma once

namespace tyon
{
using namespace std::chrono_literals;

/// Return a timekeeping sensitive point timepoint
// DO NOT use this as a way to reteive an actual date/time, ie, time since UNIX epoch
time_monotonic
FUNCTION get_time();

template<typename t_tick_interval>
t_tick_interval
FUNCTION time_elapsed()
{
    std::chrono::duration<t_tick_interval> elapsed =
        get_time() - globals::get_primary()->program_epoch;
    t_tick_interval elapsed_conversion = std::chrono::duration_cast<
        std::chrono::duration<t_tick_interval> >( elapsed ).count();

    return elapsed_conversion;
}

}
