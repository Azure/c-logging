#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#the following variables are project-wide and can be used with cmake-gui
option(log_abort_on_error "set log_abort_on_error to ON abort in case an error is encountered while using stack contexts (default is OFF)" OFF)
option(log_sink_console "Use the log console sink (send logs to console). Default is ON" ON)
option(log_sink_callback "Use the log callback sink (send logs to a custom callback function). Code must call log_sink_callback_set_callback. Default is OFF" OFF)
option(log_sink_etw "Use the TraceLogging sink. Default is OFF" OFF)

if(${log_sink_etw_provider_guid})
    #log_sink_etw_provider_guid allows overriding the provider dd to use in order to be able to separate logs for various projects
    #by default this option is set to "", if log_sink_etw_provider_guid is not set a non empty string value, the ETW sink will use the Dad provider,
    #which has been used since the beginnings of time for the MessagingStore team: DAD29F36-0A48-4DEF-9D50-8EF9036B92B4
    set(log_sink_etw_provider_guid "" CACHE STRING "Provider GUID in the form DAD29F36-0A48-4DEF-9D50-8EF9036B92B4")
endif()

macro(ConvertGuid guid result_var)
    # Split the GUID string by '-'
    string(REPLACE "-" ";" guid_list ${guid})
    list(GET guid_list 0 first_part)
    list(GET guid_list 1 second_part)
    list(GET guid_list 2 third_part)
    list(GET guid_list 3 second_to_last_part)
    list(GET guid_list 4 last_part)

    # Split the last part into two-byte chunks
    string(SUBSTRING ${second_to_last_part} 0 2 fourth_part)
    string(SUBSTRING ${second_to_last_part} 2 2 fifth_part)
    string(SUBSTRING ${last_part} 0 2 sixth_part)
    string(SUBSTRING ${last_part} 2 2 seventh_part)
    string(SUBSTRING ${last_part} 4 2 eighth_part)
    string(SUBSTRING ${last_part} 6 2 ninth_part)
    string(SUBSTRING ${last_part} 8 2 tenth_part)
    string(SUBSTRING ${last_part} 10 2 eleventh_part)

    # Convert to the desired format
    set(${result_var} "0x${first_part}, 0x${second_part}, 0x${third_part}, 0x${fourth_part}, 0x${fifth_part}, 0x${sixth_part}, 0x${seventh_part}, 0x${eighth_part}, 0x${ninth_part}, 0x${tenth_part}, 0x${eleventh_part}")
endmacro()

if(${log_abort_on_error})
    add_definitions(-DLOG_ABORT_ON_ERROR)
endif() #(${log_abort_on_error})

if(${log_sink_console})
    add_definitions(-DUSE_LOG_SINK_CONSOLE)
endif() #(${log_sink_console})

if(${log_sink_callback})
    add_definitions(-DUSE_LOG_SINK_CALLBACK)
endif() #(${log_sink_callback})

if(${log_sink_etw})
    add_definitions(-DUSE_LOG_SINK_ETW)

    if(NOT "${log_sink_etw_provider_guid}" STREQUAL "")
        message(STATUS "Setting ETW provider GUID : ${log_sink_etw_provider_guid}")

        ConvertGuid(${log_sink_etw_provider_guid} converted_log_sink_etw_provider_guid)

        add_definitions(-DLOG_SINK_ETW_PROVIDER_GUID=${converted_log_sink_etw_provider_guid})
    endif()
endif() #(${log_sink_etw})

set(c_logging_v2_h_files
    ./inc/c_logging/logger.h
    ./inc/c_logging/logger_v1_v2.h
    ./inc/c_logging/log_context.h
    ./inc/c_logging/log_context_property_type.h
    ./inc/c_logging/log_context_property_type_if.h
    ./inc/c_logging/log_context_property_basic_types.h
    ./inc/c_logging/log_context_property_bool_type.h
    ./inc/c_logging/log_context_property_to_string.h
    ./inc/c_logging/log_context_property_type_ascii_char_ptr.h
    ./inc/c_logging/log_context_property_type_struct.h
    ./inc/c_logging/log_context_property_value_pair.h
    ./inc/c_logging/log_context_property_type_wchar_t_ptr.h
    ./inc/c_logging/log_errno.h
    ./inc/c_logging/log_internal_error.h
    ./inc/c_logging/log_level.h
    ./inc/c_logging/log_sink_if.h
    ./inc/c_logging/log_sink_console.h
    ./inc/c_logging/log_sink_callback.h
    )

set(c_logging_v2_c_files
    ./src/logger.c
    ./src/log_context.c
    ./src/log_context_property_basic_types.c
    ./src/log_context_property_bool_type.c
    ./src/log_context_property_to_string.c
    ./src/log_context_property_type.c
    ./src/log_context_property_type_ascii_char_ptr.c
    ./src/log_context_property_type_struct.c
    ./src/log_context_property_type_wchar_t_ptr.c
    ./src/log_internal_error.c
    ./src/log_sink_console.c
    ./src/log_sink_callback.c
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

