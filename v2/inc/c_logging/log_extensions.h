// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_EXTENSIONS_H
#define LOG_EXTENSIONS_H

#include "c_logging/log_context.h"

#define LOG_MESSAGE(...) \
    LOG_CONTEXT_STRING_PROPERTY(message, __VA_ARGS__)

#endif /* LOG_EXTENSIONS_H */
