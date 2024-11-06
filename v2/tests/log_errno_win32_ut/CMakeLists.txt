#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#since we want to use all the other sources in c_logging_v2 except log_errno_win32.c (which we have here mocked )
get_target_property(c_logging_v2_sources c_logging_v2 SOURCES)
message("${c_logging_v2_sources}")
list(REMOVE_ITEM c_logging_v2_sources ./src/log_errno_win32.c)
message("${c_logging_v2_sources}")

get_target_property(c_logging_v2_dir c_logging_v2 SOURCE_DIR)

set(c_logging_v2_sources_full_path)
foreach(source ${c_logging_v2_sources})
    if(IS_ABSOLUTE ${source})
        set(c_logging_v2_sources_full_path ${c_logging_v2_sources_full_path} ${source})
    else()
        set(c_logging_v2_sources_full_path ${c_logging_v2_sources_full_path} ${c_logging_v2_dir}/${source})
    endif()
endforeach()

add_executable(log_errno_ut
    log_errno_win32_mocked.c
    log_errno_win32_ut.c
    ${c_logging_v2_sources_full_path}
)

#due to usage of "errno" (which is a MACRO... or a function)... or a function with the same name as the macro... log_errno_win32_mocked.c has to be compiled separately, not in UNITY
set_source_files_properties(log_errno_win32_mocked.c PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

include_directories(../../src)
target_link_libraries(log_errno_ut c_logging_v2_core)
add_test(NAME log_errno_ut COMMAND log_errno_ut)
set_target_properties(log_errno_ut PROPERTIES FOLDER "tests/c_logging_v2")
