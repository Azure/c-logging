#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(log_errno_ut
    ../../src/logger.c
    log_errno_linux_mocked.c
    log_errno_linux_ut.c
)

include_directories(../../src)
target_link_libraries(log_errno_ut c_logging_v2)
add_test(NAME log_errno_ut COMMAND log_errno_ut)
set_target_properties(log_errno_ut PROPERTIES FOLDER "tests/c_logging_v2")
