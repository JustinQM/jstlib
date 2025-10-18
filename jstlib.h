#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

/*------------------------------------------------------
                    VARRAY
------------------------------------------------------*/
#define VARRAY_GROWTH_FACTOR 2 // 2x
#define VARRAY_SHRINK_FACTOR 2 // 1/2
#define VARRAY_SHRINK_THRESHOLD 3 // 1/3 -> shrink
#define VARRAY_DEFAULT_CAPACITY 1

typedef struct _Varray_Header {
    size_t count;
    size_t capacity;
    size_t stride;
} _Varray_Header;

#define Varray(T) T*

#define varray_reserve(v, newcap) do { \
    v = _varray_reserve(v, newcap, sizeof*(v)); \
} while(0)

#define varray_push(v, value) do { \
    auto _tmp = (value); \
    v = _varray_push(v, &_tmp, sizeof(_tmp)); \
} while(0)

#define varray_push_many(v, data, n) \
    v = _varray_push_many(v, data, n, sizeof(*v))

#define varray_insert(v, value, i) do { \
    auto _tmp = (value); \
    v = _varray_insert(v, &_tmp, i, sizeof(_tmp)); \
} while(0)

#define varray_pop(v) *(typeof(v))_varray_pop((void**)&v)

#define varray_erase(v, i) v = _varray_erase(v, i)

#define varray_at(v, i) *(typeof(v))_varray_at(v, i)

#define varray_front(v) *(typeof(v))v

#define varray_back(v) *(typeof(v))_varray_back(v)

#define varray_clear(v) _varray_clear(v)

#define varray_free(v) do { \
    _varray_free((void*)v); \
    v = nullptr; \
} while(0)

#define varray_count(v) _varray_count(v)

#define varray_capacity(v) _varray_capacity(v)

#define varray_stride(v) _varray_stride(v)

void* _varray_init(size_t stride);
void* _varray_grow(void* v);
void* _varray_shrink(void* v);
void* _varray_reserve(void* v, size_t newcap, size_t stride);
void* _varray_push(void* v, const void* data, size_t stride);
void* _varray_push_many(void* v, const void* data, size_t n, size_t stride);
void* _varray_insert(void* v, const void* data, size_t i, size_t stride);
void* _varray_pop(void** v_ptr);
void* _varray_erase(void* v, size_t i);
void* _varray_at(void* v, size_t i);
void* _varray_back(void* v);
void _varray_clear(void* v);
void _varray_free(void* v);
size_t _varray_count(void* v);
size_t _varray_capacity(void* v);
size_t _varray_stride(void* v);

static inline _Varray_Header* _varray_get_header(void* v);
static inline void* _varray_get_data(_Varray_Header* header);
/*------------------------------------------------------
                    IMPLEMENTATION
------------------------------------------------------*/
#ifdef JSTLIB_IMPLEMENTATION
/*------------------------------------------------------
                    VARRAY IMPLEMENTATION
------------------------------------------------------*/


void* _varray_init(size_t stride)
{
    size_t bytes = sizeof(_Varray_Header) + (stride * VARRAY_DEFAULT_CAPACITY);
    _Varray_Header* header = (_Varray_Header*)malloc(bytes);;
    header->count = 0;
    header->capacity = VARRAY_DEFAULT_CAPACITY;
    header->stride = stride;
    return _varray_get_data(header);
}

void* _varray_grow(void* v)
{
    _Varray_Header* header = _varray_get_header(v);
    header->capacity = header->capacity * VARRAY_GROWTH_FACTOR;
    size_t bytes = sizeof(*header) + (header->capacity * header->stride);
    header = realloc(header, bytes);
    return _varray_get_data(header);
}

void* _varray_shrink(void* v)
{
    _Varray_Header* header = _varray_get_header(v);
    header->capacity = header->capacity / VARRAY_SHRINK_FACTOR;
    size_t bytes = sizeof(*header) + (header->capacity * header->stride);
    header = realloc(header, bytes);
    return _varray_get_data(header);
}

