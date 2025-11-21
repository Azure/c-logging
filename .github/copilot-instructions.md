# Azure C Logging Framework - Copilot Instructions

## Project Overview
This c-logging framework is designed to be a foundational component for Azure C libraries, providing consistent, high-performance logging across all Azure services. When using this library, prioritize correctness, thread safety, and performance optimization for production scenarios while maintaining comprehensive logging for debugging and observability.

## External Dependencies and Standards
All code must follow the comprehensive coding standards defined in #file:../deps/c-build-tools/.github/general_coding_instructions.md. For detailed patterns and conventions, refer to dependency-specific instructions:

- **Build Infrastructure**: #file:../deps/c-build-tools/.github/copilot-instructions.md
- **Macro Utilities**: #file:../deps/macro-utils-c/.github/copilot-instructions.md

## c-build-tools Integration

This project follows Azure C library standards established in the `c-build-tools` dependency:

### Build System Standards
- **Pipeline Templates**: Uses `c-build-tools` pipeline templates for CI/CD (`build/devops_gated.yml` references `/pipeline_templates/build_all_flavors.yml@c_build_tools`)
- **CMake Conventions**: Follows Azure C library CMake patterns including dependency management and target configuration
- **Testing Integration**: Supports standard Azure C library test execution patterns with `run_unittests`, `run_int_tests`, and `run_perf_tests` options

### Quality Assurance
- **Traceability**: All code must have requirements traceability using `SRS_MODULE_##_###` format
- **Include What You Use (IWYU)**: Build pipeline includes header dependency validation
- **CodeQL**: Security analysis through c-build-tools pipeline integration
- **Cross-Platform**: Validation on both Windows and Linux platforms

**Note**: When contributing to this project, ensure your changes align with both c-logging-specific patterns documented above and the broader Azure C library standards maintained in the `c-build-tools` repository.

````

## Key Architecture Components

### Core Technology Stack
- **Language**: C99 standard with cross-platform compatibility
- **Build System**: CMake with Azure C library conventions following `c-build-tools` standards
- **Dependencies**: macro-utils-c for metaprogramming utilities  
- **Testing**: Custom mock framework (not umock-c), ctest for test execution
- **Platforms**: Windows (ETW support) and Linux
- **Tooling**: Uses `c-build-tools` pipeline templates and build standards

### Code Organization Principles
- **v2 Architecture**: Current implementation in `v2/` directory (legacy v1 deprecated)
- **Separation of Concerns**: Core library (`c_logging_v2_core`) vs configured library (`c_logging_v2`)
- **Platform Abstraction**: Conditional compilation for Windows-specific features (ETW, HRESULT, GetLastError)
- **Zero-Allocation Design**: Stack-based contexts avoid heap allocation in hot paths

