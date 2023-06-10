#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(format_message_no_newline_ut
    format_message_no_newline_mocked.c
    format_message_no_newline_ut.c
)

include_directories(../../src)
target_link_libraries(format_message_no_newline_ut c_logging_v2)
add_test(NAME format_message_no_newline_ut COMMAND format_message_no_newline_ut)
set_target_properties(format_message_no_newline_ut PROPERTIES FOLDER "tests/c_logging_v2")
