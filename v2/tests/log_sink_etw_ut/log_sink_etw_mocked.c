// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "windows.h"
#include "TraceLoggingProvider.h"
#include "evntrace.h"

#include "c_logging/log_context.h"
#include "c_logging/log_context_property_value_pair.h"

#define printf mock_printf
#undef InterlockedCompareExchange
#define InterlockedCompareExchange mock_InterlockedCompareExchange
#undef InterlockedExchange
#define InterlockedExchange mock_InterlockedExchange
#define log_context_get_property_value_pair_count mock_log_context_get_property_value_pair_count
#define log_context_get_property_value_pairs mock_log_context_get_property_value_pairs
#define TraceLoggingRegister_EventRegister_EventSetInformation mock_TraceLoggingRegister_EventRegister_EventSetInformation
#define _get_pgmptr mock__get_pgmptr
#define _tlgCreate1Sz_char mock__tlgCreate1Sz_char
#define EventDataDescCreate mock_EventDataDescCreate
#define _tlgWriteTransfer_EventWriteTransfer mock__tlgWriteTransfer_EventWriteTransfer
#define vsnprintf mock_vsnprintf

int mock_printf(const char* format, ...);
LONG mock_InterlockedCompareExchange(LONG volatile* Destination, LONG ExChange, LONG Comperand);
LONG mock_InterlockedExchange(LONG volatile* Target, LONG Value);
uint32_t mock_log_context_get_property_value_pair_count(LOG_CONTEXT_HANDLE log_context);
const LOG_CONTEXT_PROPERTY_VALUE_PAIR* mock_log_context_get_property_value_pairs(LOG_CONTEXT_HANDLE log_context);
TLG_STATUS mock_TraceLoggingRegister_EventRegister_EventSetInformation(const struct _tlgProvider_t* hProvider);
errno_t mock__get_pgmptr(char** pValue);
void mock__tlgCreate1Sz_char(PEVENT_DATA_DESCRIPTOR pDesc, char const* psz);
void mock_EventDataDescCreate(PEVENT_DATA_DESCRIPTOR EventDataDescriptor, const VOID* DataPtr, ULONG DataSize);
TLG_STATUS mock__tlgWriteTransfer_EventWriteTransfer(TraceLoggingHProvider hProvider, void const* pEventMetadata, LPCGUID pActivityId, LPCGUID pRelatedActivityId, UINT32 cData, EVENT_DATA_DESCRIPTOR* pData);
int mock_vsnprintf(char* restrict s, size_t n, const char* restrict format, va_list arg);

#include "log_sink_etw.c"
