#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

set(c_logging_v2_c_files
    ./src/logger.c
    ./src/log_context.c
    ./src/log_context_property_value_pair.c
    ./src/log_context_property_basic_types.c
    ./src/log_context_property_type_ascii_char_ptr.c
    ./src/log_context_property_type_struct.c
    ./src/log_internal_error.c
    )

set(c_logging_v2_h_files
    ./inc/c_logging/logger.h
    ./inc/c_logging/log_context.h
    ./inc/c_logging/log_context_property_type.h
    ./inc/c_logging/log_context_property_type_if.h
    ./inc/c_logging/log_context_property_basic_types.h
    ./inc/c_logging/log_context_property_type_ascii_char_ptr.h
    ./inc/c_logging/log_context_property_type_struct.h
    ./inc/c_logging/log_context_property_value_pair.h
    ./inc/c_logging/log_internal_error.h
    ./inc/c_logging/log_level.h
    ./inc/c_logging/log_sink_if.h
    )

FILE(GLOB c_logging_v2_md_files "devdoc/*.md")
SOURCE_GROUP(devdoc FILES ${c_logging_v2_md_files})

#this is the product (a logging library)
add_library(c_logging_v2 ${c_logging_v2_c_files} ${c_logging_v2_h_files} ${c_logging_v2_md_files})

target_include_directories(c_logging_v2 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>)

if((CMAKE_GENERATOR MATCHES "Visual Studio") AND (${run_traceability}))
    #add traceability custom target
    add_custom_target(c_logging_v2_traceability ALL
        COMMAND traceabilitytool -buildcheck -e ${CMAKE_CURRENT_LIST_DIR}/deps -i ${CMAKE_CURRENT_LIST_DIR})
    add_dependencies(c_logging_v2_traceability traceabilitytool)
endif()

add_subdirectory(tests)

compileTargetAsC99(c_logging_v2)