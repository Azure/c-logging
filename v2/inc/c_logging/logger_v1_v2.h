// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOGGER_V1_V2_H
#define LOGGER_V1_V2_H

#define LogCritical(...) LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, __VA_ARGS__)
#define LogError(...) LOGGER_LOG(LOG_LEVEL_ERROR, NULL, __VA_ARGS__)
#define LogWarning(...) LOGGER_LOG(LOG_LEVEL_WARNING, NULL, __VA_ARGS__)
#define LogInfo(...) LOGGER_LOG(LOG_LEVEL_INFO, NULL, __VA_ARGS__)
#define LogVerbose(...) LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, __VA_ARGS__)

#define LogLastError(...) LOGGER_LOG_EX(LOG_LEVEL_ERROR, LOG_LASTERROR(), LOG_MESSAGE(__VA_ARGS__))

#endif /* LOGGER_V1_V2_H */