void* _varray_reserve(void* v, size_t newcap, size_t stride)
{
    if(v == nullptr) v = _varray_init(stride);
    _Varray_Header* header = _varray_get_header(v);
    if(header->capacity > newcap) return v;

    header->capacity = newcap;
    size_t bytes = sizeof(*header) + (header->capacity * header->stride);
    header = realloc(header, bytes);
    
    return _varray_get_data(header);
}

void* _varray_push(void* v, const void* data, size_t stride)
{
    if(v == nullptr) v = _varray_init(stride);
    _Varray_Header* header = _varray_get_header(v);
    if(header->capacity == header->count) 
    {
        v = _varray_grow(v);
        header = _varray_get_header(v);
    }    

    memcpy((uint8_t*)v + (header->stride * header->count), data, header->stride); 
    header->count++;

    return v;
}

void* _varray_push_many(void* v, const void* data, size_t n, size_t stride)
{
    if(v == nullptr) v = _varray_init(stride);
    _Varray_Header* header = _varray_get_header(v);

    v = _varray_reserve(v, header->count + n, stride);
    header = _varray_get_header(v);

    void* dest = (uint8_t*)v + (header->count * header->stride);
    memcpy(dest, data, n * header->stride);
    header->count = header->count + n;

    return v;
}

void* _varray_insert(void* v, const void* data, size_t i, size_t stride)
{
    if(v == nullptr) v = _varray_init(stride);
    _Varray_Header* header = _varray_get_header(v);
    assert(i <= header->count && "Varray Insert: Cannot insert beyond last element");
    if(header->capacity == header->count) 
    {
        v = _varray_grow(v);
        header = _varray_get_header(v);
    } 
    
    void* src = ((uint8_t*)v+(i*header->stride));
    void* dest = (uint8_t*)src + header->stride;
    size_t bytes = (header->count-i)*header->stride;
    memmove(dest, src, bytes);
    memcpy(src, data, header->stride);
    header->count++;

    return v;
}

void* _varray_pop(void** v_ptr)
{
    void* v = *v_ptr;
    assert(v != nullptr);
    _Varray_Header* header = _varray_get_header(v);
    if(header->count < header->capacity / VARRAY_SHRINK_THRESHOLD)
    {
        *v_ptr = _varray_shrink(v);
        v = *v_ptr;
        header = _varray_get_header(v);
    }
    header->count--;
    return ((uint8_t*)v + (header->stride * header->count));
}

void* _varray_erase(void* v, size_t i)
{
    _Varray_Header* header = _varray_get_header(v);
    assert(i < header->count);
    if(header->count < header->capacity / VARRAY_SHRINK_THRESHOLD)
    {
        v = _varray_shrink(v);
        header = _varray_get_header(v);
    }

    if(i == header->count - 1)
    {
        header->count--;
        return v;
    }

    void* src = (uint8_t*)v + ((i+1) * header->stride);
    void* dest = (uint8_t*)v + (i * header->stride);
    size_t bytes = header->stride * (header->count - (i+1));
    memmove(dest, src, bytes);
    header->count--;

    return v;
}

void* _varray_at(void* v, size_t i)
{
    assert(v != nullptr);
    _Varray_Header* header = _varray_get_header(v);

    return ((uint8_t*)v + (header->stride * i));
}

void* _varray_back(void* v)
{
    _Varray_Header* header = _varray_get_header(v);
    assert(header->count > 0);
    return ((uint8_t*)v + (header->stride * (header->count - 1)));
}

void _varray_clear(void* v)
{
    _Varray_Header* header = _varray_get_header(v);
    header->count = 0;
}

void _varray_free(void* v)
{
    _Varray_Header* header = _varray_get_header(v);
    free(header);
}

static inline _Varray_Header* _varray_get_header(void* v)
{
    return (void*)((uint8_t*)v - sizeof(_Varray_Header));
}

static inline void* _varray_get_data(_Varray_Header* header)
{
    return (void*)((uint8_t*)header + sizeof(_Varray_Header));
}

[[maybe_unused]] size_t _varray_count(void* v)
{
    return v ? _varray_get_header(v)->count : 0;
}

[[maybe_unused]] size_t _varray_capacity(void* v)
{
    return v ? _varray_get_header(v)->capacity : 0;
}

[[maybe_unused]] size_t _varray_stride(void* v)
{
    return v ? _varray_get_header(v)->stride : 0;
}


#endif
