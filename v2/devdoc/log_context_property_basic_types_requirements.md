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
    int LOG_CONTEXT_PROPERTY_TYPE_INIT(type_name)(void* dst_value, type_name src_value); \
    size_t LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(type_name)(type_name src_value); \
    extern const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(type_name); \

MU_FOR_EACH_1(DECLARE_BASIC_TYPE, SUPPORTED_BASIC_TYPES)
```

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string` produces the string representation of an `int64_t`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_001: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string` shall call `snprintf` with `buffer`, `buffer_length` and format string `PRId64` and pass in the values list the `int64_t` value pointed to be `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_004: [** If any error is encountered, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string` shall fail and return a negative value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy` copies the `int64_t` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_005: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_006: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_007: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy` shall copy the bytes of the `int64_t` value from the address pointed by `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_008: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).free

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_009: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_010: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_int64_t`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)(void* dst_value, int64_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)` copies the `src_value` value to the address pointed by `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_081: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_082: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)` shall copy the `src_value` bytes of the `int64_t` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_083: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t)(int64_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t)` returns the number of bytes needed to hold `src_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_084: [** `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t)` shall return `sizeof(int64_t)`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string` produces the string representation of an `uint64_t`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_011: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_012: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string` shall call `snprintf` with `buffer`, `buffer_length` and format string `PRIu64` and pass in the values list the `uint64_t` value pointed to be `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_013: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_014: [** If any error is encountered, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string` shall fail and return a negative value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy` copies the `uint64_t` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_015: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_016: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_017: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy` shall copy the bytes of the `uint64_t` value from the address pointed by `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_018: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).free

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_019: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_020: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_uint64_t`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)(void* dst_value, uint64_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)` copies the `src_value` value to the address pointed by `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_085: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_086: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)` shall copy the `src_value` bytes of the `uint64_t` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_087: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t)(uint64_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t)` returns the number of bytes needed to hold `src_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_088: [** `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t)` shall return `sizeof(uint64_t)`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string` produces the string representation of an `int32_t`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_021: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_022: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string` shall call `snprintf` with `buffer`, `buffer_length` and format string `PRId32` and pass in the values list the `int32_t` value pointed to be `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_023: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_024: [** If any error is encountered, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string` shall fail and return a negative value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy` copies the `int32_t` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_025: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_026: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_027: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy` shall copy the bytes of the `int32_t` value from the address pointed by `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_028: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).free

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_029: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_030: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_int32_t`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)(void* dst_value, int32_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)` copies the `src_value` value to the address pointed by `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_089: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_090: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)` shall copy the `src_value` bytes of the `int32_t` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_091: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t)(int32_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t)` returns the number of bytes needed to hold `src_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_092: [** `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t)` shall return `sizeof(int32_t)`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string` produces the string representation of an `uint32_t`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_031: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_032: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string` shall call `snprintf` with `buffer`, `buffer_length` and format string `PRIu32` and pass in the values list the `uint32_t` value pointed to be `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_033: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_034: [** If any error is encountered, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string` shall fail and return a negative value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy` copies the `uint32_t` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_035: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_036: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_037: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy` shall copy the bytes of the `uint32_t` value from the address pointed by `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_038: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).free

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_039: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_040: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_uint32_t`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)(void* dst_value, uint32_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)` copies the `src_value` value to the address pointed by `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_093: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_094: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)` shall copy the `src_value` bytes of the `uint32_t` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_095: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t)(uint32_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t)` returns the number of bytes needed to hold `src_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_096: [** `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t)` shall return `sizeof(uint32_t)`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string` produces the string representation of an `int16_t`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_041: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_042: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string` shall call `snprintf` with `buffer`, `buffer_length` and format string `PRId16` and pass in the values list the `int16_t` value pointed to be `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_043: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_044: [** If any error is encountered, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string` shall fail and return a negative value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy` copies the `int16_t` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_045: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_046: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_047: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy` shall copy the bytes of the `int16_t` value from the address pointed by `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_048: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).free

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_049: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_050: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_int16_t`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)(void* dst_value, int16_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)` copies the `src_value` value to the address pointed by `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_097: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_098: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)` shall copy the `src_value` bytes of the `int16_t` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_099: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t)(int16_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t)` returns the number of bytes needed to hold `src_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_100: [** `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t)` shall return `sizeof(int16_t)`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string` produces the string representation of an `uint16_t`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_051: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_052: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string` shall call `snprintf` with `buffer`, `buffer_length` and format string `PRIu16` and pass in the values list the `uint16_t` value pointed to be `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_053: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_054: [** If any error is encountered, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string` shall fail and return a negative value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy` copies the `uint16_t` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_055: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_056: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_057: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy` shall copy the bytes of the `uint16_t` value from the address pointed by `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_058: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).free

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_059: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_060: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_uint16_t`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)(void* dst_value, uint16_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)` copies the `src_value` value to the address pointed by `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_101: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_102: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)` shall copy the `src_value` bytes of the `uint16_t` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_103: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t)(uint16_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t)` returns the number of bytes needed to hold `src_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_104: [** `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t)` shall return `sizeof(uint16_t)`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string` produces the string representation of an `int8_t`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_061: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_062: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string` shall call `snprintf` with `buffer`, `buffer_length` and format string `PRId8` and pass in the values list the `int8_t` value pointed to be `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_063: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_064: [** If any error is encountered, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string` shall fail and return a negative value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy` copies the `int8_t` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_065: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_066: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_067: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy` shall copy the byte of the `int8_t` value from the address pointed by `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_068: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).free

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_069: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_070: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_int8_t`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)(void* dst_value, int8_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)` copies the `src_value` value to the address pointed by `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_105: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_106: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)` shall copy the `src_value` bytes of the `int8_t` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_107: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t)(int8_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t)` returns the number of bytes needed to hold `src_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_108: [** `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t)` shall return `sizeof(int8_t)`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string` produces the string representation of an `uint8_t`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_071: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_072: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string` shall call `snprintf` with `buffer`, `buffer_length` and format string `PRIu8` and pass in the values list the `uint8_t` value pointed to be `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_073: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_074: [** If any error is encountered, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string` shall fail and return a negative value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy` copies the `uint8_t` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_075: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_076: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_077: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy` shall copy the byte of the `uint8_t` value from the address pointed by `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_078: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).free

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_079: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_080: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_uint8_t`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)(void* dst_value, uint8_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)` copies the `src_value` value to the address pointed by `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_109: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_110: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)` shall copy the `src_value` bytes of the `uint8_t` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_111: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t)(uint8_t src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t)` returns the number of bytes needed to hold `src_value`.

**SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_112: [** `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t)` shall return `sizeof(uint8_t)`. **]**
