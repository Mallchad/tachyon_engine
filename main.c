#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    static_assert(true, "wut");
    /* bool foo = true; */
    true;
    false;
    _Bool a = 1;

    char array_t[100] = {};
    char const* string_t = (char*)calloc(1000, sizeof(char));
    printf("%s", "sizeof array_t: ");
    printf("%i", sizeof(array_t));
    printf("%s", "\nsizeof string_t: ");
    printf("%i", sizeof(string_t));
    printf("%s", "\n");
    return 1;
                            }
