#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(log_stacktrace_int
    log_stacktrace_int.c
)

target_link_libraries(log_stacktrace_int c_logging_v2)
add_test(NAME log_hresult_int COMMAND log_hresult_int)
set_target_properties(log_stacktrace_int PROPERTIES FOLDER "tests/c_logging_v2")
