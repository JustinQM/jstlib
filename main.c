#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "jstlib.h"

#define VARRAY_GROWTH_FACTOR 2 // 2x
#define VARRAY_SHRINK_FACTOR 2 // 1/2
#define VARRAY_SHRINK_THRESHOLD 3 // 1/3 -> shrink
#define VARRAY_DEFAULT_CAPACITY 1

typedef struct _Varray_Header {
    size_t count;
    size_t capacity;
    size_t stride;
} _Varray_Header;

static inline _Varray_Header* _varray_get_header(void* mem)
{
    return (void*)((uint8_t*)mem - sizeof(_Varray_Header));
}

static inline void* _varray_get_data(_Varray_Header* header)
{
    return (void*)((uint8_t*)header + sizeof(_Varray_Header));
}

void* _varray_init(size_t stride)
{
    size_t bytes = sizeof(_Varray_Header) + (stride * VARRAY_DEFAULT_CAPACITY);
    _Varray_Header* header = (_Varray_Header*)malloc(bytes);;
    header->count = 0;
    header->capacity = VARRAY_DEFAULT_CAPACITY;
    header->stride = stride;
    return _varray_get_data(header);
}


void* _varray_grow(void* mem)
{
    printf("Reallocation: GROW\n");
    _Varray_Header* header = _varray_get_header(mem);
    header->capacity = header->capacity * VARRAY_GROWTH_FACTOR;
    size_t bytes = sizeof(*header) + (header->capacity * header->stride);
    header = realloc(header, bytes);
    return _varray_get_data(header);
}

void* _varray_shrink(void* mem)
{
    printf("Reallocation: SHRINK\n");
    _Varray_Header* header = _varray_get_header(mem);
    header->capacity = header->capacity / VARRAY_SHRINK_FACTOR;
    size_t bytes = sizeof(*header) + (header->capacity * header->stride);
    header = realloc(header, bytes);
    return _varray_get_data(header);
}

void* _varray_reserve(void* mem, size_t newcap, size_t stride)
{
    if(mem == nullptr) mem = _varray_init(stride);
    _Varray_Header* header = _varray_get_header(mem);
    if(header->capacity > newcap) return mem;

    header->capacity = newcap;
    size_t bytes = sizeof(*header) + (header->capacity * header->stride);
    header = realloc(header, bytes);
    
    return _varray_get_data(header);
}

void* _varray_push(void* mem, const void* data, size_t stride)
{
    if(mem == nullptr) mem = _varray_init(stride);
    _Varray_Header* header = _varray_get_header(mem);
    if(header->capacity == header->count) 
    {
        mem = _varray_grow(mem);
        header = _varray_get_header(mem);
    }    

    memcpy((uint8_t*)mem + (header->stride * header->count), data, header->stride); 
    header->count++;

    return mem;
}

void* _varray_pop(void** raw_mem)
{
    void* mem = *raw_mem;
    assert(mem != nullptr);
    _Varray_Header* header = _varray_get_header(mem);
    if(header->count < header->capacity / VARRAY_SHRINK_THRESHOLD)
    {
        *raw_mem = _varray_shrink(mem);
        mem = *raw_mem;
        header = _varray_get_header(mem);
    }
    header->count--;
    return ((uint8_t*)mem + (header->stride * header->count));
}

void* _varray_at(void* mem, size_t i)
{
    assert(mem != nullptr);
    _Varray_Header* header = _varray_get_header(mem);

    return ((uint8_t*)mem + (header->stride * i));
}

void* _varray_back(void* mem)
{
    _Varray_Header* header = _varray_get_header(mem);
    assert(header->count > 0);
    return ((uint8_t*)mem + (header->stride * (header->count - 1)));
}

void _varray_clear(void* mem)
{
    _Varray_Header* header = _varray_get_header(mem);
    header->count = 0;
}

void _varray_free(void* mem)
{
    _Varray_Header* header = _varray_get_header(mem);
    free(header);
}

#define Varray(T) T*

#define varray_reserve(v, newcap) do { \
    v = _varray_reserve(v, newcap, sizeof*(v)); \
} while(0)

#define varray_push(v, value) do { \
    auto _tmp = (value); \
    v = _varray_push(v, &_tmp, sizeof(_tmp)); \
} while(0)

#define varray_pop(v) *(typeof(v))_varray_pop((void**)&v)

#define varray_at(v, i) *(typeof(v))_varray_at(v, i)

#define varray_front(v) *(typeof(v))v

#define varray_back(v) *(typeof(v))_varray_back(v)

#define varray_clear(v) _varray_clear(v)

#define varray_free(v) do { \
    _varray_free((void*)v); \
    v = nullptr; \
} while(0)

[[maybe_unused]] static inline size_t varray_count(void* mem)
{
    return mem ? _varray_get_header(mem)->count : 0;
}

[[maybe_unused]] static inline size_t varray_capacity(void* mem)
{
    return mem ? _varray_get_header(mem)->capacity : 0;
}

[[maybe_unused]] static inline size_t varray_stride(void* mem)
{
    return mem ? _varray_get_header(mem)->stride : 0;
}

//TODO:
//insert
//erase
//swap
//?shrink_to_fit

typedef struct {
    uint64_t a;
    uint64_t b;
} TestType;

int main(void)
{
    Varray(TestType) v = {};

    varray_reserve(v, 72);

    printf("Varray capacity:%zu\n", varray_capacity(v));
    printf("Varray count:%zu\n", varray_count(v));
    printf("Varray stride:%zu\n", varray_stride(v));

    TestType test = {993, 69};

    varray_push(v, test);

    for(int i = 0; i <= 69; i++)
    {
        TestType think = {i, i};
        varray_push(v, think);
    }

    test.a = 69;

    varray_push(v, test);

    printf("Varray capacity:%zu\n", varray_capacity(v));
    printf("Varray count:%zu\n", varray_count(v));

    test = varray_back(v);
    printf("Value at index %zu: %lu\n", varray_count(v) - 1, test.a);
    test = varray_front(v);
    printf("Value at front:  %lu\n", test.a);

    for(int i = varray_count(v); i > 0; i--)
    {
        varray_pop(v);
    }

    varray_free(v);

    return 0;
}
