#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(logging_stacktrace_ut
    logging_stacktrace_ut.c
)

target_link_libraries(logging_stacktrace_ut c_logging_v2)
add_test(NAME logging_stacktrace_ut COMMAND logging_stacktrace_ut)
set_target_properties(logging_stacktrace_ut PROPERTIES FOLDER "tests/c_logging_v2")
