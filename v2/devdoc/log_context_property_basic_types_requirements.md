# `log_context_property_basic_types` requirements

`log_context_property_basic_types` is a collection of log context property interface concrete implementations that implement the code needed for handling basic types (integers, bool, etc.).

The list of supported basic types is:

- int64_t
- uint64_t
- int32_t
- uint32_t
- int16_t
- uint16_t
- int8_t
- uint8_t

Note: More types will be added as needed.

## Exposed API

```
#define SUPPORTED_BASIC_TYPES \
    int64_t, \
    uint64_t, \
    int32_t, \
    uint32_t, \
    int16_t, \
    uint16_t, \
    int8_t, \
    uint8_t

#define DECLARE_BASIC_TYPE(type_name) \
    extern const LOG_CONTEXT_PROPERTY_TYPE_IF MU_C2(type_name, _log_context_property_type);

MU_FOR_EACH_1(DECLARE_BASIC_TYPE, SUPPORTED_BASIC_TYPES)
```

## LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string` produces the string representation of an `int64_t`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_001: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string` shall fail and return -1. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [** `LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string` shall call `snprintf` with `buffer`, `buffer_length` and format string `PRId64` and pass in the values list the `int64_t` value pointed to be `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [** `LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_004: [** If any error is encountered, `LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string` shall fail and return -1. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy` copies the `int64_t` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_005: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_006: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_007: [** `LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy` shall copy the bytes of the `int64_t` value from the address pointed by `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_008: [** `LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).free

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_009: [** `LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_010: [** `LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).get_type` shall returns the property type `LOG_CONTEXT_PROPERTY_TYPE_int64_t`. **]**

Note: the rest of the types will be implemented by cloning int32_t, thus will be done in a separate PR.
