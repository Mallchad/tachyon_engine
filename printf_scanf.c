#include <stdio.h>
#include <math.h>

int printf_scanf(){
    double num = 0.0f;
    printf("%s", "Input a number to cube: ");
    scanf("%g", &num);
    printf("Answer: %g", num*num*num);
    return 0;
}

int main()
{
    printf_scanf();
    return 0;
}
