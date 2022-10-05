# `log_context` requirements

`log_context` encapsulates storing a series of properties and their values (that are meant to be used when logging is done).

A `log_context` holds a count of properties and the backing memory to hold the properties name and values.

## Exposed API

```c
typedef struct LOG_CONTEXT_TAG* LOG_CONTEXT_HANDLE;

uint32_t log_context_get_property_value_pair_count(LOG_CONTEXT_HANDLE log_context);
const LOG_CONTEXT_PROPERTY_VALUE_PAIR* log_context_get_property_value_pairs(LOG_CONTEXT_HANDLE log_context);

#define LOG_CONTEXT_CREATE(dest_log_context, parent_context, ...) \
    ...

#define LOG_CONTEXT_DESTROY(log_context) \
    ...

#define LOG_MAX_STACK_DATA_SIZE                 4096
#define LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT 64
```

## LOG_CONTEXT_CREATE

```c
#define LOG_CONTEXT_CREATE(dest_log_context, parent_context, ...) \
    ...
```

`LOG_CONTEXT_CREATE` creates a dynamically allocated log context.

`...` is a list of `LOG_CONTEXT_PROPERTY`, `LOG_CONTEXT_STRING_PROPERTY` or `LOG_CONTEXT_NAME` entries.

**SRS_LOG_CONTEXT_01_001: [** `LOG_CONTEXT_CREATE` shall allocate memory for the log context. **]**

**SRS_LOG_CONTEXT_01_013: [** `LOG_CONTEXT_CREATE` shall store one property/value pair that with a property type of `struct` with as many fields as the total number of properties passed to `LOG_CONTEXT_CREATE`. **]**

**SRS_LOG_CONTEXT_01_012: [** The name of the `struct` property shall be the context name specified by using `LOG_CONTEXT_NAME` (if specified). **]**

**SRS_LOG_CONTEXT_01_003: [** `LOG_CONTEXT_CREATE` shall store the property types and values specified by using `LOG_CONTEXT_PROPERTY` in the context. **]**

**SRS_LOG_CONTEXT_01_014: [** If `parent_context` is non-`NULL`, the created context shall copy all the property/value pairs of `parent_context`. **]**

**SRS_LOG_CONTEXT_01_002: [** If any error occurs, `LOG_CONTEXT_CREATE` shall fail and return `NULL`. **]**

## LOG_CONTEXT_DESTROY

```c
#define LOG_CONTEXT_DESTROY(log_context) \
    ...
```

`LOG_CONTEXT_DESTROY` frees up all resources associated with `log_context`.

**SRS_LOG_CONTEXT_01_006: [** `LOG_CONTEXT_DESTROY` shall free the memory and resources associated with `log_context` that were allocated by `LOG_CONTEXT_CREATE`. **]**

## LOG_CONTEXT_PROPERTY

```c
#define LOG_CONTEXT_PROPERTY(property_type, property_name, property_value) \
    ...
```

`LOG_CONTEXT_PROPERTY` allows defining a property/value pair of a given supported type.

**SRS_LOG_CONTEXT_01_004: [** `LOG_CONTEXT_PROPERTY` shall expand to code allocating a property/value pair entry with the type `property_type` and the name `property_name`. **]**

**SRS_LOG_CONTEXT_01_005: [** `LOG_CONTEXT_PROPERTY` shall expand to code copying the value `property_value` to be the value of the property/value pair. **]**

**SRS_LOG_CONTEXT_01_019: [** If 2 properties have the same `property_name` for a context a compiler error shall be emitted. **]**

## LOG_CONTEXT_STRING_PROPERTY

```c
#define LOG_CONTEXT_STRING_PROPERTY(property_name, format, ...) \
    ...
```

`LOG_CONTEXT_STRING_PROPERTY` allows defining a property/value pair of type `ascii_char_ptr` where the value stored in the pair is constructed using `printf`-like formatting.

**SRS_LOG_CONTEXT_01_007: [** `LOG_CONTEXT_STRING_PROPERTY` shall expand to code allocating a property/value pair of type `ascii_char_ptr` and the name `property_name`. **]**

