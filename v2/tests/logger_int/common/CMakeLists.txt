#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

set(exeName "logger_int_console_${log_sink_console_test_value}_etw_${log_sink_etw_test_value}")

set(LOGGER_SINK_CONFIG_C_FILE logger_sink_config_console_${log_sink_console_test_value}_etw_${log_sink_etw_test_value}.c)

add_executable(${exeName}
    ${LOGGER_SINK_CONFIG_C_FILE}
    logger_int.c)

set_target_properties(${exeName} PROPERTIES FOLDER "tests/c_logging_v2")
target_link_libraries(${exeName} c_logging_v2_core)
add_test(NAME ${exeName} COMMAND ${exeName})
