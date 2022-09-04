# `log_context_property_type_ascii_char_ptr` requirements

`log_context_property_type_ascii_char_ptr` implements the code needed for handling an ASCII string.

## Exposed API

```
int LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(void* dst_value, const char* src_value);
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)(const char* format, ...);

extern const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr);

```

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(const void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` produces the string representation of an `ascii_char_ptr`.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_001: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_017: [** If `buffer` is `NULL` and `buffer_length` is greater than 0, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_002: [** If `buffer` is `NULL` and `buffer_length` is 0, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall return the length of the string pointed to by `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_003: [** Otherwise, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall copy the string pointed to by `property_value` to `buffer` by using `snprintf` with `buffer`, `buffer_length` and format string `%s` and pass in the values list the `const char*` value pointed to be `property_value`. **]**

Note: the copying of the string can be somewhat optimized by not using `snprintf`, this will be considered in a separate task.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_004: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_005: [** If any error is encountered, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall fail and return a negative value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, const void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy` copies the `ascii_char_ptr` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_006: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_007: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_008: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy` shall copy the entire string (including the `NULL` terminator) from `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_009: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).free

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_018: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_010: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(void* dst_value, const char* format, ...);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)` initializes the memory at `dst_value` with the `printf` style formatted string given by `format` and the arguments in `...`.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_011: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_012: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)` shall initialize the memory at `dst_value` with the `printf` style formatted string given by `format` and the arguments in `...`. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_013: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)` shall succeed and return 0. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_014: [** If formatting the string fails, `LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)` shall fail and return a non-zero value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)(const char* format, ...);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)` returns the amount of memory needed to store the `printf` style formatted string given by `format` and the arguments in `...`.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_015: [** If the underlying C runtime call used to obtain the amount of memory needed fails, `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)` shall return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_016: [** Otherwise, on success, `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)` shall return the amount of memory needed to store the `printf` style formatted string given by `format` and the arguments in `...`. **]**