### Major Components
1. **v2/src/logger.c** - Core logging engine and sink management
2. **v2/src/log_context.c** - Hierarchical context system with property support
3. **v2/src/log_sink_console.c** - Thread-safe console output sink
4. **v2/src/log_sink_etw.c** - Windows ETW (Event Tracing) sink
5. **v2/src/log_sink_callback.c** - Custom callback sink for user-defined outputs
6. **v2/inc/c_logging/** - Public API headers

## Development Workflow

### Environment Setup
- **Standard Azure C Library Structure**: Follow deps/ pattern for dependencies
- **CMake Integration**: Use standard `add_subdirectory(deps/c-logging)` pattern
- **Build Directory**: Generate CMake files in project's cmake/ directory
- **Dependencies**: Ensure macro-utils-c is available in deps/ folder

### Build Workflow
**Standard Development Build Process:**
```powershell
# 1. Set up dependencies (if not already present)
git submodule update --init --recursive

# 2. Generate CMake files
cmake -S . -B cmake -G "Visual Studio 17 2022" -A x64

# 3. Build with logging options
cmake --build cmake --config Debug -Dlog_sink_console=ON -Dlog_sink_etw=ON
```

**Directory Structure:**
- `v2/` - Main library implementation (current version)
- `v2/inc/c_logging/` - Public API headers
- `v2/src/` - Implementation source files
- `v2/tests/` - Unit and integration tests
- `v2/devdoc/` - Requirements documentation
- `deps/` - Git submodules for dependencies

### Key Build Options
```cmake
-Dlog_abort_on_error=ON|OFF     # Abort on internal errors (development)
-Dlog_sink_console=ON|OFF       # Enable console output sink (default ON)
-Dlog_sink_callback=ON|OFF      # Enable custom callback sink (default OFF)
-Dlog_sink_etw=ON|OFF          # Enable Windows ETW sink (default OFF)
-Dlog_sink_etw_provider_guid="GUID"  # Custom ETW provider GUID
-Drun_unittests=ON|OFF         # Build and run unit tests
-Drun_int_tests=ON|OFF         # Build and run integration tests
```

### Testing Strategy
- **Unit Tests**: Per-component testing using custom mock framework (not umock-c)
- **Integration Tests**: End-to-end logging scenarios with multiple sinks
- **Performance Tests**: Latency and throughput benchmarking
- **Cross-Platform Tests**: Verify functionality on Windows and Linux
- **Build Pipeline**: Uses `c-build-tools` templates for CI/CD automation

### Custom Mock Framework Patterns
**Function Declaration**: Use standard C functions for testability:
```c
int log_sink_init(LOG_SINK_HANDLE handle);
```

**Test Setup**: Define mock call types and structures:
```c
#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_log_sink1_init, \
    MOCK_CALL_TYPE_log_sink1_log

typedef struct log_sink1_log_CALL_TAG
{
    LOG_LEVEL captured_log_level;
    LOG_CONTEXT_HANDLE captured_log_context;
    char captured_file[MAX_FILE_STRING_LENGTH];
    char captured_message[MAX_MESSAGE_STRING_LENGTH];
} log_sink1_log_CALL;
```

**Expected Calls**: Verify behavior through custom mock validation:
```c
// Verify call was made with expected parameters
ASSERT_ARE_EQUAL(MOCK_CALL_TYPE, MOCK_CALL_TYPE_log_sink1_log, mock_calls[0].mock_call_type);
ASSERT_ARE_EQUAL(LOG_LEVEL, LOG_LEVEL_ERROR, mock_calls[0].log_sink1_log_call.captured_log_level);
```

**Context Validation**: Verify context properties in tests:
```c
// Verify context contains expected properties
ASSERT_ARE_EQUAL(size_t, 2, test_context.property_count);
ASSERT_IS_NOT_NULL(test_context.property_value_pairs);
```

## API Design and Usage Patterns

### Basic Logging
```c
#include "c_logging/logger.h"

// Simple logging without context
LOGGER_LOG(LOG_LEVEL_ERROR, NULL, "Operation failed with code %d", error_code);

// Logging with pre-created context
LOGGER_LOG(LOG_LEVEL_INFO, my_context, "User %s logged in successfully", username);
```

### Context Creation Patterns
```c
// Stack-allocated context (preferred for performance - zero allocation)
LOG_CONTEXT_LOCAL_DEFINE(local_context, parent_context,
    LOG_CONTEXT_STRING_PROPERTY(operation, "%s", "database_query"),
    LOG_CONTEXT_PROPERTY(int32_t, query_id, 12345),
    LOG_CONTEXT_PROPERTY(uint64_t, duration_ms, execution_time));

// Use stack context (note the & operator)
LOGGER_LOG(LOG_LEVEL_INFO, &local_context, "Query executed successfully");

// Dynamic context (use when context outlives current scope)
LOG_CONTEXT_HANDLE dynamic_context;
if (LOG_CONTEXT_CREATE(dynamic_context, NULL,
    LOG_CONTEXT_STRING_PROPERTY(user_id, "%s", user->id),
    LOG_CONTEXT_PROPERTY(int32_t, session_id, session->id)) == 0)
{
    LOGGER_LOG(LOG_LEVEL_WARNING, dynamic_context, "Session timeout warning");
    LOG_CONTEXT_DESTROY(dynamic_context);
}
```

### Extended Logging with LOGGER_LOG_EX
```c
// Direct structured logging without explicit context creation
LOGGER_LOG_EX(LOG_LEVEL_ERROR,
    LOG_CONTEXT_STRING_PROPERTY(component, "%s", "network_client"),
    LOG_CONTEXT_PROPERTY(int32_t, retry_count, 3),
    LOG_CONTEXT_PROPERTY(uint32_t, error_code, GetLastError()),
    LOG_MESSAGE("Connection failed after %d retries", max_retries));
```

### Context Chaining and Hierarchies
```c
// Create parent context
LOG_CONTEXT_HANDLE request_context;
LOG_CONTEXT_CREATE(request_context, NULL,
    LOG_CONTEXT_NAME(request),
    LOG_CONTEXT_STRING_PROPERTY(request_id, "%s", request_id),
    LOG_CONTEXT_PROPERTY(uint64_t, timestamp, get_timestamp()));

// Create child context inheriting from parent
LOG_CONTEXT_LOCAL_DEFINE(operation_context, request_context,
    LOG_CONTEXT_NAME(database_operation),
    LOG_CONTEXT_STRING_PROPERTY(table_name, "%s", "users"),
    LOG_CONTEXT_PROPERTY(int32_t, record_count, count));

// Log will include both request and operation context data
LOGGER_LOG(LOG_LEVEL_INFO, &operation_context, "Database operation completed");

LOG_CONTEXT_DESTROY(request_context);
```

## Platform-Specific Features

### Windows-Specific Capabilities
- **ETW (Event Tracing for Windows)**: High-performance event tracing to Windows Event Log
- **HRESULT Logging**: Built-in support for Windows error codes via `log_hresult.h`
- **GetLastError Integration**: Automatic Windows error capture via `log_lasterror.h`
- **Stack Tracing**: Debug stack capture through `get_thread_stack.h`
- **Format Message**: Windows message formatting via `format_message_no_newline.h`

### Cross-Platform Code Patterns
```c
// Platform-specific error logging
#ifdef WIN32
    #include "c_logging/log_lasterror.h"
    #include "c_logging/log_hresult.h"
    
    // Log Windows-specific errors
    LOG_LASTERROR("File operation failed");
    LOG_HRESULT(hr, "COM operation failed");
#else
    #include "c_logging/log_errno.h"
    
    // Log POSIX errors
    LOG_ERRNO("File operation failed");
#endif
```

### ETW Configuration
```c
// Configure ETW provider GUID (Windows only)
#ifdef USE_LOG_SINK_ETW
    // Use custom provider GUID or default Azure provider
    #ifndef LOG_SINK_ETW_PROVIDER_GUID
        #define LOG_SINK_ETW_PROVIDER_GUID (0xDAD29F36, 0x0A48, 0x4DEF, 0x9D, 0x50, 0x8E, 0xF9, 0x03, 0x6B, 0x92, 0xB4)
    #endif
#endif
```

## Sink Configuration and Management

### Multiple Sink Setup
```c
#include "c_logging/log_sink_console.h"
#include "c_logging/log_sink_callback.h"

// Configure multiple sinks
const LOG_SINK_IF* sinks[] = {
    &console_log_sink,
    &callback_log_sink,
    NULL  // NULL-terminated array
};

LOGGER_CONFIG config = {
    .log_sink_count = 2,
    .log_sinks = sinks
};

logger_set_config(config);
```

### Custom Callback Sink
```c
static void my_log_callback(void* context, LOG_LEVEL log_level, 
    LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, 
    int line_no, const char* message)
{
    // Custom logging implementation
    // Send to external logging service, write to custom file, etc.
    send_to_monitoring_service(log_level, message);
}

// Register callback
log_sink_callback_set_callback(my_log_callback, my_context);
```

### Thread-Safe Console Output
```c
// Console sink automatically handles thread safety
// Multiple threads can log simultaneously without interleaved output
LOGGER_LOG(LOG_LEVEL_INFO, NULL, "Thread %d: Operation started", thread_id);
```

## Property Type System

### Supported Property Types
```c
// Basic integer types
LOG_CONTEXT_PROPERTY(int8_t, status_code, -1);
LOG_CONTEXT_PROPERTY(uint8_t, flags, 0xFF);
LOG_CONTEXT_PROPERTY(int16_t, port, 443);
LOG_CONTEXT_PROPERTY(uint16_t, protocol, HTTP_PROTOCOL);
LOG_CONTEXT_PROPERTY(int32_t, user_id, 12345);
LOG_CONTEXT_PROPERTY(uint32_t, session_id, session->id);
LOG_CONTEXT_PROPERTY(int64_t, file_size, stat.st_size);
LOG_CONTEXT_PROPERTY(uint64_t, timestamp, get_timestamp_us());

// String types
LOG_CONTEXT_PROPERTY(ascii_char_ptr, username, user->name);
LOG_CONTEXT_PROPERTY(wchar_t_ptr, filename, wide_filename);

// Boolean type
LOG_CONTEXT_PROPERTY(bool, is_authenticated, user->authenticated);

// Formatted string properties
LOG_CONTEXT_STRING_PROPERTY(correlation_id, "%s-%d", prefix, sequence);
LOG_CONTEXT_STRING_PROPERTY(formatted_time, "%04d-%02d-%02d", year, month, day);
```

### Custom Property Type Extensions
```c
// Define new property type (in header file)
typedef struct MY_CUSTOM_TYPE_TAG
{
    int field1;
    const char* field2;
} MY_CUSTOM_TYPE;

// Implement property type interface (in source file)
static LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(my_custom_type)
{
    .get_type = my_custom_type_get_type,
    .to_string = my_custom_type_to_string,
    .copy = my_custom_type_copy,
    .free = my_custom_type_free
};

// Register the new type
LOG_CONTEXT_PROPERTY_TYPE_IF_REGISTER(MY_CUSTOM_TYPE, my_custom_type);
```

## Error Handling and Internal Error Management

### LOG_ABORT_ON_ERROR Development Mode
```c
// Enable during development to catch internal errors immediately
#ifdef LOG_ABORT_ON_ERROR
    // Any internal logging error will abort the process
    // Helps catch configuration and usage errors during development
#endif

// In production, internal errors are logged but execution continues
```

### Internal Error Reporting
```c
// Internal error handling pattern
if (validation_failed)
{
    LOG_INTERNAL_ERROR("Invalid parameter: expected non-NULL handle");
    return LOGGING_ERROR_INVALID_ARG;
}

// Context creation error handling
LOG_CONTEXT_HANDLE context;
if (LOG_CONTEXT_CREATE(context, NULL, /* properties */) != 0)
{
    // Context creation failed - handle gracefully
    LOG_INTERNAL_ERROR("Failed to create logging context");
    // Continue with NULL context or alternative approach
    context = NULL;
}
```

### Memory Allocation Patterns
```c
// Always check allocation results
void* buffer = malloc(size);
if (buffer == NULL)
{
    LOG_INTERNAL_ERROR("Memory allocation failed for size %zu", size);
    return LOGGING_ERROR_OUT_OF_MEMORY;
}

