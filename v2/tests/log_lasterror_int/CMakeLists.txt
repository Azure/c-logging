#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(log_lasterror_int
    ../../src/logger.c
    log_lasterror_int.c
    log_lasterror_mocked.c
)

include_directories(../../src)
target_link_libraries(log_lasterror_int c_logging_v2)
add_test(NAME log_lasterror_int COMMAND log_lasterror_int)
set_target_properties(log_lasterror_int PROPERTIES FOLDER "tests/c_logging_v2")
