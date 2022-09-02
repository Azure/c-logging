#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#unit tests
if(${run_unittests})
   add_subdirectory(log_context_property_basic_types_ut)
   add_subdirectory(log_context_property_type_ascii_char_ptr_ut)
   add_subdirectory(log_context_property_type_if_ut)
   add_subdirectory(log_context_property_type_struct_ut)
   add_subdirectory(log_context_ut)
endif()

if(${run_int_tests})
endif()

if(${run_perf_tests})
endif()
