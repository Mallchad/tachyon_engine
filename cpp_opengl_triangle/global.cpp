
#include "global.h"

global_database global_database::primary_database = {};

global_database& global_database::get_database()
{
    return this->primary_database;
}
