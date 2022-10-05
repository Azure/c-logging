# `c-logging` requirements

## Basic functionality

`c-logging` shall expose APIs allowing the user to log events that can be used for observing the software activity, debugging etc.

The exposed API shall allow creating contexts both dynamically and on the stack and using those contexts for logging.

The API shall allow easy formatting of values to be placed in the contexts by using printf-like formatting.

## Exposed API

```
#define LOG_LEVEL_VALUES \
    LOG_LEVEL_CRITICAL, \
    LOG_LEVEL_ERROR, \
    LOG_LEVEL_WARNING, \
    LOG_LEVEL_INFO, \
    LOG_LEVEL_VERBOSE

MU_DEFINE_ENUM_WITHOUT_INVALID(LOG_LEVEL, LOG_LEVEL_VALUES);

typedef struct LOG_CONTEXT_TAG* LOG_CONTEXT_HANDLE;

void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...);

#define LOGGER_LOG(log_level, log_context, format, ...) \
    logger_log(log_level, log_context, __FILE__, __FUNCTION__, __LINE__, format MU_IFCOMMALOGIC(MU_COUNT_ARG(__VA_ARGS__)) __VA_ARGS__)

#define LOGGER_LOG_EX(log_level, ...) \
    ...

// macro that can be used to create a dynamically allocated context
#define LOG_CONTEXT_CREATE(dest_log_context, parent_context, ...) \
    ...

// destroy a dynamically allocated context
#define LOG_CONTEXT_DESTROY(log_context_handle) \
    ...

// macro that can be used to create a context on the stack.
// stack allocated contexts do not require any destructor
#define LOG_CONTEXT_LOCAL_DEFINE(log_context, parent_context, ...) \
    ...

// when a user wants to specify the log context type name, the user can use this macro
// otherwise, the default is to use the context variable name as type name
// LOG_CONTEXT_TYPE_NAME(log_context_type_name)

// when a user wants to specify properties for a context, they can use LOG_CONTEXT_PROPERTY
// LOG_CONTEXT_PROPERTY(property_type, property_name, property_value)

// when a user wants to specify string fields in a context using printf-style formatting, they can use LOG_CONTEXT_STRING_PROPERTY
// LOG_CONTEXT_STRING_PROPERTY(property_name, property_conversion_specifier, property_value)

// when a user wants specify a string property with the property name message
// LOG_MESSAGE(message)
```

Various example usages (note that `LOG_CONTEXT_NAME` is described later in the document):

```c
void log_from_a_function(LOG_CONTEXT_HANDLE log_context)
{
    LOGGER_LOG(LOG_LEVEL_ERROR, log_context, "log from a function!");
}

void examples(void)
{
    char* prop_value = malloc(2);
    (void)strcpy(prop_value, "a");

    // allocate a context dynamically
    LOG_CONTEXT_HANDLE log_context;
    LOG_CONTEXT_CREATE(log_context, NULL, LOG_CONTEXT_STRING_PROPERTY(property_name, "%s", MU_P_OR_NULL(prop_value)));

    // free the string, showing that a copy is made in the context
    free(prop_value);

    LOGGER_LOG(LOG_LEVEL_CRITICAL, log_context, "some critical error with context");

    LOG_CONTEXT_DESTROY(log_context);

    // chained allocated context
    LOG_CONTEXT_HANDLE context_1;
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_NAME(context_1), LOG_CONTEXT_STRING_PROPERTY(name, "%s", "haga"));
    LOG_CONTEXT_HANDLE context_2;
    LOG_CONTEXT_CREATE(context_2, context_1, LOG_CONTEXT_NAME(context_2), LOG_CONTEXT_STRING_PROPERTY(last name, "%s%s", "ua", "ga"), LOG_CONTEXT_PROPERTY(int32_t, age, 42));
    
    LOGGER_LOG(LOG_LEVEL_ERROR, context_1, "value of some_var = %d", 42);
    LOGGER_LOG(LOG_LEVEL_WARNING, context_2, "some other string value is %s", "mumu");
    
    LOG_CONTEXT_DESTROY(context_1);

    log_from_a_function(context_2);

    // stack allocated context
    LOG_CONTEXT_LOCAL_DEFINE(local_context, context_2, LOG_CONTEXT_STRING_PROPERTY(the_knights_that_say, "%s", "Nee!"));

    LOGGER_LOG(LOG_LEVEL_INFO, &local_context, "log info with local context");
    
    // local context chaining another local context
    LOG_CONTEXT_LOCAL_DEFINE(local_context_2, &local_context, LOG_CONTEXT_STRING_PROPERTY(other_knights_that_say, "%s", "Moo!"));
    
    LOGGER_LOG(LOG_LEVEL_VERBOSE, &local_context_2, "log with nee and moo");

    log_from_a_function(&local_context_2);
    
    LOG_CONTEXT_DESTROY(context_2);

    LOGGER_LOG_EX(LOG_LEVEL_ERROR,
        LOG_CONTEXT_STRING_PROPERTY(name, "%s%s", "go", "gu"),
        LOG_CONTEXT_PROPERTY(int32_t, age, 42));

    LOGGER_LOG_EX(LOG_LEVEL_ERROR,
        LOG_CONTEXT_STRING_PROPERTY(name, "%s%s", "go", "gu"),
        LOG_MESSAGE("some message here with an integer = %d", 42));
}
```

