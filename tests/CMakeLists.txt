#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#unit tests
if(${run_unittests})
   add_subdirectory(logging_ut)
endif()

if(${run_int_tests})
   add_subdirectory(logging_int)
endif()
