# `log_errno` requirements

`log_errno` implements a custom context property that adds to a logging context the string formatted `errno` value.

## Exposed API

```c
int log_errno_fill_property(void* buffer, int dummy)

#define LOG_LASTERROR() \
    ...
```

## log_errno_fill_property

```c
int log_errno_fill_property(void* buffer, int dummy);
```

`log_errno_fill_property` fills a property buffer with the `errno` formatted as a string. There is no check on the size of the buffer (it is by convention that the buffer has 512 bytes in size).

Note: The argument `dummy` is not used, it only exists to make expansion of macros less complex.

**SRS_LOG_ERRNO_01_002: [** If `buffer` is `NULL`, `log_errno_fill_property` shall return 512 to indicate how many bytes shall be reserved for the string formatted `errno`. **]**

**SRS_LOG_ERRNO_01_003: [** Otherwise, `log_errno_fill_property` shall obtain the `errno` value. **]**

**SRS_LOG_ERRNO_01_004: [** `log_errno_fill_property` shall call `strerror_s` with `buffer`, 512 and the `errno` value. **]**

**SRS_LOG_ERRNO_01_005: [** If `strerror_s` fails, `log_errno_fill_property` shall copy in `buffer` the string `failure in strerror_s` and return 512. **]**

**SRS_LOG_ERRNO_01_007: [** `log_errno_fill_property` shall return 512. **]**

## LOG_ERRNO

```c
#define LOG_ERRNO() \
    ...

```

`LOG_ERRNO` adds the `errno` value to the logging context. It is a wrapper for adding a property with the name `errno_value` of  type `ascii_char_ptr`.

Note: The name of the property is `errno_value` in order not to collide with the `errno` keyword of the C language.

**SRS_LOG_ERRNO_01_001: [** `LOG_ERRNO` shall expand to a `LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION` with name `errno_value`, type `ascii_char_ptr` and value function call being `log_errno_fill_property`. **]**