The above example produces:

```
LOG_LEVEL_CRITICAL Time:Mon Aug 15 15:14:03 2022 File:G:\w\c-logging\v2\tests\logger_int\main.c:89 Func:main { property_name=a } some critical error with context
LOG_LEVEL_ERROR Time:Mon Aug 15 15:14:03 2022 File:G:\w\c-logging\v2\tests\logger_int\main.c:102 Func:main context_1={ name=haga } value of some_var = 42
LOG_LEVEL_WARNING Time:Mon Aug 15 15:14:03 2022 File:G:\w\c-logging\v2\tests\logger_int\main.c:103 Func:main context_2={ context_1={ name=haga } last name=uaga age=42 } some other string value is mumu
LOG_LEVEL_ERROR Time:Mon Aug 15 15:14:03 2022 File:G:\w\c-logging\v2\tests\logger_int\main.c:16 Func:log_from_a_function context_2={ context_1={ name=haga } last name=uaga age=42 } log from a function!
LOG_LEVEL_INFO Time:Mon Aug 15 15:14:03 2022 File:G:\w\c-logging\v2\tests\logger_int\main.c:112 Func:main { context_2={ context_1={ name=haga } last name=uaga age=42 } the_knights_that_say=Nee! } log info with local context
LOG_LEVEL_VERBOSE Time:Mon Aug 15 15:14:03 2022 File:G:\w\c-logging\v2\tests\logger_int\main.c:117 Func:main { { context_2={ context_1={ name=haga } last name=uaga age=42 } the_knights_that_say=Nee! } other_knights_that_say=Moo! } log with nee and moo
LOG_LEVEL_ERROR Time:Mon Aug 15 15:14:03 2022 File:G:\w\c-logging\v2\tests\logger_int\main.c:16 Func:log_from_a_function { { context_2={ context_1={ name=haga } last name=uaga age=42 } the_knights_that_say=Nee! } other_knights_that_say=Moo! } log from a function!
LOG_LEVEL_ERROR Time:Mon Aug 15 15:14:03 2022 File:G:\w\c-logging\v2\tests\logger_int\main.c:125 Func:main { name=gogu age=42 }
LOG_LEVEL_ERROR Time:Mon Aug 15 15:14:03 2022 File:G:\w\c-logging\v2\tests\logger_int\main.c:129 Func:main { name=gogu message=some message here with an integer = 42 }
```

## Log levels

The following log levels shall be exposed by the library:

- `CRITICAL`

- `WARNING`

- `ERROR`

- `INFO`

- `VERBOSE`

## Sinks

Multiple sinks shall be supported. Setting up globally which sinks to be used shall be supported (so that they do not have to be specified every time a `c_logging_log` call is made).

Defining the sinks to use shall be done by defining a NULL terminated array, where each entry shall be a pointer to a structure that represents the sink interface.

Example:

```c
const LOG_SINK* log_sinks[] = { &console_log_sink, &etw_log_sink, NULL };
```

### ETW sink

An `ETW` sink shall be supported so that events are emitted via TraceLogging.

For the `ETW` sink there should be a posibility of specifying the provider GUID to use.

### console sink

A console sink shall be supported so that events are printed to the console using `printf`.

The console sink shall have support for multithreading so that the output is not intermingled.

## API

### logger_log

```c
void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...);
```

`log_context` may be `NULL`. If `log_context` is `NULL`, no context information shall be added to the log line.

`format` shall be a `printf`-like format string.

`...` shall be the variable number of arguments required by `format`.

### LOGGER_LOG

```c
#define LOGGER_LOG(log_level, log_context, format, ...) \
    ...
```

`LOGGER_LOG` is a helper function that automatically adds the file, function and line number information to the log line.

It calls `logger_log` under the hood with the arguments passed by the user while adding the file, function and line number information.

## Contextual logging

For each log line the library shall support specifying a context to be added to the log line.

The context shall allow defining property-value pairs that are added to the event when the event is emitted.

The properties are evaluated at context construction definition time (be that dynamically allocated or stack context).

Once the context is built, the properties cannot be changed in the context.

### Local stack context creation

The library shall support defining a context using only stack memory, not requiring any memory allocation:

```c
#define LOG_CONTEXT_LOCAL_DEFINE(log_context, parent_context, ...) \
```

where `...` is a list of context fields defined using `LOG_CONTEXT_PROPERTY` or `LOG_CONTEXT_STRING_PROPERTY` entries.

Example:

```c
    LOG_CONTEXT_LOCAL_DEFINE(log_context, NULL, LOG_CONTEXT_STRING_PROPERTY(property_name, "%s", MU_P_OR_NULL(prop_value)));
```

