#include <stdio.h>

#include "jstlib.h"

void print_varray(int* v)
{
    for(int i = 0; i < varray_count(v); i++)
    {
        printf("%d,",v[i]);
    }

    printf("\n");
    printf("Count:%zu\n", varray_count(v));
}
