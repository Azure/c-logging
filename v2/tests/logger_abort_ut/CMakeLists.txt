#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(logger_abort_ut
    logger_mocked.c
    logger_abort_ut.c
)

include_directories(../../src)
target_link_libraries(logger_abort_ut c_logging_v2)
add_test(NAME logger_abort_ut COMMAND logger_abort_ut)
set_target_properties(logger_abort_ut PROPERTIES FOLDER "tests/c_logging_v2")
