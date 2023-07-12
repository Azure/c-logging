# `log_context_property_type_wchar_t_ptr` requirements

`log_context_property_type_wchar_t_ptr` implements the code needed for handling a wchar_t string.

## Exposed API

```
int LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(void* dst_value, size_t count, const wchar_t* format, ...);
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)(const wchar_t* format, ...);

extern const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr);

```

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(const void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string` produces the string representation of a `wchar_t_ptr`.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_001: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string` shall fail and return a negative value.  **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_002: [** If `buffer` is `NULL` and `buffer_length` is greater than 0, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_003: [** If `buffer` is `NULL` and `buffer_length` is 0, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string` shall return the length of the `wchar_t` string pointed to by `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_004: [** Otherwise, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string` shall copy the wchar_t string pointed to by `property_value` to `buffer` by using `wcstombs` with `buffer`, `buffer_length` and pass in the values list the `const wchar_t*` value pointed to by `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_005: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_006: [** If any error is encountered (truncation is not an error), `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string` shall fail and return a negative value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, const void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy` copies the `wchar_t_ptr` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_007: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_008: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_009: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy` shall copy the entire string (including the null terminator) from `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_010: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).free

```c
typedef void (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_011: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_012: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_wchar_t_ptr`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(void* dst_value, size_t count, const wchar_t* format, ...);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)` initializes the memory at `dst_value` with the `wprintf` style formatted string given by `format` and the arguments in `...`.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_013: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_014: [** If `format` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_015: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)` shall initialize by calling `vswprintf` the memory at `dst_value` with the `wprintf` style formatted wchar_t string given by `format` and the arguments in `...`. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_016: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)` shall succeed and return 0. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_017: [** If formatting the string fails, `LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)` shall fail and return a non-zero value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)(const wchar_t* format, ...);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)` returns the amount of memory needed in number of wide-characters to store the `printf` style formatted string given by `format` and the arguments in `...`.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_018: [** If `vswprintf` fails, `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)` shall return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_019: [** Otherwise, on success, `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)` shall return the amount of memory needed in number of wide-chracters to store the `wprintf` style formatted `wchar_t` string given by `format` and the arguments in `...`. **]**
