# `log_context_property_boolean_type` requirements

`log_context_property_boolean_type` implements the code needed for handing boolean.

## Exposed API

```
int LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(void* dst_value, bool src_value);
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool)(void);

extern const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool);
```

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(const void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string` produces the string representation of an `bool`.

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_001: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_002: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string` shall call `snprintf` with `buffer`, `buffer_length` and format string `PRI_BOOL` and pass in the values list the `bool` value pointed to be `property_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_003: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string` shall succeed and return the result of `snprintf`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_004: [** If any error is encountered, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string` shall fail and return a negative value. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, const void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy` copies the `bool` value from the address pointed by `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_005: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_006: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_007: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy` shall copy the bytes of the `bool` value from the address pointed by `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_008: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free

```c
typedef void (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_009: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_010: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_bool`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(void* dst_value, bool src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)` copies the `src_value` value to the address pointed by `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_011: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_012: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)` shall copy the `src_value` bytes of the `bool` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_013: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool)(void);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool)` returns the number of bytes needed to hold an `bool`.

**SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_014: [** `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool)` shall return `sizeof(bool)`. **]**
