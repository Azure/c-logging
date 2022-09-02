#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(log_context_property_type_struct_ut
    log_context_property_type_struct_ut.c
    log_context_property_type_struct_mocked.c
)

include_directories(../../src)
target_link_libraries(log_context_property_type_struct_ut c_logging_v2)
add_test(NAME log_context_property_type_struct_ut COMMAND log_context_property_type_struct_ut)
set_target_properties(log_context_property_type_struct_ut PROPERTIES FOLDER "tests/c_logging_v2")
