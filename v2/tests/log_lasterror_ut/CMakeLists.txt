#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#since we want to use all the other sources in c_logging_v2 except log_lasterror.c (which we have here mocked... we need to get the sources of c_logging_v2)
get_target_property(c_logging_v2_sources c_logging_v2 SOURCES)
message("${c_logging_v2_sources}")
list(REMOVE_ITEM c_logging_v2_sources ./src/log_lasterror.c)
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


add_executable(log_lasterror_ut
    ../../src/logger.c
    log_lasterror_mocked.c
    log_lasterror_ut.c
    ${c_logging_v2_sources_full_path}
)

include_directories(../../src)
target_link_libraries(log_lasterror_ut c_logging_v2)
add_test(NAME log_lasterror_ut COMMAND log_lasterror_ut)
set_target_properties(log_lasterror_ut PROPERTIES FOLDER "tests/c_logging_v2")
