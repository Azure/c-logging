#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(log_lasterror_ut
    ../../src/logger.c
    ../../src/log_lasterror.c
    log_lasterror_ut.c
)

include_directories(../../src)
target_link_libraries(log_lasterror_ut c_logging_v2)
add_test(NAME log_lasterror_ut COMMAND log_lasterror_ut)
set_target_properties(log_lasterror_ut PROPERTIES FOLDER "tests/c_logging_v2")