// Proper cleanup in error paths
LOG_CONTEXT_HANDLE context = NULL;
char* temp_buffer = NULL;

if (LOG_CONTEXT_CREATE(context, NULL, /* properties */) != 0)
{
    goto cleanup;
}

temp_buffer = malloc(BUFFER_SIZE);
if (temp_buffer == NULL)
{
    goto cleanup;
}

// ... normal operation ...

cleanup:
    if (temp_buffer != NULL)
    {
        free(temp_buffer);
    }
    if (context != NULL)
    {
        LOG_CONTEXT_DESTROY(context);
    }
```

## Header Inclusion Order Rules

The c-logging library follows Azure C library header inclusion conventions:

**1. Standard C Library Headers (first):**
```c
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
```

**2. Platform Headers (second):**
```c
#ifdef WIN32
#include "windows.h"
#include "evntprov.h"  // For ETW support
#endif
```

**3. Test Framework Headers (test files only):**
```c
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umocktypes_charptr.h"
```

**4. Core Infrastructure Headers (required order):**
```c
#include "macro_utils/macro_utils.h"  // ALWAYS first infrastructure header
```

**5. Dependency Headers (c-logging depends on macro-utils-c only):**
```c
// No additional dependency headers beyond macro_utils
```

**6. Project Headers (c-logging internal, last):**
```c
#include "c_logging/log_level.h"
#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/logger.h"  // Main API header, typically last
```

**Test File Pattern:**
```c
#include "c_logging/log_level.h"  // Headers that should NOT be mocked

