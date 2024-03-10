
#include "time.hpp"

monotonic_time
FUNCTION get_time()
{
    auto current_time = std::chrono::steady_clock::now();

    return current_time;
}
