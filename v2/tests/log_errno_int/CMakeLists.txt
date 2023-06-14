#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(log_errno_int
    ../../src/logger.c
    log_errno_int.c
    log_errno_mocked.c
)

include_directories(../../src)
target_link_libraries(log_errno_int c_logging_v2)
add_test(NAME log_errno_int COMMAND log_errno_int)
set_target_properties(log_errno_int PROPERTIES FOLDER "tests/c_logging_v2")
