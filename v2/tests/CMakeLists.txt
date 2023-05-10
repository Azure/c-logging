#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#unit tests
if(${run_unittests})
   add_subdirectory(log_context_property_basic_types_ut)
   add_subdirectory(log_context_property_type_ascii_char_ptr_ut)
   add_subdirectory(log_context_property_type_if_ut)
   add_subdirectory(log_context_property_type_struct_ut)
   add_subdirectory(log_context_ut)
   add_subdirectory(log_internal_error_ut)
   add_subdirectory(log_internal_error_with_abort_ut)
   add_subdirectory(log_sink_console_ut)
   add_subdirectory(log_sink_etw_ut)
   add_subdirectory(log_sink_etw_2_ut)
   add_subdirectory(log_sink_etw_registering_ut)
endif()

if(${run_int_tests})
   add_subdirectory(log_context_property_basic_types_int)
   add_subdirectory(log_context_property_type_ascii_char_ptr_int)
   add_subdirectory(log_sink_console_int)
   add_subdirectory(log_sink_etw_int)
endif()

if(${run_perf_tests})
endif()