#define ENABLE_MOCKS
#include "c_pal/gballoc_hl.h"     // Mock memory allocation
#include "c_logging/log_sink_if.h" // Mock sink interface
#undef ENABLE_MOCKS

#include "c_logging/logger.h"      // Component under test
```

## Testing Patterns and Best Practices

### Unit Test Structure
```c
// Test initialization pattern
TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error));
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(malloc, NULL);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(log_context_create, NULL);
}

// Individual test pattern
TEST_FUNCTION(logger_log_with_NULL_context_succeeds)
{
    // arrange
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(console_sink_log(IGNORED_ARG, LOG_LEVEL_INFO, NULL, IGNORED_ARG));

    // act
    logger_log(LOG_LEVEL_INFO, NULL, __FILE__, __FUNCTION__, __LINE__, "test message");

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}
```

### Context Testing Patterns
```c
// Test context property validation
TEST_FUNCTION(log_context_with_string_property_formats_correctly)
{
    // arrange
    LOG_CONTEXT_LOCAL_DEFINE(test_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(user_name, "%s", "test_user"),
        LOG_CONTEXT_PROPERTY(int32_t, user_id, 42));

    // act & assert - verify context contains expected properties
    ASSERT_IS_NOT_NULL(test_context.property_value_pairs);
    ASSERT_ARE_EQUAL(size_t, 2, test_context.property_count);
}
```

### Performance Testing
```c
// Benchmark stack context creation (should be near-zero cost)
TEST_FUNCTION(stack_context_creation_performance)
{
    // arrange
    LARGE_INTEGER start, end, frequency;
    QueryPerformanceFrequency(&frequency);
    
    // act
    QueryPerformanceCounter(&start);
    for (int i = 0; i < 10000; i++)
    {
        LOG_CONTEXT_LOCAL_DEFINE(perf_context, NULL,
            LOG_CONTEXT_PROPERTY(int32_t, iteration, i));
        (void)perf_context; // Suppress unused variable warning
    }
    QueryPerformanceCounter(&end);
    
    // assert - should be very fast (< 1ms for 10k iterations)
    double elapsed_ms = ((double)(end.QuadPart - start.QuadPart) * 1000.0) / frequency.QuadPart;
    ASSERT_IS_TRUE(elapsed_ms < 1.0);
}
```

## Requirements Documentation and Traceability

### Requirements Structure
Each module in `v2/devdoc/` follows standard Azure C library requirements format:
- `logger_requirements.md` - Core logging API requirements
- `log_context_requirements.md` - Context system requirements  
- `log_sink_console_requirements.md` - Console sink requirements
- `log_sink_etw_requirements.md` - ETW sink requirements

### Traceability Format
```c
// In requirements document:
// **SRS_LOGGER_01_001: [ logger_log shall validate all input parameters. ]**