Bonus:

An ideal implementation for the stack allocated contexts will do as little work as possible at the definition of the local context.

However the evaluation of the context property builder macros has to be done at the time of context construction (where `LOG_CONTEXT_LOCAL_DEFINE` is used).

```c
void a(LOG_CONTEXT_HANDLE log_context, const char* correlation_id)
{
    LOG_CONTEXT_LOCAL_DEFINE(local_context, NULL, LOG_CONTEXT_STRING_PROPERTY(correlation_id, "%s", correlation_id));

    if (malloc(42) == NULL)
    {
        LOGGER_LOG(LOG_LEVEL_ERROR, &local_context, "malloc failed");
    }
}
```

Note: When logging with local contexts, the context passed to `LOGGER_LOG` has to be passed using the address of the context (&).

If too many properties are passed when defining local contexts, the error shall be indicated by calling a log internal error report function.

### LOG_ABORT_ON_ERROR

The internal error report function shall abort the execution if `LOG_ABORT_ON_ERROR` is defined so that such error cases can easily be noticed during development.

### Dynamically allocated context creation

The library shall support creating a dynamically allocated context.

```c
#define LOG_CONTEXT_CREATE(log_context_handle, parent_context, ...) \

#define LOG_CONTEXT_CREATE(log_context_handle) \
```

where `...` is a list of context fields defined using `LOG_CONTEXT_STRING_PROPERTY`.

Example:

```c
    LOG_CONTEXT_CREATE(dynamically_allocated_log_context, NULL, LOG_CONTEXT_STRING_PROPERTY(property_name, "%s", MU_P_OR_NULL(prop_value)));
    LOG_CONTEXT_DESTROY(dynamically_allocated_log_context);
```

### Context chaining

It shall be supported to chain contexts (define a context or create a context dynamically while specifying a parent context to inherit the information from).

### LOG_CONTEXT_PROPERTY

`LOG_CONTEXT_PROPERTY` is a macro that allows defining non-string properties by specifying a property type, name and a value.

```c
#define LOG_CONTEXT_PROPERTY(property_type, property_name, property_value) \
    ...
```

The following types shall be supported out of the box:

- ascii_char_ptr
- wchar_t_ptr
- int8_t
- uint8_t
- int16_t
- uint16_t
- int32_t
- uint32_t
- int64_t
- uint64_t

Note: more types will be added as needed.

### LOG_CONTEXT_STRING_PROPERTY

`LOG_CONTEXT_STRING_PROPERTY` is a macro that allows defining context properties by formatting the value as a string with `printf`-like formatting.

```c
#define LOG_CONTEXT_STRING_PROPERTY(property_name, property_format_specifier, property_value) \
    ...
```

### LOG_CONTEXT_NAME

```
#define LOG_CONTEXT_NAME(context_name) \
```

`LOG_CONTEXT_NAME` shall allow specifying the context name to be used either when printing logs to console or when ETW events are published.

By default if not specified, the context name shall be an empty string.

Example:

```c
    LOG_CONTEXT_HANDLE context_1;
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_NAME(xxx), LOG_CONTEXT_STRING_PROPERTY(name, "%s", "haga"));

    LOGGER_LOG(LOG_LEVEL_ERROR, context_1, "log error with custom context name");
```

This shall output:

```
LOG_LEVEL_ERROR Time:Mon Aug 15 11:16:09 2022 File:G:\w\c-logging\v2\tests\logger_int\main.c:102 Func:main xxx={ name=haga  }  log error with custom context name
```

### Extensions

A set of extensions properties shall be available in order to ease writing logging code.
Examples of what would fall in the extensions category are:

- syntactic sugar for a message property
- `GetLastError` on Windows
- `HRESULT` on Windows
- `errno`

## LOGGER_LOG_EX

```c
#define LOGGER_LOG_EX(log_level, ...) \
    ...
```

`LOGGER_LOG_EX` allows logging directly with structured data without explicitly creating a context.

`LOGGER_LOG_EX` creates a stack context under the hood and logs the data of that context.

Example:

```c
LOGGER_LOG_EX(LOG_LEVEL_ERROR, 
    LOG_CONTEXT_STRING_PROPERTY(name, "%s%s", "go", "gu"),
    LOG_CONTEXT_PROPERTY(int32_t, age, 42));
```

This would output:

```
LOG_LEVEL_ERROR Time:Mon Aug 15 11:15:35 2022 File:G:\w\c-logging\v2\tests\logger_int\main.c:125 Func:main { name=gogu age=42 }
```

## LOG_MESSAGE

`LOG_MESSAGE` is a macro that is syntactic sugar to wrap `LOG_CONTEXT_STRING_PROPERTY` in order to output a `message` property, while typing less.

Example:

```c
LOGGER_LOG_EX(LOG_LEVEL_ERROR, 
    LOG_CONTEXT_STRING_PROPERTY(int32_t, an_integer, 42),
    LOG_MESSAGE("hello world"));
```
