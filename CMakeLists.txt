#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

# canon way of using azure-c-logging from another repo is below. It assumes the using repo has placed azure-c-logging in "deps"

#if ((NOT TARGET azure_c_logging) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-logging/CMakeLists.txt))
#    add_subdirectory(deps/azure-c-logging)
#    include_directories(deps/azure-c-logging/inc)
#endif() 

if (TARGET azure_c_logging)
    RETURN()
endif()

project(azure_c_logging)

#the following variables are project-wide and can be used with cmake-gui
option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)
option(run_int_tests "set run_int_tests to ON to run integration tests (default is OFF)." OFF)
option(run_e2e_tests "set run_e2e_tests to ON to run e2e tests (default is OFF)." OFF)
option(use_cppunittest "set use_cppunittest to ON to build CppUnitTest tests on Windows (default is ON)" ON)
option(run_traceability "run traceability tool (default is ON)" ON)

if(WIN32)
    option(logerror_captures_stack_traces "LogError will make a reasonable attempt to capture the stack trace at the time of calling LogError. Only available on Windows" OFF)
    set(use_etw OFF CACHE STRING "set use_etw to OFF = No ETW logging, TRACELOGGING = Use ETW from tracelogging.h, TRACELOGGING_WITH_CONSOLE = Use ETW from tracelogging.h and call console logger. Default is OFF")
    set_property(CACHE use_etw PROPERTY STRINGS TRACELOGGING_WITH_CONSOLE TRACELOGGING OFF)
endif()

option(no_logging "disable logging (default is OFF)" OFF)

#bring in dependencies
#do not add or build any tests of the dependencies
set(original_run_e2e_tests ${run_e2e_tests})
set(original_run_int_tests ${run_int_tests})
set(original_run_perf_tests ${run_perf_tests})
set(original_run_unittests ${run_unittests})
set(original_run_traceability ${run_traceability})

set(run_e2e_tests OFF)
set(run_int_tests OFF)
set(run_perf_tests OFF)
set(run_unittests OFF)
set(run_traceability OFF)

if ((NOT TARGET c_build_tools) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/c-build-tools/CMakeLists.txt))
    add_subdirectory(deps/c-build-tools)
    set_default_build_options()
endif() 

if ((NOT TARGET azure_macro_utils_c) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-macro-utils-c/CMakeLists.txt))
    add_subdirectory(deps/azure-macro-utils-c)
    include_directories(${MACRO_UTILS_INC_FOLDER})
endif() 

set(run_e2e_tests ${original_run_e2e_tests})
set(run_int_tests ${original_run_int_tests})
set(run_perf_tests ${original_run_perf_tests})
set(run_unittests ${original_run_unittests})
set(run_traceability ${original_run_traceability})

include_directories(${UMOCK_C_INC_FOLDER})
include_directories(${CMAKE_CURRENT_LIST_DIR}/inc)

include(CTest)
enable_testing()

if(WIN32)

if(${use_etw} STREQUAL "TRACELOGGING_WITH_CONSOLE")
    add_definitions(-DUSE_TRACELOGGING)
    add_definitions(-DCALL_CONSOLE_LOGGER)
endif() #(${use_etw} STREQUAL "TRACELOGGING_WITH_CONSOLE")

if(${use_etw} STREQUAL "TRACELOGGING")
    add_definitions(-DUSE_TRACELOGGING)
endif() #(${use_etw} STREQUAL "TRACELOGGING")

endif()

if(${logerror_captures_stack_traces})
    #set a preprocessor option (LOGERROR_CAPTURES_STACK_TRACES) and add the files that provide the functionality
    add_definitions(-DLOGERROR_CAPTURES_STACK_TRACES)
    set(LOGGING_STACKTRACE_C_FILE ./src/logging_stacktrace.c)
    set(LOGGING_STACKTRACE_H_FILE ./inc/azure_c_logging/logging_stacktrace.h)
else()
    set(LOGGING_STACKTRACE_C_FILE ./src/logging_stacktrace_dummy.c)
    set(LOGGING_STACKTRACE_H_FILE ./inc/azure_c_logging/logging_stacktrace.h)
endif()

if(${no_logging})
    add_definitions(-DNO_LOGGING)
endif()

