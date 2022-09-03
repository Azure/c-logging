// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>

#include "c_logging/log_internal_error.h"

void log_internal_error_report(void)
{
    /* Codes_SRS_LOG_INTERNAL_ERROR_01_001: [ If LOG_ABORT_ON_ERROR is defined, log_internal_error_report shall call abort the execution. ]*/
#ifdef LOG_ABORT_ON_ERROR
#if defined _MSC_VER
    /* Codes_SRS_LOG_INTERNAL_ERROR_01_003: [ On Windows, if LOG_ABORT_ON_ERROR is defined, log_internal_error_report shall call _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT) to disable the abort message. ]*/
    (void)_set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
    abort();
#endif // LOG_ABORT_ON_ERROR

    /* Codes_SRS_LOG_INTERNAL_ERROR_01_002: [ Otherwise log_internal_error_report shall return. ]*/
}
