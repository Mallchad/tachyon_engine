#include <iostream>
// Tweaked version which nulls value and can unset bits?

// Also all with added sanity checks
template<typename t_binary> t_binary sset_bitr(t_binary subject, int max_range, bool set_bit = true) {
    if (max_range >= sizeof(subject)*8 || max_range < 0)
        throw(max_range);
    t_binary tmp = 0b0;
    for (int i=0; i<max_range; ++i)
    {
        if (set_bit) tmp |= 1<<i; else tmp &= ~(1<<i);
    }
    return tmp;
}
// More fliptastic stuff
template<typename t_binary>
t_binary stwiddle_bit(t_binary subject, int target_bit, bool set_bit = true)
{
    if (target_bit >= sizeof(subject)*8 || target_bit < 0)
        throw(target_bit);
    t_binary tmp = subject;
    set_bit ?
        tmp |= 1<<target_bit :
        tmp &= ~(1<<target_bit); return tmp;
}
template<typename t_binary> t_binary
sflip_bit(t_binary subject, int target_bit)
{
    if (target_bit >= sizeof(subject)*8 || target_bit < 0)
        throw(target_bit);
    t_binary tmp = subject;
    tmp ^= 1<<target_bit;
    return tmp;
}
int bits()
{
    std::cout << sset_bitr(1l, 62) << "\n"
              << sflip_bit(1123123123, 10)
              << std::endl;
    return 1;
}
int main()
{
    return bits();
}
