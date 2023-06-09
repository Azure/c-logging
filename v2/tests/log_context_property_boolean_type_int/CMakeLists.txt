#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(log_context_property_boolean_type_int
    log_context_property_boolean_type_int.c
)

include_directories(../../src)
target_link_libraries(log_context_property_boolean_type_int c_logging_v2)
add_test(NAME log_context_property_boolean_type_int COMMAND log_context_property_boolean_type_int)
set_target_properties(log_context_property_boolean_type_int PROPERTIES FOLDER "tests/c_logging_v2")