#include <stdio.h>

#define JSTLIB_IMPLEMENTATION
#include "jstlib.h"

int main(void)
{
    Varray(char) string = {};
    char* raw_string = "Hello, World!";

    varray_reserve(string, strlen(raw_string));
    varray_push_many(string, raw_string, strlen(raw_string));

    char* raw_string2 = " Goodbye, World!";
    varray_push_many(string, raw_string2, strlen(raw_string2) + 1);

    printf("%s\n", string);
    printf("Length of varray:%zu\n", varray_count(string));
    printf("Capacity of varray:%zu\n", varray_capacity(string));

    varray_free(string);

    return 0;
}