// In source code:
/*Codes_SRS_LOGGER_01_001: [ logger_log shall validate all input parameters. ]*/
if (format == NULL)
{
    LOG_INTERNAL_ERROR("Invalid parameter: format is NULL");
    return;
}

// In test code:
// Tests_SRS_LOGGER_01_001: [ logger_log shall validate all input parameters. ]
TEST_FUNCTION(logger_log_with_NULL_format_logs_internal_error)
```

## Performance Optimization Guidelines

### Zero-Allocation Stack Contexts
```c
// Prefer stack contexts for hot paths - no malloc/free overhead
void high_frequency_function(int operation_id)
{
    LOG_CONTEXT_LOCAL_DEFINE(operation_context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, operation_id, operation_id),
        LOG_CONTEXT_PROPERTY(uint64_t, timestamp, get_timestamp()));
    
    // This creates no heap allocations
    LOGGER_LOG(LOG_LEVEL_VERBOSE, &operation_context, "Operation started");
}
```

### Conditional Logging
```c
// Avoid expensive operations when logging level is disabled
if (logger_is_level_enabled(LOG_LEVEL_VERBOSE))
{
    char* expensive_debug_info = generate_debug_string();
    LOGGER_LOG_EX(LOG_LEVEL_VERBOSE,
        LOG_CONTEXT_STRING_PROPERTY(debug_info, "%s", expensive_debug_info));
    free(expensive_debug_info);
}
```

### Sink Performance Considerations
```c
// ETW sink is fastest for high-volume logging on Windows
// Console sink has thread synchronization overhead
// Callback sink performance depends on user implementation

