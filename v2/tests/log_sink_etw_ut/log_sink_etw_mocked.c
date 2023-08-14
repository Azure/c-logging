// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdint.h>
#include <time.h>
#include <wchar.h>

#include "windows.h"
#include "TraceLoggingProvider.h"
#include "evntrace.h"

#include "c_logging/log_context.h"
#include "c_logging/log_context_property_value_pair.h"

#define printf mock_printf
#define log_context_get_property_value_pair_count mock_log_context_get_property_value_pair_count
#define log_context_get_property_value_pairs mock_log_context_get_property_value_pairs
#define TraceLoggingRegister_EventRegister_EventSetInformation mock_TraceLoggingRegister_EventRegister_EventSetInformation
#define GetModuleFileNameA mock_GetModuleFileNameA
#define _tlgCreate1Sz_char mock__tlgCreate1Sz_char
#define _tlgCreate1Sz_wchar_t mock__tlgCreate1Sz_wchar_t
#define EventDataDescCreate mock_EventDataDescCreate
#define _tlgWriteTransfer_EventWriteTransfer mock__tlgWriteTransfer_EventWriteTransfer
#define vsnprintf mock_vsnprintf
#undef TraceLoggingUnregister
#define TraceLoggingUnregister mock_TraceLoggingUnregister

int mock_printf(const char* format, ...);
uint32_t mock_log_context_get_property_value_pair_count(LOG_CONTEXT_HANDLE log_context);
const LOG_CONTEXT_PROPERTY_VALUE_PAIR* mock_log_context_get_property_value_pairs(LOG_CONTEXT_HANDLE log_context);
TLG_STATUS mock_TraceLoggingRegister_EventRegister_EventSetInformation(const struct _tlgProvider_t* hProvider);
BOOL mock_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
void mock__tlgCreate1Sz_char(PEVENT_DATA_DESCRIPTOR pDesc, char const* psz);
void mock__tlgCreate1Sz_wchar_t(PEVENT_DATA_DESCRIPTOR pDesc, wchar_t const* psz);
void mock_EventDataDescCreate(PEVENT_DATA_DESCRIPTOR EventDataDescriptor, const VOID* DataPtr, ULONG DataSize);
TLG_STATUS mock__tlgWriteTransfer_EventWriteTransfer(TraceLoggingHProvider hProvider, void const* pEventMetadata, LPCGUID pActivityId, LPCGUID pRelatedActivityId, UINT32 cData, EVENT_DATA_DESCRIPTOR* pData);
int mock_vsnprintf(char* s, size_t n, const char* format, va_list arg);
void mock_TraceLoggingUnregister(TraceLoggingHProvider hProvider);

#include "log_sink_etw.c"
