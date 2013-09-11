#include <stdio.h>

int main(int argc, const char * argv[])
{
    char buf[256];
    printf("Input: ");
    gets(buf);
    printf(buf);
    return 0;
}

