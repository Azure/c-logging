#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(logging_stacktrace_int
    logging_stacktrace_int.c
)

target_link_libraries(logging_stacktrace_int c_logging_v2)
add_test(NAME logging_stacktrace_int COMMAND logging_stacktrace_int)
set_target_properties(logging_stacktrace_int PROPERTIES FOLDER "tests/c_logging_v2")