// For high-performance scenarios, prefer ETW on Windows
#ifdef WIN32
    if (is_high_performance_mode)
    {
        const LOG_SINK_IF* high_perf_sinks[] = { &etw_log_sink, NULL };
        logger_set_config((LOGGER_CONFIG){ .log_sink_count = 1, .log_sinks = high_perf_sinks });
    }
#endif
```

## Common Patterns and Anti-Patterns

### Recommended Patterns
```c
// ✅ Use stack contexts for short-lived logging
LOG_CONTEXT_LOCAL_DEFINE(local_ctx, NULL, LOG_CONTEXT_PROPERTY(int32_t, id, 123));
LOGGER_LOG(LOG_LEVEL_INFO, &local_ctx, "Operation completed");

// ✅ Check context creation results for dynamic contexts
LOG_CONTEXT_HANDLE ctx;
if (LOG_CONTEXT_CREATE(ctx, NULL, /* properties */) == 0)
{
    LOGGER_LOG(LOG_LEVEL_INFO, ctx, "Message");
    LOG_CONTEXT_DESTROY(ctx);
}

// ✅ Use appropriate log levels
LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "System is shutting down");   // Fatal errors
LOGGER_LOG(LOG_LEVEL_ERROR, NULL, "Operation failed");             // Recoverable errors  
LOGGER_LOG(LOG_LEVEL_WARNING, NULL, "Retrying operation");         // Potential issues
LOGGER_LOG(LOG_LEVEL_INFO, NULL, "User logged in");               // Normal events
LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, "Detailed debug info");       // Debug information
```

### Anti-Patterns to Avoid
```c
// ❌ Don't forget & operator with stack contexts
LOG_CONTEXT_LOCAL_DEFINE(ctx, NULL, /* properties */);
LOGGER_LOG(LOG_LEVEL_INFO, ctx, "Wrong!");      // Missing &
LOGGER_LOG(LOG_LEVEL_INFO, &ctx, "Correct!");   // Correct

// ❌ Don't leak dynamic contexts
LOG_CONTEXT_HANDLE ctx;
LOG_CONTEXT_CREATE(ctx, NULL, /* properties */);
LOGGER_LOG(LOG_LEVEL_INFO, ctx, "Message");
// Missing LOG_CONTEXT_DESTROY(ctx); - MEMORY LEAK!

// ❌ Don't use expensive string operations in log messages
LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, "Data: %s", 
    expensive_json_serialize(large_object));  // Avoid!

// ✅ Use conditional logging instead
if (logger_is_level_enabled(LOG_LEVEL_VERBOSE))
{
    char* json = expensive_json_serialize(large_object);
    LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, "Data: %s", json);
    free(json);
}
```

## Integration Guidelines

### Adding c-logging to Existing Projects
```cmake
# In your CMakeLists.txt
if ((NOT TARGET c_logging) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/c-logging/CMakeLists.txt))
    add_subdirectory(deps/c-logging)
endif()

# Link to your target
target_link_libraries(your_component c_logging_v2)
```

### Library Initialization
```c
#include "c_logging/logger.h"

int main()
{
    // Initialize logging system
    if (logger_init() != 0)
    {
        fprintf(stderr, "Failed to initialize logging system\n");
        return -1;
    }

    // Your application code here
    LOGGER_LOG(LOG_LEVEL_INFO, NULL, "Application started");

    // Cleanup on exit
    logger_deinit();
    return 0;
}
```

This c-logging framework is designed to be a foundational component for Azure C libraries, providing consistent, high-performance logging across all Azure services. When using this library, prioritize correctness, thread safety, and performance optimization for production scenarios while maintaining comprehensive logging for debugging and observability.

## External Dependencies
For build/test/pipeline conventions, refer to #file:../deps/c-build-tools/.github/copilot-instructions.md. This project inherits all build infrastructure from c-build-tools.

**IMPORTANT**: All code changes must follow the comprehensive coding standards defined in #file:../deps/c-build-tools/.github/general_coding_instructions.md, including:
- Function naming conventions (snake_case, module prefixes, internal function patterns)
- Parameter validation rules and error handling patterns  
- Variable naming and result variable conventions
- Header inclusion order and memory management requirements
- Requirements traceability system (SRS/Codes_SRS/Tests_SRS patterns)
- Async callback patterns and goto usage rules
- Indentation, formatting, and code structure guidelines
