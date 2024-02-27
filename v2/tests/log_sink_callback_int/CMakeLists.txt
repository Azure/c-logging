#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(log_sink_callback_int
    log_sink_callback_int.c
)

include_directories(../../src)
target_link_libraries(log_sink_callback_int c_logging_v2)
add_test(NAME log_sink_callback_int COMMAND log_sink_callback_int)
set_target_properties(log_sink_callback_int PROPERTIES FOLDER "tests/c_logging_v2")
