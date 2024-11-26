#pragma once

#include "include_core.h"
#include "global.h"

namespace tyon
{
    using namespace std::chrono_literals;
}

/// Has no time period adjustments, is most accurate for time measurements
using monotonic_time = std::chrono::time_point<std::chrono::steady_clock>;
using date_time = std::chrono::time_point<std::chrono::system_clock>;
// using utc_time = std::chrono::time_point<std::chrono::utc_clock>;
/// Atomic Time
// using tai_time = std::chrono::time_point<std::chrono::tai_clock>;
// using gps_time = std::chrono::time_point<std::chrono::gps_clock>;

/// Return a timekeeping sensitive point timepoint
// DO NOT use this as a way to reteive an actual date/time, ie, time since UNIX epoch
monotonic_time
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
