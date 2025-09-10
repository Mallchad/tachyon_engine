
#include "include_core.h"

using def = globals;

globals* global;
globals* def::primary_database = nullptr;

globals* def::get_primary()
{
    return primary_database;
}
