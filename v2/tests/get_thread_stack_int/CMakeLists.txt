#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

# Exclude this test on ARM64 due to known issues with cross-thread stack walking
if(NOT (CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64"))

add_executable(get_thread_stack_int
    get_thread_stack_int.c
)

target_link_libraries(get_thread_stack_int c_logging_v2)
add_test(NAME get_thread_stack_int COMMAND get_thread_stack_int)
set_target_properties(get_thread_stack_int PROPERTIES FOLDER "tests/c_logging_v2")

endif()
