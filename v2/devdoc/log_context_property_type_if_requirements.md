# `log_context_property_type_if` requirements

`log_context_property_type_if` is an interface that defines a set of APIs commonly used when working with types in the `c_logging` implementation:

- to_string
- copy
- free
- get_type

## Exposed API

```
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
typedef void (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);

typedef struct LOG_CONTEXT_PROPERTY_TYPE_IF_TAG
{
    LOG_CONTEXT_PROPERTY_TYPE_TO_STRING to_string;
    LOG_CONTEXT_PROPERTY_TYPE_COPY copy;
    LOG_CONTEXT_PROPERTY_TYPE_FREE free;
    LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE get_type;
} LOG_CONTEXT_PROPERTY_TYPE_IF;

// a convenient macro for obtaining a certain type concrete implementation
// in a consistent way
#define LOG_CONTEXT_PROPERTY_TYPE_IMPL(type_name) \
    MU_C2(type_name, _log_context_property_type)
```

## LOG_CONTEXT_PROPERTY_TYPE_TO_STRING

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
```

LOG_CONTEXT_PROPERTY_TYPE_TO_STRING produces the string representation of the value pointed to be `property_value`.

The result is placed in `buffer`, while observing the size of `buffer` to be `buffer_length`.

## LOG_CONTEXT_PROPERTY_TYPE_COPY

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy` copies the value pointed by `src_value` to `dst_value`.

## LOG_CONTEXT_PROPERTY_TYPE_COPY

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* value);
```

`LOG_CONTEXT_PROPERTY_TYPE_COPY` releases any resources associated with `value`.

## LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE` returns the implemented type as known by `c_logging`.

## LOG_CONTEXT_PROPERTY_TYPE_IMPL(type_name)

**SRS_LOG_CONTEXT_PROPERTY_TYPE_IF_01_001: [** `LOG_CONTEXT_PROPERTY_TYPE_IMPL(type_name)` shall produce the token `{type_name}_log_context_property_type`. **]**
