#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(log_sink_etw_int_exe_zrpc
    log_sink_etw_int.c
)

include_directories(../../src)
target_link_libraries(log_sink_etw_int_exe_zrpc c_logging_v2 rpcrt4 synchronization)
add_test(NAME log_sink_etw_int_exe_ebs COMMAND log_sink_etw_int_exe_zrpc)
set_target_properties(log_sink_etw_int_exe_zrpc PROPERTIES FOLDER "tests/c_logging_v2")