**SRS_LOG_CONTEXT_01_008: [** `LOG_CONTEXT_STRING_PROPERTY` shall expand to code that stores as value a string that is constructed using `printf`-like formatting based on `format` and all the arguments in `...`. **]**

**SRS_LOG_CONTEXT_01_026: [** If 2 properties have the same `property_name` for a context a compiler error shall be emitted. **]**

## LOG_CONTEXT_NAME

```c
#define LOG_CONTEXT_NAME(log_context_name) \
    ...
```

`LOG_CONTEXT_NAME` allows defining the name for a context.

**SRS_LOG_CONTEXT_01_009: [** `LOG_CONTEXT_NAME` shall be optional. **]**

**SRS_LOG_CONTEXT_01_010: [** If `LOG_CONTEXT_NAME` is not used the name for the context shall be empty string. **]**

**SRS_LOG_CONTEXT_01_011: [** If `LOG_CONTEXT_NAME` is specified multiple times a compiler error shall be emitted. **]**

## LOG_CONTEXT_CREATE

```c
#define LOG_CONTEXT_LOCAL_DEFINE(dest_log_context, parent_context, ...) \
    ...
```

`LOG_CONTEXT_LOCAL_DEFINE` allows defining a stack log context in a similar way to how dynamically allocated contexts are defined, but without having a heap allocation.

Note: The number of properties that can be contained in a stack context and the amount of data bytes is limited in order to not use too much of the stack.

`...` is a list of `LOG_CONTEXT_PROPERTY`, `LOG_CONTEXT_STRING_PROPERTY` or `LOG_CONTEXT_NAME` entries.

**SRS_LOG_CONTEXT_01_015: [** `LOG_CONTEXT_LOCAL_DEFINE` shall store one property/value pair that with a property type of `struct` with as many fields as the total number of properties passed to `LOG_CONTEXT_LOCAL_DEFINE` in the `...` arguments. **]**

**SRS_LOG_CONTEXT_01_017: [** The name of the `struct` property shall be the context name specified by using `LOG_CONTEXT_NAME` (if specified). **]**

**SRS_LOG_CONTEXT_01_016: [** `LOG_CONTEXT_LOCAL_DEFINE` shall store the property types and values specified by using `LOG_CONTEXT_PROPERTY` in the context. **]**

**SRS_LOG_CONTEXT_01_018: [** If `parent_context` is non-`NULL`, the created context shall copy all the property/value pairs of `parent_context`. **]**

**SRS_LOG_CONTEXT_01_024: [** If the number of properties to be stored in the log context exceeds `LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT`, an error shall be reported by calling `log_internal_error_report` and no properties shall be stored in the context. **]**

**SRS_LOG_CONTEXT_01_025: [** If the memory size needed for all properties to be stored in the context exceeds `LOG_MAX_STACK_DATA_SIZE`, an error shall be reported by calling `log_internal_error_report` and no properties shall be stored in the context. **]**

Note: No properties stored in the context means `log_context_get_property_value_pair_count` will return 0.

## log_context_get_property_value_pair_count

```c
uint32_t log_context_get_property_value_pair_count(LOG_CONTEXT_HANDLE log_context);
```

`log_context_get_property_value_pair_count` returns the number of property/value pairs stored by the context.

**SRS_LOG_CONTEXT_01_020: [** If `log_context` is `NULL`, `log_context_get_property_value_pair_count` shall return `UINT32_MAX`. **]**

**SRS_LOG_CONTEXT_01_021: [** Otherwise, `log_context_get_property_value_pair_count` shall return the number of property/value pairs stored by `log_context`. **]**

## log_context_get_property_value_pairs

```c
const LOG_CONTEXT_PROPERTY_VALUE_PAIR* log_context_get_property_value_pairs(LOG_CONTEXT_HANDLE log_context);
```

`log_context_get_property_value_pairs` returns the array of stored property/value pairs.

**SRS_LOG_CONTEXT_01_022: [** If `log_context` is `NULL`, `log_context_get_property_value_pairs` shall fail and return `NULL`. **]**

**SRS_LOG_CONTEXT_01_023: [** Otherwise, `log_context_get_property_value_pairs` shall return the array of property/value pairs stored by the context. **]**

