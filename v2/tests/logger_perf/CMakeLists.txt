#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(logger_perf main.c timer.h timer_win32.c)
target_link_libraries(logger_perf c_logging c_logging_v2)
add_test(NAME logger_perf COMMAND logger_perf)
