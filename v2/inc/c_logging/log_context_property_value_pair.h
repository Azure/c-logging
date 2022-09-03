// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_CONTEXT_PROPERTY_VALUE_PAIR_H
#define LOG_CONTEXT_PROPERTY_VALUE_PAIR_H

#include "c_logging/log_context_property_type_if.h"

typedef struct LOG_CONTEXT_PROPERTY_VALUE_PAIR_TAG
{
    const char* name;
    void* value;
    const LOG_CONTEXT_PROPERTY_TYPE_IF* type;
} LOG_CONTEXT_PROPERTY_VALUE_PAIR;

#endif /* LOG_CONTEXT_PROPERTY_VALUE_PAIR_H */
