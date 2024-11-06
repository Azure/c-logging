#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#since we want to use all the other sources in c_logging_v2 except log_sink_callback.c (which we have here mocked... we need to get the sources of c_logging_v2)
get_target_property(c_logging_v2_sources c_logging_v2 SOURCES)
message("${c_logging_v2_sources}")
list(REMOVE_ITEM c_logging_v2_sources ./src/log_sink_callback.c)
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

add_executable(log_sink_callback_ut
    log_sink_callback_ut.c
    log_sink_callback_mocked.c
    ${c_logging_v2_sources_full_path}
)

include_directories(../../src)
target_link_libraries(log_sink_callback_ut c_logging_v2_core)
add_test(NAME log_sink_callback_ut COMMAND log_sink_callback_ut)
set_target_properties(log_sink_callback_ut PROPERTIES FOLDER "tests/c_logging_v2")
