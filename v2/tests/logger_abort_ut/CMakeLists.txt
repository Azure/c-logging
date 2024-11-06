#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#since we want to use all the other sources in c_logging_v2 except logger.c (which we have here mocked )
get_target_property(c_logging_v2_sources c_logging_v2 SOURCES)
message("${c_logging_v2_sources}")
list(REMOVE_ITEM c_logging_v2_sources ./src/logger.c  ./src/logger_sinks_config.c)
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

add_executable(logger_abort_ut
    logger_mocked.c
    logger_abort_ut.c
    ${c_logging_v2_sources_full_path}
)

include_directories(../../src)
target_link_libraries(logger_abort_ut c_logging_v2_core)
add_test(NAME logger_abort_ut COMMAND logger_abort_ut)
set_target_properties(logger_abort_ut PROPERTIES FOLDER "tests/c_logging_v2")
