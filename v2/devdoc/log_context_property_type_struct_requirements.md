# `log_context_property_type_struct` requirements

`log_context_property_type_struct` implements the code needed for handling a structure that contains several fields.

## Exposed API

```
int LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(void* dst_value, uint8_t src_value);
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(struct)();

extern const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct);

```

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(const void* property_value, char* buffer, size_t buffer_length);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string` produces an empty string.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_001: [** If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_003: [** If `buffer` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string` shall return 0 (the length of an empty string). **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_015: [** If `buffer_length` is 0, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string` shall return 0 (the length of an empty string). **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_004: [** Otherwise, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string` shall write 0 as the first element in `buffer`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, const void* src_value);
```

`LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy` copies the number of fields from `src_value` to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_005: [** If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_006: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_007: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy` shall copy the number of fields associated with the structure from `src_value` to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_008: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).free

```c
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
```

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_009: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).free` shall return. **]**

## LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).get_type

```c
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);
```

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_010: [** `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_struct`. **]**

## LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)

```c
int LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(void* dst_value, uint8_t field_count);
```

`LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)` copies the `field_count` byte to `dst_value`.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_011: [** If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)` shall fail and return a non-zero value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_012: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)` shall copy the `field_count` byte to `dst_value`. **]**

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_013: [** `LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)` shall succeed and return 0. **]**

## LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(struct)

```c
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(struct)(void);
```

`LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(struct)` returns the amount of memory needed to store the fields for a struct.

**SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_014: [** `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(struct)` shall return `sizeof(uint8_t)`. **]**
