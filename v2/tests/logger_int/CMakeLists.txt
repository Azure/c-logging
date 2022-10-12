#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(logger_int main.c)
target_link_libraries(logger_int c_logging_v2)
add_test(NAME logger_int COMMAND logger_int)
