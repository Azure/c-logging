#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(log_sink_property_to_string_ut
    log_sink_property_to_string_ut.c
    log_sink_property_to_string_mocked.c
)

include_directories(../../src)
target_link_libraries(log_sink_property_to_string_ut c_logging_v2)
add_test(NAME log_sink_property_to_string_ut COMMAND log_sink_property_to_string_ut)
set_target_properties(log_sink_property_to_string_ut PROPERTIES FOLDER "tests/c_logging_v2")
