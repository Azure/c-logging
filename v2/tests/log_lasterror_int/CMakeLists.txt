#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#since we want to use all the other sources in c_logging_v2 except log_last_error (which we have here mocked... we need to get the sources of c_logging_v2)
get_target_property(c_logging_v2_sources c_logging_v2 SOURCES)
list(REMOVE_ITEM c_logging_v2_sources ./src/log_lasterror.c)

get_target_property(c_logging_v2_dir c_logging_v2 SOURCE_DIR)

set(c_logging_v2_sources_full_path)
foreach(source ${c_logging_v2_sources})
    if(IS_ABSOLUTE ${source})
        set(c_logging_v2_sources_full_path ${c_logging_v2_sources_full_path} ${source})
    else()
        set(c_logging_v2_sources_full_path ${c_logging_v2_sources_full_path} ${c_logging_v2_dir}/${source})
    endif()

    #source_group("external files" FILES ${source})
endforeach()

add_executable(log_lasterror_int
    log_lasterror_int.c
    log_lasterror_mocked.c
    ${c_logging_v2_sources_full_path}
)

include_directories(../../src)
target_link_libraries(log_lasterror_int c_logging_v2_core)
add_test(NAME log_lasterror_int COMMAND log_lasterror_int)
set_target_properties(log_lasterror_int PROPERTIES FOLDER "tests/c_logging_v2")
