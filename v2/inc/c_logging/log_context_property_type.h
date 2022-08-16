// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_CONTEXT_PROPERTY_TYPE_H
#define LOG_CONTEXT_PROPERTY_TYPE_H

#include "macro_utils/macro_utils.h"

#define LOG_CONTEXT_PROPERTY_TYPE_VALUES \
    LOG_CONTEXT_PROPERTY_TYPE_int64_t, \
    LOG_CONTEXT_PROPERTY_TYPE_uint64_t, \
    LOG_CONTEXT_PROPERTY_TYPE_int32_t, \
    LOG_CONTEXT_PROPERTY_TYPE_uint32_t, \
    LOG_CONTEXT_PROPERTY_TYPE_int16_t, \
    LOG_CONTEXT_PROPERTY_TYPE_uint16_t, \
    LOG_CONTEXT_PROPERTY_TYPE_int8_t, \
    LOG_CONTEXT_PROPERTY_TYPE_uint8_t \

MU_DEFINE_ENUM(LOG_CONTEXT_PROPERTY_TYPE, LOG_CONTEXT_PROPERTY_TYPE_VALUES)

#endif /* LOG_CONTEXT_PROPERTY_TYPE_H */
