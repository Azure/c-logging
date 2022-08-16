// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_LEVEL_H
#define LOG_LEVEL_H

#include "macro_utils/macro_utils.h"

#define LOG_LEVEL_VALUES \
    LOG_LEVEL_CRITICAL, \
    LOG_LEVEL_ERROR, \
    LOG_LEVEL_WARNING, \
    LOG_LEVEL_INFO, \
    LOG_LEVEL_VERBOSE

MU_DEFINE_ENUM_WITHOUT_INVALID(LOG_LEVEL, LOG_LEVEL_VALUES);

#endif /* LOG_LEVEL_H */
