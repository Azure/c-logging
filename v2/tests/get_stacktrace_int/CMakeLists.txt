#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(get_stacktrace_int
    get_stacktrace_int.c
)

target_link_libraries(get_stacktrace_int c_logging_v2)
add_test(NAME get_stacktrace_int COMMAND get_stacktrace_int)
set_target_properties(get_stacktrace_int PROPERTIES FOLDER "tests/c_logging_v2")
