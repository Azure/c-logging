#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

if (TARGET azure_c_util)
    RETURN()
endif()

project(azure_c_logging)

#the following variables are project-wide and can be used with cmake-gui
option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)
option(run_int_tests "set run_int_tests to ON to integration tests (default is OFF)." OFF)
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

set(C_UTIL_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using sharedLib lib" FORCE)

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
function(set_platform_files c_util_dir)
    if(WIN32)
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
        set(INTERLOCKED_HL_C_FILE ${c_util_dir}/adapters/interlocked_hl_win32.c PARENT_SCOPE)
        set(LOCK_C_FILE ${c_util_dir}/adapters/lock_win32.c PARENT_SCOPE)
        set(PLATFORM_C_FILE ${c_util_dir}/adapters/platform_win32.c PARENT_SCOPE)
        set(THREAD_C_FILE ${c_util_dir}/adapters/threadapi_win32.c PARENT_SCOPE)
        set(TICKCOUTER_C_FILE ${c_util_dir}/adapters/tickcounter_win32.c PARENT_SCOPE)
        set(TIMER_C_FILE ${c_util_dir}/adapters/timer.c PARENT_SCOPE)
        set(UNIQUEID_C_FILE ${c_util_dir}/adapters/uniqueid_win32.c PARENT_SCOPE)
        set(STRING_UTILS_C_FILE ${c_util_dir}/adapters/string_utils.c PARENT_SCOPE)
        set(SRW_LOCK_C_FILE ${c_util_dir}/adapters/srw_lock.c PARENT_SCOPE)
    else()
        set(LOCK_C_FILE ${c_util_dir}/adapters/lock_pthreads.c PARENT_SCOPE)
        set(PLATFORM_C_FILE ${c_util_dir}/adapters/platform_linux.c PARENT_SCOPE)
        set(THREAD_C_FILE ${c_util_dir}/adapters/threadapi_pthreads.c PARENT_SCOPE)
        set(TICKCOUTER_C_FILE ${c_util_dir}/adapters/tickcounter_linux.c PARENT_SCOPE)
        set(UNIQUEID_C_FILE ${c_util_dir}/adapters/uniqueid_stub.c PARENT_SCOPE)
        set(XLOGGING_C_FILE ${c_util_dir}/src/xlogging.c PARENT_SCOPE)
        set(LOGGING_C_FILE ${c_util_dir}/src/consolelogger.c PARENT_SCOPE)
        set(LOGGING_H_FILE ${c_util_dir}/inc/azure_c_util/consolelogger.h PARENT_SCOPE)
    endif()
    
    if(WIN32 OR MACOSX OR LINUX)
        set (ENVIRONMENT_VARIABLE_C_FILE ${c_util_dir}/adapters/envvariable.c PARENT_SCOPE)
    endif()
endfunction(set_platform_files)

#Setup the platform files
set_platform_files(${CMAKE_CURRENT_LIST_DIR})

include_directories(${UMOCK_C_INC_FOLDER})

set(azure_c_util_c_files
    ./adapters/agenttime.c
    ./src/azure_base64.c
    ./src/buffer.c
    ./src/crt_abstractions.c
    ./src/constbuffer.c
    ./src/constbuffer_array.c
    ./src/constbuffer_array_batcher_nv.c
    ./src/doublylinkedlist.c
    ./src/gballoc.c
    ./src/map.c
    ./src/memory_data.c
    ./src/singlylinkedlist.c
    ./src/strings.c
    ./src/thandle_stdlib.c
    ./src/uuid.c
    ${LOCK_C_FILE}
    ${SRW_LOCK_C_FILE}
    ${TIMER_C_FILE}
    ${STRING_UTILS_C_FILE}
    ${TICKCOUTER_C_FILE}
    ${THREAD_C_FILE}
    ${UNIQUEID_C_FILE}
    ${ENVIRONMENT_VARIABLE_C_FILE}
    ${PLATFORM_C_FILE}
    ${INTERLOCKED_HL_C_FILE}
)

set(azure_c_util_h_files
    ./inc/azure_c_util/agenttime.h
    ./inc/azure_c_util/azure_base64.h
    ./inc/azure_c_util/buffer_.h
    ./inc/azure_c_util/constbuffer.h
    ./inc/azure_c_util/constbuffer_array.h
    ./inc/azure_c_util/constbuffer_array_batcher_nv.h
    ./inc/azure_c_util/crt_abstractions.h
    ./inc/azure_c_util/doublylinkedlist.h
    ./inc/azure_c_util/envvariable.h
    ./inc/azure_c_util/gballoc.h
    ./inc/azure_c_util/lock.h
    ./inc/azure_c_util/map.h
    ./inc/azure_c_util/memory_data.h
    ./inc/azure_c_util/platform.h
    ./inc/azure_c_util/refcount.h
    ./inc/azure_c_util/singlylinkedlist.h
    ./inc/azure_c_util/srw_lock.h
    ./inc/azure_c_util/string_utils.h
    ./inc/azure_c_util/strings.h
    ./inc/azure_c_util/strings_types.h
    ./inc/azure_c_util/timer.h
    ./inc/azure_c_util/tickcounter.h
    ./inc/azure_c_util/threadapi.h
    ./inc/azure_c_util/thandle.h
    ./inc/azure_c_util/thandle_stdlib.h
    ./inc/azure_c_util/uniqueid.h
    ./inc/azure_c_util/uuid.h
    ./inc/azure_c_util/xlogging.h
    ./inc/azure_c_util/interlocked_hl.h
)

set(azure_c_util_logging_c_files
    ${LOGGING_C_FILE}
    ${XLOGGING_C_FILE}
    ${LOGGING_STACKTRACE_C_FILE}
    )

set(azure_c_util_logging_h_files
    ${LOGGING_H_FILE}
    ${LOGGING_STACKTRACE_H_FILE}
    )

add_library(azure_c_util_logging ${azure_c_util_logging_c_files} ${azure_c_util_logging_h_files})

FILE(GLOB azure_c_util_md_files "devdoc/*.md")
SOURCE_GROUP(devdoc FILES ${azure_c_util_md_files})

if(MSVC)
    set(azure_c_util_h_files ${azure_c_util_h_files}
        ./pal/windows/refcount_os.h
    )
else()
    set(azure_c_util_h_files ${azure_c_util_h_files}
        ./pal/linux/refcount_os.h
    )
endif()

#this is the product (a library)
add_library(azure_c_util ${azure_c_util_c_files} ${azure_c_util_h_files} ${azure_c_util_md_files})

if(MSVC)
    target_include_directories(azure_c_util PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/pal/windows>)
    target_link_libraries(azure_c_util synchronization azure_c_util_logging)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/pal/windows)
