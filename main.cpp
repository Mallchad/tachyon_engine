#include <cstdio>


struct struct_t
{
    int x;
    int y;
    int z;
};
int main()
{

    wchar_t test = L'æ';
    const char* uni8 = "£abab æłðæłð ababa";
    const wchar_t* uni_literal;
    uni_literal = L"ababæłðæłðabab";

    int uninitialized;
    struct_t default_initialized {};
    // uni8 = (const char*)(uni_literal);

    // std::u32string unicode_string = U"æłðæłð";

    // std::cout << uninitialized << "\n";
    // std::cout << default_initialized.x << "\n";
    // std::cout << unicode_string << "\n";

    printf("%s", uni8);
    // printf("%ls", uni_literal);
    printf("%s", "\n");


}