if(WIN32)
    if(${use_etw} STREQUAL "OFF")
        set(XLOGGING_C_FILE ${CMAKE_CURRENT_LIST_DIR}/src/xlogging.c CACHE INTERNAL "this is the xlogging C file(s)" FORCE)
        set(LOGGING_C_FILE ${CMAKE_CURRENT_LIST_DIR}/src/consolelogger.c CACHE INTERNAL "this is the logging C file(s)" FORCE)
        set(LOGGING_H_FILE ${CMAKE_CURRENT_LIST_DIR}/inc/azure_c_logging/consolelogger.h CACHE INTERNAL "this is the logging H file(s)" FORCE)
    elseif(${use_etw} STREQUAL "TRACELOGGING")
        set(XLOGGING_C_FILE ${CMAKE_CURRENT_LIST_DIR}/src/etwxlogging.c CACHE INTERNAL "this is the xlogging C file(s)" FORCE)
        set(LOGGING_C_FILE ${CMAKE_CURRENT_LIST_DIR}/src/etwlogger_driver.c CACHE INTERNAL "this is the logging C file(s)" FORCE)
        set(LOGGING_H_FILE ${CMAKE_CURRENT_LIST_DIR}/inc/azure_c_logging/etwlogger_driver.h CACHE INTERNAL "this is the logging H file(s)" FORCE)
    elseif(${use_etw} STREQUAL "TRACELOGGING_WITH_CONSOLE")
        set(XLOGGING_C_FILE ${CMAKE_CURRENT_LIST_DIR}/src/etwxlogging.c CACHE INTERNAL "this is the xlogging C file(s)" FORCE)
        set(LOGGING_C_FILE ${CMAKE_CURRENT_LIST_DIR}/src/etwlogger_driver.c ${CMAKE_CURRENT_LIST_DIR}/src/consolelogger.c CACHE INTERNAL "this is the logging C file(s)" FORCE)
        set(LOGGING_H_FILE ${CMAKE_CURRENT_LIST_DIR}/inc/azure_c_logging/etwlogger_driver.h ${CMAKE_CURRENT_LIST_DIR}/inc/azure_c_logging/consolelogger.h CACHE INTERNAL "this is the logging H file(s)" FORCE)
    endif()
else()
    set(XLOGGING_C_FILE ${CMAKE_CURRENT_LIST_DIR}/src/xlogging.c CACHE INTERNAL "this is the xlogging C file(s)" FORCE)
    set(LOGGING_C_FILE ${CMAKE_CURRENT_LIST_DIR}/src/consolelogger.c CACHE INTERNAL "this is the logging C file(s)" FORCE)
    set(LOGGING_H_FILE ${CMAKE_CURRENT_LIST_DIR}/inc/azure_c_logging/consolelogger.h CACHE INTERNAL "this is the logging H file(s)" FORCE)
endif()

set(azure_c_logging_c_files
    ${LOGGING_C_FILE}
    ${XLOGGING_C_FILE}
    ${LOGGING_STACKTRACE_C_FILE}
    )

set(azure_c_logging_h_files
    ${LOGGING_H_FILE}
    ${LOGGING_STACKTRACE_H_FILE}
    ./inc/azure_c_logging/xlogging.h
    )

FILE(GLOB azure_c_logging_md_files "devdoc/*.md")
SOURCE_GROUP(devdoc FILES ${azure_c_logging_md_files})

#this is the product (a logging library)
add_library(azure_c_logging ${azure_c_logging_c_files} ${azure_c_logging_h_files} ${azure_c_logging_md_files})

target_include_directories(azure_c_logging PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc>)

set(azure_c_logging_target_libs)

if(${logerror_captures_stack_traces})
    set(azure_c_logging_target_libs ${azure_c_logging_target_libs} dbghelp)
endif()

target_link_libraries(azure_c_logging ${azure_c_logging_target_libs})

if((WIN32) AND (${run_traceability}))
    #add traceability custom target
    add_custom_target(azure_c_logging_traceability ALL
        COMMAND traceabilitytool -buildcheck -e ${CMAKE_CURRENT_LIST_DIR}/deps -i ${CMAKE_CURRENT_LIST_DIR})
    add_dependencies(azure_c_logging_traceability traceabilitytool)
endif()

add_subdirectory(tests)

compileTargetAsC99(azure_c_logging)
