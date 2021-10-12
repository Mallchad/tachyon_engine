#include <iostream>
#include <string>

#include <cstdint>

int main()
{
/*
 * My soltion is roughly as follows, theory on-paper first,
 * then actual code.
 *
 * First we start with a fixed width integer (for sanity reasons), lets use 16 bit signed
 * for both numbers `x and y`, we take them away from each other,
 * `x-y ; y-x`
 * Then we take the sign bit of each to check if they are negative.
 * `sign_bit_x = x & (1 << 16); sign_bit_y = y & (1 << 16)`
 *
 * From here we need to 'conditionally' set all bits if each respective sign bit is set.
 * Here we have two options, naive solution
 * Shift the bit around then OR it with each of the 16 positions on an variable with unset bits
 * unsigned bitcontainer = 0
 * bitcontainer |= sign_bit ; bitcontainer |= (sign_bit >> 1) ; bitcontainer |= (sign_bit >> 2)
 * etc etc, all the way till 16.
 *
 * Alternative method
 * That's a lot of writing, if you want to be lazy you can just use some signed integer bit representaitons
 * To obtained a bitset where everything is 1, we can use signed 0, and subtract 1
 * To conditionally set everything, do '1 & sign_bit', and subtract from that instead
 * If the bit is set, it takes away 1, otherwise, take away 0.
 * conditionull_x = 0 - (1 & sign_bit_x)
 * conditionull_y = 0 - (1 & sign_bit_y)
 * We can then simply AND the bitset to change the number to itself, or 0
 *
 * We can then create strings of the appropriate original numbers with stoi
 * And then simply set the null terminator at 0 if the index is 0
 * std::string string_x = {}; string_x.reserve(100); string_x = std::to_string(x)
 * std::string string_y = {}; string_y.reserve(100); string_y = std::to_string(y)
 * string_x[conditionull_x & 90] = '\0'
 * Then print out both strings and only the greater shall remain
 * std::cout << string_x << string_y << "\n";
 *
 * Or instead of doing the 'conditional' stuff you can just rightshift the sign bit 15 places and multiply
 *
 * Footnote:
 * For some reason the program doesn't work if the c string doens't reference the
 * std::string's internal array. I'm not in the right frame of mind to figure out why.
 * So it's staying there for now. As an advantage, you can view it in debuggers as a
 * trivial type more consistently now...
 */

    int x = 5;
    int y = 24;

    int16_t difference_x = x-y;
    int16_t difference_y = y-x;

    uint16_t sign_bit_x = difference_x & (1<<15);
    uint16_t sign_bit_y = difference_y & (1<<15);

    // Flip so the positive number has its one_bit set rather than unset
    uint16_t one_bit_x = 1 ^ (sign_bit_x >> 15);
    uint16_t one_bit_y = 1 ^ (sign_bit_y >> 15);

    int16_t conditionull_x = x * one_bit_x;
    int16_t conditionull_y = y * one_bit_y;

    std::string string_x = std::to_string(x);
    std::string string_y = std::to_string(y);
    string_x.reserve(100);
    string_y.reserve(100);
    string_x = std::to_string(x);
    string_x = std::to_string(y);

    // --- "The Fix" ---
    char* str_view_x = (char*)string_x.c_str();
    char* str_view_y = (char*)string_y.c_str();
    string_x = std::to_string(x);
    string_x = std::to_string(y);
    // --- End of "The Fix" ---

    string_x[one_bit_x & 0b1111] = '\0';
    string_y[one_bit_y & 0b1111] = '\0';

    std::cout << "The larger number is:  "
              << string_x
              << string_y
              << "\n";
}
