
#define TRACY_NO_EXIT 1
#define TRACY_ENABLE 1
// Useful if memory demand and precision is high, messes up the tree structure
#define TRACY_ON_DEMAND 1

#include <Tracy.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <functional>

long long iterations    = 100'000;
long long frames        = 100;

void reference_add_one(long long& subject)
{
    ZoneScoped;
    subject = subject + 1;
}

void reference_increment(long long& subject)
{
    ZoneScoped;
    subject +=1;
}
void value_increment(long long subject)
{
    ZoneScoped;
    ++subject;
}
long long value_increment_loop(long long subject)
{
    ZoneScopedN("value_increment_loop frame");
    for (int i=1'000'000; i>0; --i)
    {
        subject = subject + 1;
    }
    return subject;
}
long long reference_increment_loop(long long subject)
{
    ZoneScopedN("reference_increment_loop frame");
    for (int i=1'000'000; i>0; --i)
    {
        subject = subject + 1;
    }
    return subject;
}
std::string value_copy_loop(std::string subject)
{
    ZoneScoped;
    for (int i=1'000'000; i>0; --i)
    {
        subject = "new string that must be longer than 25 chars";
    }
    return subject;
}
std::string reference_copy_loop(std::string& subject)
{
    ZoneScoped;
    for (int i=1'000'000; i>0; --i)
    {
        subject = "new string that must be longer than 25 chars";
    }
    return subject;
}
int main()
{
    ZoneScopedC(tracy::Color::ColorType::Brown);
    long long add_one_subject = 0;
    long long normal_increment_subject = 0;
    long long value_increment_subject = 0;

    long long value_increment_loop_subject = 0;
    long long reference_increment_loop_subject = 0;

    std::string value_copy_loop_subject = {};
    std::string reference_copy_loop_subject = {};

    char* rgba_t[32*32] = {};
    FrameImage(rgba_t, 32, 32, 0, 0);

    char* big_array = new char [1'000'000'000];
    memset(big_array, 69, sizeof(char));

    // Mean frametime 35ns
    // for (int i=iterations; i<iterations; ++i)
    // {
    // subject = subject + 1;
    // FrameMark;
    // }

    // Seeing if threads affects the impact of the loops (it doesn't).
    std::function<void (long long)> func_t = value_increment;
    // std::thread test(value_increment, add_one_subject);

    // if ( test.joinable() )
    // {
    //     std::cout << "Thread is joinable" << std::endl;
    //     test.join();
    // }
    // else
    // {
    //     std::cout << "Thread is not joinable" << std::endl;
    // }
    for (int i=0; i<frames; ++i)
    {
        // ZoneScopedN("Main Loop");
        reference_add_one(add_one_subject);
        reference_increment(normal_increment_subject);
        value_increment(value_increment_subject);

        // Both ~24ns inner loop within margin of error, no difference
        value_increment_loop(value_increment_loop_subject);
        reference_increment_loop(reference_increment_loop_subject);

        // Also ~24ns ????
        value_copy_loop(value_copy_loop_subject);
        reference_copy_loop(reference_copy_loop_subject);

        FrameMark;              // Profiler hint
    }

    delete[] big_array;
    return 0;
}
