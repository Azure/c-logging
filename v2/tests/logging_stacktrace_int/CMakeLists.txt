#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

# Exclude this test on ARM64 due to known issues with stack walking
if(NOT (CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64"))

add_executable(logging_stacktrace_int
    logging_stacktrace_int.c
)

target_link_libraries(logging_stacktrace_int c_logging_v2)
add_test(NAME logging_stacktrace_int COMMAND logging_stacktrace_int)
set_target_properties(logging_stacktrace_int PROPERTIES FOLDER "tests/c_logging_v2")

endif()
