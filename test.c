#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    while (1) { fork(); }
    return EXIT_SUCCESS;
}
