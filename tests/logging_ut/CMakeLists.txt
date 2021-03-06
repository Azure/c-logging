#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#the purpose of these tests is to see that a LogError/LogInfo/LogLastError would compile under all switch combinations
#the tests will not recompile the complete c_logging library, only the relevant parts. These tests only target Windows

if(${run_unittests})
    if(${WIN32})
        foreach(no_logging_value ON OFF)
            foreach(logerror_captures_stack_traces_value ON OFF)
                foreach(use_etw_value OFF TRACELOGGING TRACELOGGING_WITH_CONSOLE)
        
                    add_subdirectory(minimal "min${no_logging_value}${logerror_captures_stack_traces_value}${use_etw_value}")
        
                endforeach(use_etw_value)
            endforeach(logerror_captures_stack_traces_value)
        endforeach(no_logging_value)
    else() #LINUX - note it doesn't have LogLastError/stack capturing/ETW
        foreach(no_logging_value ON OFF)
            foreach(logerror_captures_stack_traces_value OFF)
                foreach(use_etw_value OFF)
        
                    add_subdirectory(minimal "min${no_logging_value}${logerror_captures_stack_traces_value}${use_etw_value}")
        
                endforeach(use_etw_value)
            endforeach(logerror_captures_stack_traces_value)
        endforeach(no_logging_value)
    endif()
endif()
