
#include "include_core.h"

namespace tyon
{

globals* global;
globals* globals::primary_database = nullptr;

globals* globals::get_primary()
{
    return primary_database;
}

}
