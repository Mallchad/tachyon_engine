
#include "global.h"

using def = globals;

globals* def::primary_database = nullptr;

globals* def::get_primary()
{
    return primary_database;
}
