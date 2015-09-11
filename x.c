#include <stdio.h>

struct a {
    char a;
    int x[0];
};
    int
main(int argc, char **argv)
{
    struct a *b = NULL;
    printf("xx=%d\n", (int)b->x);
    printf("sizeof(a)=%lu\n", sizeof(struct a));
    return 0;
}
