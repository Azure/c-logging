#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

if (TARGET azure_c_util)
    RETURN()
endif()

project(azure_c_logging)

#the following variables are project-wide and can be used with cmake-gui
option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)
option(run_int_tests "set run_int_tests to ON to run integration tests (default is OFF)." OFF)
option(run_int_tests "set run_e2e_tests to ON to run e2e tests (default is OFF)." OFF)
option(use_cppunittest "set use_cppunittest to ON to build CppUnitTest tests on Windows (default is ON)" ON)
option(run_traceability "run traceability tool (default is ON)" ON)

if(WIN32)
    option(logerror_captures_stack_traces "LogError will make a reasonable attempt to capture the stack trace at the time of calling LogError. Only available on Windows" OFF)
    set(use_etw OFF CACHE STRING "set use_etw to OFF = No ETW logging, TRACELOGGING = Use ETW from tracelogging.h, TRACELOGGING_WITH_CONSOLE = Use ETW from tracelogging.h and call console logger. Default is OFF")
    set_property(CACHE use_etw PROPERTY STRINGS TRACELOGGING_WITH_CONSOLE TRACELOGGING OFF)
endif()

option(no_logging "disable logging (default is OFF)" OFF)

set(original_run_e2e_tests ${run_e2e_tests})
set(original_run_unittests ${run_unittests})
set(original_run_int_tests ${run_int_tests})
set(original_run_traceability ${run_traceability})

set(run_e2e_tests OFF)
set(run_unittests OFF)
set(run_int_tests OFF)
set(run_traceability OFF)

if ((NOT TARGET azure_c_build_tools) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-build-tools/CMakeLists.txt))
        add_subdirectory(deps/azure-c-build-tools)
endif()
set_default_build_options()

include(CTest)
enable_testing()


if ((NOT TARGET azure_macro_utils_c) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-macro-utils-c/CMakeLists.txt))
        add_subdirectory(deps/azure-macro-utils-c)
        include_directories(deps/azure-macro-utils-c/inc)
endif()

if ((NOT TARGET ctest) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-ctest/CMakeLists.txt))
        add_subdirectory(deps/azure-ctest)
        include_directories(deps/azure-ctest/inc)
endif()

if ((NOT TARGET testrunnerswitcher) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-testrunnerswitcher/CMakeLists.txt))
        add_subdirectory(deps/azure-c-testrunnerswitcher)
        include_directories(deps/azure-c-testrunnerswitcher/inc)
endif()

if ((NOT TARGET umock_c) AND (EXISTS ${CMAKE_CURRENT_LIST_DIR}/deps/umock-c/CMakeLists.txt))
        add_subdirectory(deps/umock-c)
        include_directories(deps/umock-c/inc)
endif()

include_directories(${MACRO_UTILS_INC_FOLDER})
include_directories(${UMOCK_C_INC_FOLDER})
include_directories(${CMAKE_CURRENT_LIST_DIR}/inc)

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
    set(LOGGING_STACKTRACE_H_FILE ./inc/azure_c_util/logging_stacktrace.h)
else()
    set(LOGGING_STACKTRACE_C_FILE)
    set(LOGGING_STACKTRACE_H_FILE)
endif()

if(${no_logging})
    add_definitions(-DNO_LOGGING)
endif()

# This function focuses on setting files which are unique to a given hardware platform.
# The choice of tlsio is not unique per-platform, and is set in the main CMakeLists.txt
if (WIN32)
    if(${use_etw} STREQUAL "OFF")
        set(XLOGGING_C_FILE ${c_util_dir}/src/xlogging.c PARENT_SCOPE)
        set(LOGGING_C_FILE ${c_util_dir}/src/consolelogger.c PARENT_SCOPE)
        set(LOGGING_H_FILE ${c_util_dir}/inc/azure_c_util/consolelogger.h PARENT_SCOPE)
    elseif(${use_etw} STREQUAL "TRACELOGGING")
        set(XLOGGING_C_FILE ${c_util_dir}/src/etwxlogging.c PARENT_SCOPE)
        set(LOGGING_C_FILE ${c_util_dir}/src/etwlogger_driver.c PARENT_SCOPE)
        set(LOGGING_H_FILE ${c_util_dir}/inc/azure_c_util/etwlogger_driver.h PARENT_SCOPE)
    elseif(${use_etw} STREQUAL "TRACELOGGING_WITH_CONSOLE")
        set(XLOGGING_C_FILE ${c_util_dir}/src/etwxlogging.c PARENT_SCOPE)
        set(LOGGING_C_FILE ${c_util_dir}/src/etwlogger_driver.c ${c_util_dir}/src/consolelogger.c PARENT_SCOPE)
        set(LOGGING_H_FILE ${c_util_dir}/inc/azure_c_util/etwlogger_driver.h ${c_util_dir}/inc/azure_c_util/consolelogger.h PARENT_SCOPE)
    endif()
endif()

set(azure_c_logging_c_files
    ${LOGGING_C_FILE}
    ${XLOGGING_C_FILE}
    ${LOGGING_STACKTRACE_C_FILE}
    )

set(azure_c_logging_h_files
    ${LOGGING_H_FILE}
    ${LOGGING_STACKTRACE_H_FILE}
    )

add_library(azure_c_logging ${azure_c_logging_c_files} ${azure_c_logging_h_files})

FILE(GLOB azure_c_logging_md_files "devdoc/*.md")
SOURCE_GROUP(devdoc FILES ${azure_c_logging_md_files})

if(MSVC)
    set(azure_c_logging_h_files ${azure_c_logging_h_files}
        ./pal/windows/refcount_os.h
    )
else()
    set(azure_c_logging_h_files ${azure_c_logging_h_files}
        ./pal/linux/refcount_os.h
    )
endif()

#this is the product (a logging library)
add_library(azure_c_logging ${azure_c_logging_c_files} ${azure_c_logging_h_files} ${azure_c_logging_md_files})

target_include_directories(azure_c_logging PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc>)

set(azure_c_util_target_libs)

if(${logerror_captures_stack_traces})
    set(azure_c_logging_target_libs ${azure_c_logging_target_libs} dbghelp)
endif()

target_link_libraries(azure_c_logging ${azure_c_logging_target_libs})

set(run_e2e_tests ${original_run_e2e_tests})
set(run_unittests ${original_run_unittests})
set(run_int_tests ${original_run_int_tests})
set(run_traceability ${original_run_traceability})

if((WIN32) AND (${run_traceability}))
    #add traceability custom target
    add_custom_target(azure_c_logging_traceability ALL
        COMMAND ${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-build-tools/traceabilitytool/traceability_tool/bin/$<CONFIG>/traceabilitytool.exe -buildcheck -e ${CMAKE_CURRENT_LIST_DIR}/deps -i ${CMAKE_CURRENT_LIST_DIR})
    add_dependencies(azure_c_logging_traceability traceabilitytool)
endif()

add_subdirectory(tests)

include(CMakePackageConfigHelpers)

compileTargetAsC99(azure_c_logging)
