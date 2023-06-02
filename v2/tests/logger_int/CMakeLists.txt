#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

if(${WIN32})
    foreach(log_sink_console_test_value on off)
        foreach(log_sink_etw_test_value on off)
            add_subdirectory(common "logger_int_console_${log_sink_console_test_value}_etw_${log_sink_etw_test_value}")
        endforeach(log_sink_etw_test_value)
    endforeach(log_sink_console_test_value)
else() #LINUX - note it doesn't have LogLastError/stack capturing/ETW
    foreach(log_sink_console_test_value on off)
        foreach(log_sink_etw_test_value off)
            add_subdirectory(common "logger_int_console_${log_sink_console_test_value}_etw_${log_sink_etw_test_value}")
        endforeach(log_sink_etw_test_value)
    endforeach(log_sink_console_test_value)
endif()
