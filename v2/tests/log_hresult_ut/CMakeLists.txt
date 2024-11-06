#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#since we want to use all the other sources in c_logging_v2 except logger.c (which we have here mocked ) and logger_sinks_config.c (we use our own sinks) ... we need to get the other sources of c_logging_v2
get_target_property(c_logging_v2_sources c_logging_v2 SOURCES)
message("${c_logging_v2_sources}")
list(REMOVE_ITEM c_logging_v2_sources ./src/log_hresult.c)
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

add_executable(log_hresult_ut
    log_hresult_mocked.c
    log_hresult_ut.c
    ${c_logging_v2_sources_full_path}
)

#due to usage of "errno" (which is a MACRO... or a function)... or a function with the same name as the macro... log_errno_win32_mocked.c has to be compiled separately, not in UNITY
set_source_files_properties(log_hresult_mocked.c PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

include_directories(../../src)
target_link_libraries(log_hresult_ut c_logging_v2_core)
add_test(NAME log_hresult_ut COMMAND log_hresult_ut)
set_target_properties(log_hresult_ut PROPERTIES FOLDER "tests/c_logging_v2")