else()
    target_include_directories(azure_c_util PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/pal/linux>)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/pal/linux)
endif()

target_include_directories(azure_c_util PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc>)

set(azure_c_util_target_libs)

if(${logerror_captures_stack_traces})
    set(azure_c_util_target_libs ${azure_c_util_target_libs} Dbghelp)
endif()

if(${use_applessl})
    set(azure_c_util_target_libs ${azure_c_util_target_libs} ${cf_foundation} ${cf_network})
endif()

if(WIN32)
    set(azure_c_util_target_libs ${azure_c_util_target_libs} rpcrt4.lib)
endif()

if(LINUX)
    set(azure_c_util_target_libs ${azure_c_util_target_libs} pthread m rt)
    set(azure_c_util_target_libs ${azure_c_util_target_libs} uuid)
endif()

target_link_libraries(azure_c_util ${azure_c_util_target_libs})

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

set(run_e2e_tests ${original_run_e2e_tests})
set(run_unittests ${original_run_unittests})
set(run_int_tests ${original_run_int_tests})
set(run_traceability ${original_run_traceability})

if((WIN32) AND (${run_traceability}))
    #add traceability custom target
    add_custom_target(azure_c_util_traceability ALL
        COMMAND ${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-build-tools/traceabilitytool/traceability_tool/bin/$<CONFIG>/traceabilitytool.exe -buildcheck -e ${CMAKE_CURRENT_LIST_DIR}/deps -i ${CMAKE_CURRENT_LIST_DIR})
    add_dependencies(azure_c_util_traceability traceabilitytool)
endif()

add_subdirectory(tests)

include(CMakePackageConfigHelpers)

compileTargetAsC99(azure_c_util)
