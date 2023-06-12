#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(format_message_no_newline_int
    format_message_no_newline_int.c
)

include_directories(../../src)
target_link_libraries(format_message_no_newline_int c_logging_v2)
add_test(NAME format_message_no_newline_int COMMAND format_message_no_newline_int)
set_target_properties(format_message_no_newline_int PROPERTIES FOLDER "tests/c_logging_v2")
