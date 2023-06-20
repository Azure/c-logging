#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(log_hresult_ut
    ../../src/logger.c
    log_hresult_mocked.c
    log_hresult_ut.c
)

include_directories(../../src)
target_link_libraries(log_hresult_ut c_logging_v2)
add_test(NAME log_hresult_ut COMMAND log_hresult_ut)
set_target_properties(log_hresult_ut PROPERTIES FOLDER "tests/c_logging_v2")
