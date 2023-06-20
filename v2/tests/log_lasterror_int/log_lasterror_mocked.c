// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "windows.h"

// only mock last error so we can trigger the property formatting
DWORD mock_GetLastError(void);

#define GetLastError mock_GetLastError

#include "log_lasterror.c"
