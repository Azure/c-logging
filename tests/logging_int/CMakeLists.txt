#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

add_executable(logging_int main.c)
target_link_libraries(logging_int c_logging)
add_test(NAME logging_int COMMAND logging_int)
