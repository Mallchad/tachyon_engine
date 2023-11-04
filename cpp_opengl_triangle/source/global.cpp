
#include "global.h"

global_database* global_database::primary_database = nullptr;

global_database* global_database::get_primary()
{
    return primary_database;
}
