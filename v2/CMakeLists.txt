#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#the following variables are project-wide and can be used with cmake-gui
option(log_abort_on_error "set log_abort_on_error to ON abort in case an error is encountered while using stack contexts (default is OFF)" OFF)
option(log_sink_console "Use the log console sink (send logs to console). Default is ON" ON)
option(log_sink_etw "Use the TraceLogging sink. Default is OFF" OFF)

if(${log_abort_on_error})
    add_definitions(-DLOG_ABORT_ON_ERROR)
endif() #(${log_abort_on_error})

if(${log_sink_console})
    add_definitions(-DUSE_LOG_SINK_CONSOLE)
endif() #(${log_sink_console})

if(${log_sink_etw})
    add_definitions(-DUSE_LOG_SINK_ETW)
endif() #(${log_sink_etw})

set(c_logging_v2_h_files
    ./inc/c_logging/logger.h
    ./inc/c_logging/logger_v1_v2.h
    ./inc/c_logging/log_context.h
    ./inc/c_logging/log_context_property_type.h
    ./inc/c_logging/log_context_property_type_if.h
    ./inc/c_logging/log_context_property_basic_types.h
    ./inc/c_logging/log_context_property_bool_type.h
    ./inc/c_logging/log_context_property_type_ascii_char_ptr.h
    ./inc/c_logging/log_context_property_type_struct.h
    ./inc/c_logging/log_context_property_value_pair.h
    ./inc/c_logging/log_context_property_type_wchar_t_ptr.h
    ./inc/c_logging/log_errno.h
    ./inc/c_logging/log_internal_error.h
    ./inc/c_logging/log_level.h
    ./inc/c_logging/log_sink_if.h
    ./inc/c_logging/log_sink_console.h
    )

set(c_logging_v2_c_files
    ./src/logger.c
    ./src/log_context.c
    ./src/log_context_property_basic_types.c
    ./src/log_context_property_bool_type.c
    ./src/log_context_property_type.c
    ./src/log_context_property_type_ascii_char_ptr.c
    ./src/log_context_property_type_struct.c
    ./src/log_context_property_type_wchar_t_ptr.c
    ./src/log_internal_error.c
    ./src/log_sink_console.c
    )

if(WIN32)
set(c_logging_v2_h_files
    ${c_logging_v2_h_files}
    ./inc/c_logging/format_message_no_newline.h
    ./inc/c_logging/log_sink_etw.h
    ./inc/c_logging/log_lasterror.h
    ./inc/c_logging/log_hresult.h
    ./inc/c_logging/get_thread_stack.h
    )

set(c_logging_v2_c_files
    ${c_logging_v2_c_files}
    ./src/format_message_no_newline.c
    ./src/log_sink_etw.c
    ./src/log_lasterror.c
    ./src/log_hresult.c
    ./src/log_errno_win32.c
    ./src/get_thread_stack.c
    )
else()
set(c_logging_v2_c_files
    ${c_logging_v2_c_files}
    ./src/log_errno_linux.c
    )
endif()

FILE(GLOB c_logging_v2_md_files "devdoc/*.md")
SOURCE_GROUP(devdoc FILES ${c_logging_v2_md_files})

#this is the product (a logging library)
add_library(c_logging_v2_core ${c_logging_v2_c_files} ${c_logging_v2_h_files})
if(WIN32)
    target_link_libraries(c_logging_v2_core dbghelp) #dbghelp is needed for stack tracing
endif()

add_library(c_logging_v2 ${c_logging_v2_c_files} ${c_logging_v2_h_files} ${c_logging_v2_md_files} ./src/logger_sinks_config.c)

target_include_directories(c_logging_v2 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>)
target_include_directories(c_logging_v2_core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>)

target_link_libraries(c_logging_v2 c_logging_v2_core)

install_library_with_prefix(c_logging_v2 c_logging/v2/c_logging ${c_logging_v2_h_files})

if((CMAKE_GENERATOR MATCHES "Visual Studio") AND (${run_traceability}))
    #add traceability custom target
    add_custom_target(c_logging_v2_traceability ALL
        COMMAND traceabilitytool -buildcheck -e ${CMAKE_CURRENT_LIST_DIR}/deps -i ${CMAKE_CURRENT_LIST_DIR})
    add_dependencies(c_logging_v2_traceability traceabilitytool)
endif()

add_subdirectory(tests)

