# `log_context_property_to_string` requirements

## Overview

`log_context_property_to_string` is a helper for creating a string from an array of `LOG_CONTEXT_PROPERTY_VALUE_PAIR`. This may be useful for a console sink or another sink that produces a single string with all of the context properties.

## Format

Individual properties are formatted as follows:

```
 name=value
```

Structure properties are formatted as follows:

```
 [struct_name=]{ field1=value, field2=value }
```

Where the `struct_name` is omitted if the structure is unnamed.

For structure properties, the structure contains a property count, which is the number of fields in the structure. Note that if a structure is nested, then the count does not include the count of properties in the nested structure(s).

For example, the following properties:

- struct name = foo, count = 3
- struct name = bar, count = 2
- struct name = baz, count = 2
- property name = a, value = 1
- property name = b, value = 2
- property name = c, value = 3
- property name = d, value = 4
- property name = e, value = 5

Will produce the following string with structure nesting:

```
 foo={ bar={ baz={ a=1, b=2 }, c=3 }, d=4, e=5 }
```

## Exposed API

```
int log_context_property_to_string(char* buffer, size_t buffer_size, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs, size_t property_value_pair_count);
```

### log_context_property_to_string

```c
int log_context_property_to_string(char* buffer, size_t buffer_size, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs, size_t property_value_pair_count);
```

`log_context_property_to_string` converts all of the property value pairs into a string for printing. Properties of type struct are parsed recursively.

**SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_027: [** If `buffer` is `NULL` then `log_context_property_to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_028: [** If `buffer_size` is 0 then `log_context_property_to_string` shall return 0. **]**

**SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_029: [** If `property_value_pairs` is `NULL` then `log_context_property_to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_030: [** If `property_value_pair_count` is 0 then `log_context_property_to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_016: [** For each property: **]**

 - **SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_017: [** If the property type is `struct` (used as a container for context properties): **]**

   - **SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_025: [** `log_context_property_to_string` shall print the `struct` property name and an opening brace. **]**

   - **SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_018: [** `log_context_property_to_string` shall obtain the number of fields in the `struct`. **]**

   - **SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_019: [** `log_context_property_to_string` shall print the next `n` properties as being the fields that are part of the `struct`. **]**

   - **SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_026: [** `log_context_property_to_string` shall print a closing brace as end of the `struct`. **]**

 - **SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_020: [** Otherwise `log_context_property_to_string` shall call `to_string` for the property and print its name and value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_021: [** `log_context_property_to_string` shall store at most `buffer_size` characters including the null terminator in `buffer` (the rest of the context shall be truncated). **]**

**SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_022: [** If any encoding error occurs during formatting of the line (i.e. if any `printf` class functions fails), `log_context_property_to_string` shall fail and return a negative value. **]**

**SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_031: [** `log_context_property_to_string` shall return the number of bytes written to the `buffer`. **]**
