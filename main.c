#include <stdio.h>

#define JSTLIB_IMPLEMENTATION
#include "jstlib.h"

void print_varray(int* v);
int main(void)
{
    Varray(int) v = {};

    varray_reserve(v, 100);

    for(int i = 0; i <= 69; i++)
    {
        varray_push(v, i);
    }

    print_varray(v);
    varray_insert(v, 420, 10);
    print_varray(v);
    varray_erase(v, 10);
    print_varray(v);

    int test = varray_at(v, 10);
    printf("Test Value:%d\n", test);

    varray_free(v);

    return 0;
}
