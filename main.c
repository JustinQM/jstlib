#include <stdio.h>

#define JSTLIB_IMPLEMENTATION
#include "jstlib.h"

int main(void)
{

    Varray(char) string = {};
    char* raw_string = "Hello, World!";

    varray_reserve(string, strlen(raw_string) + 1);

    for (int i = 0; i < strlen(raw_string); i++)
    {
        varray_push(string, raw_string[i]);
    }

    varray_push(string, '\0');

    printf("%s\n", string);
    printf("Length of varray:%zu\n", varray_count(string));
    printf("Capacity of varray:%zu\n", varray_capacity(string));

    varray_free(string);

    return 0;
}
