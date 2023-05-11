// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "windows.h"
#include "TraceLoggingProvider.h"
#include "evntrace.h"

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_level.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/log_context.h"
#include "c_logging/logger.h"

#include "c_logging/log_sink_etw.h"

static size_t asserts_failed = 0;

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define FILE_EVENT_DESCRIPTOR_ENTRY 3
#define FUNC_EVENT_DESCRIPTOR_ENTRY 4
#define LINE_EVENT_DESCRIPTOR_ENTRY 5

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_printf, \
    MOCK_CALL_TYPE_InterlockedCompareExchange, \
    MOCK_CALL_TYPE_InterlockedExchange, \
    MOCK_CALL_TYPE_log_context_get_property_value_pair_count, \
    MOCK_CALL_TYPE_log_context_get_property_value_pairs, \
    MOCK_CALL_TYPE_TraceLoggingRegister_EventRegister_EventSetInformation, \
    MOCK_CALL_TYPE__get_pgmptr, \
    MOCK_CALL_TYPE__tlgCreate1Sz_char, \
    MOCK_CALL_TYPE_EventDataDescCreate, \
    MOCK_CALL_TYPE__tlgWriteTransfer_EventWriteTransfer, \
    MOCK_CALL_TYPE_log_context_property_if_get_type, \
    MOCK_CALL_TYPE_vsnprintf

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(

#define MAX_PRINTF_CAPTURED_OUPUT_SIZE (LOG_MAX_MESSAGE_LENGTH * 2)

static TraceLoggingHProvider test_provider;

__pragma(pack(push, 1))
typedef struct SELF_DESCRIBED_EVENT_TAG
{
    uint8_t _tlgBlobTyp;
    uint8_t _tlgChannel;
    uint8_t _tlgLevel;
    uint8_t _tlgOpcode;
    uint64_t _tlgKeyword;
    uint16_t _tlgEvtMetaSize;
    uint16_t _tlgEvtTag;
    uint8_t metadata[];
} SELF_DESCRIBED_EVENT;
__pragma(pack(pop))

typedef struct printf_CALL_TAG
{
    bool override_result;
    int call_result;
    char captured_output[MAX_PRINTF_CAPTURED_OUPUT_SIZE];
} printf_CALL;

typedef struct InterlockedCompareExchange_CALL_TAG
{
    bool override_result;
    int call_result;
} InterlockedCompareExchange_CALL;

typedef struct InterlockedExchange_CALL_TAG
{
    bool override_result;
    int call_result;
} InterlockedExchange_CALL;

typedef struct log_context_get_property_value_pair_count_CALL_TAG
{
    bool override_result;
    uint32_t call_result;
    LOG_CONTEXT_HANDLE captured_log_context;
} log_context_get_property_value_pair_count_CALL;

typedef struct log_context_get_property_value_pairs_CALL_TAG
{
    bool override_result;
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* call_result;
    LOG_CONTEXT_HANDLE captured_log_context;
} log_context_get_property_value_pairs_CALL;

typedef struct TraceLoggingRegister_EventRegister_EventSetInformation_CALL_TAG
{
    bool override_result;
    TLG_STATUS call_result;
    uint8_t enable_provider_level;
    const char* expected_provider_id_as_string;
} TraceLoggingRegister_EventRegister_EventSetInformation_CALL;

typedef struct _get_pgmptr_CALL_TAG
{
    bool override_result;
    char* injected_pValue;
    errno_t call_result;
} _get_pgmptr_CALL;

typedef struct _tlgWriteTransfer_EventWriteTransfer_CALL_TAG
{
    bool override_result;
    TLG_STATUS call_result;
    SELF_DESCRIBED_EVENT* expected_self_described_event;
    uint32_t expected_cData;
    const EVENT_DATA_DESCRIPTOR* expected_pData;
    bool ignore_file_and_func;
} _tlgWriteTransfer_EventWriteTransfer_CALL;

typedef struct log_context_property_if_get_type_CALL_TAG
{
    bool override_result;
    LOG_CONTEXT_PROPERTY_TYPE call_result;
} log_context_property_if_get_type_CALL;

typedef struct vsnprintf_CALL_TAG
{
    bool override_result;
    int call_result;
    char captured_output[MAX_PRINTF_CAPTURED_OUPUT_SIZE];
} vsnprintf_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        printf_CALL printf_call;
        InterlockedCompareExchange_CALL InterlockedCompareExchange_call;
        InterlockedExchange_CALL InterlockedExchange_call;
        log_context_get_property_value_pair_count_CALL log_context_get_property_value_pair_count_call;
        log_context_get_property_value_pairs_CALL log_context_get_property_value_pairs_call;
        TraceLoggingRegister_EventRegister_EventSetInformation_CALL TraceLoggingRegister_EventRegister_EventSetInformation_call;
        _get_pgmptr_CALL _get_pgmptr_call;
        _tlgWriteTransfer_EventWriteTransfer_CALL _tlgWriteTransfer_EventWriteTransfer_call;
        log_context_property_if_get_type_CALL log_context_property_if_get_type_call;
        vsnprintf_CALL vsnprintf_call;
    } u;
} MOCK_CALL;

static char* stringify_bytes(const unsigned char* bytes, size_t byte_count)
{
    size_t i;
    size_t pos = 0;
    size_t stringified_length = 3 + (byte_count * 4);
    char* result;

    if (byte_count > 0)
    {
        stringified_length += (byte_count - 1);
    }

    result = malloc(stringified_length);
    if (result != NULL)
    {
        result[pos++] = '[';

        for (i = 0; i < byte_count; i++)
        {
            int sprintf_result = sprintf(&result[pos], "0x%02X%s", bytes[i], (i < byte_count - 1) ? "," : "");
            pos += sprintf_result;
        }
        result[pos++] = ']';
        result[pos] = '\0';
    }

    return result;
}

static MOCK_CALL expected_calls[MAX_MOCK_CALL_COUNT];
static size_t expected_call_count;
static size_t actual_call_count;
static bool actual_and_expected_match;

static void setup_mocks(void)
{
    expected_call_count = 0;
    actual_call_count = 0;
    actual_and_expected_match = true;
}

int mock_printf(const char* format, ...)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_printf))
    {
        actual_and_expected_match = false;
        result = -1;
    }
    else
    {
        if (expected_calls[actual_call_count].u.printf_call.override_result)
        {
            result = expected_calls[actual_call_count].u.printf_call.call_result;
        }
        else
        {
            va_list args;
            va_start(args, format);
            // also capture the result in a variable for comparisons in tests
            (void)vsnprintf(expected_calls[actual_call_count].u.printf_call.captured_output, sizeof(expected_calls[actual_call_count].u.printf_call.captured_output),
                format, args);
            va_end(args);

            va_start(args, format);
            result = vprintf(format, args);
            va_end(args);
        }

        actual_call_count++;
    }

    return result;
}

LONG mock_InterlockedCompareExchange(LONG volatile* Destination, LONG ExChange, LONG Comperand)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_InterlockedCompareExchange))
    {
        actual_and_expected_match = false;
        result = -1;
    }
    else
    {
        if (expected_calls[actual_call_count].u.printf_call.override_result)
        {
            result = expected_calls[actual_call_count].u.InterlockedCompareExchange_call.call_result;
        }
        else
        {
            result = InterlockedCompareExchange(Destination, ExChange, Comperand);
        }

        actual_call_count++;
    }

    return result;
}

LONG mock_InterlockedExchange(LONG volatile* Target, LONG Value)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_InterlockedExchange))
    {
        actual_and_expected_match = false;
        result = -1;
    }
    else
    {
        if (expected_calls[actual_call_count].u.printf_call.override_result)
        {
            result = expected_calls[actual_call_count].u.InterlockedExchange_call.call_result;
        }
        else
        {
            result = InterlockedExchange(Target, Value);
        }

        actual_call_count++;
    }

    return result;
}

uint32_t mock_log_context_get_property_value_pair_count(LOG_CONTEXT_HANDLE log_context)
{
    uint32_t result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_context_get_property_value_pair_count))
    {
        actual_and_expected_match = false;
        result = UINT32_MAX;
    }
    else
    {
        if (expected_calls[actual_call_count].u.log_context_get_property_value_pair_count_call.override_result)
        {
            result = expected_calls[actual_call_count].u.log_context_get_property_value_pair_count_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].u.log_context_get_property_value_pair_count_call.captured_log_context = log_context;

            result = log_context_get_property_value_pair_count(log_context);
        }

        actual_call_count++;
    }

    return result;
}

const LOG_CONTEXT_PROPERTY_VALUE_PAIR* mock_log_context_get_property_value_pairs(LOG_CONTEXT_HANDLE log_context)
{
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_context_get_property_value_pairs))
    {
        actual_and_expected_match = false;
        result = NULL;
    }
    else
    {
        if (expected_calls[actual_call_count].u.log_context_get_property_value_pairs_call.override_result)
        {
            result = expected_calls[actual_call_count].u.log_context_get_property_value_pairs_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].u.log_context_get_property_value_pairs_call.captured_log_context = log_context;

            result = log_context_get_property_value_pairs(log_context);
        }

        actual_call_count++;
    }

    return result;
}

TLG_STATUS mock_TraceLoggingRegister_EventRegister_EventSetInformation(const struct _tlgProvider_t* hProvider)
{
    TLG_STATUS result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_TraceLoggingRegister_EventRegister_EventSetInformation))
    {
        actual_and_expected_match = false;
        result = E_FAIL;
    }
    else
    {
        if (expected_calls[actual_call_count].u.TraceLoggingRegister_EventRegister_EventSetInformation_call.override_result)
        {
            result = expected_calls[actual_call_count].u.TraceLoggingRegister_EventRegister_EventSetInformation_call.call_result;
        }
        else
        {
            // save the provider globally
            test_provider = hProvider;
            GUID provider_guid = TraceLoggingProviderId(hProvider);
            char provider_guid_string[128];

            (void)sprintf(provider_guid_string, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", provider_guid.Data1, provider_guid.Data2, provider_guid.Data3, provider_guid.Data4[0], provider_guid.Data4[1], provider_guid.Data4[2], provider_guid.Data4[3], provider_guid.Data4[4], provider_guid.Data4[5], provider_guid.Data4[6], provider_guid.Data4[7]);

            if (strcmp(provider_guid_string, expected_calls[actual_call_count].u.TraceLoggingRegister_EventRegister_EventSetInformation_call.expected_provider_id_as_string) != 0)
            {
                (void)printf("Expected provider id to be %s, actual = %s",
                    expected_calls[actual_call_count].u.TraceLoggingRegister_EventRegister_EventSetInformation_call.expected_provider_id_as_string, provider_guid_string);
                actual_and_expected_match = false;

                result = E_FAIL;
            }
            else
            {
                result = TraceLoggingRegister_EventRegister_EventSetInformation(hProvider);

                ((struct _tlgProvider_t*)test_provider)->LevelPlus1 = expected_calls[actual_call_count].u.TraceLoggingRegister_EventRegister_EventSetInformation_call.enable_provider_level + 1;
            }
        }

        actual_call_count++;
    }

    return result;
}

errno_t mock__get_pgmptr(char** pValue)
{
    TLG_STATUS result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE__get_pgmptr))
    {
        actual_and_expected_match = false;
        result = E_FAIL;
    }
    else
    {
        if (expected_calls[actual_call_count].u._get_pgmptr_call.override_result)
        {
            *pValue = expected_calls[actual_call_count].u._get_pgmptr_call.injected_pValue;
            result = expected_calls[actual_call_count].u._get_pgmptr_call.call_result;
        }
        else
        {
            result = _get_pgmptr(pValue);
        }

        actual_call_count++;
    }

    return result;
}

void mock__tlgCreate1Sz_char(PEVENT_DATA_DESCRIPTOR pDesc, char const* psz)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE__tlgCreate1Sz_char))
    {
        actual_and_expected_match = false;
    }
    else
    {
        _tlgCreate1Sz_char(pDesc, psz);

        actual_call_count++;
    }
}

void mock_EventDataDescCreate(PEVENT_DATA_DESCRIPTOR EventDataDescriptor, const VOID* DataPtr, ULONG DataSize)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_EventDataDescCreate))
    {
        actual_and_expected_match = false;
    }
    else
    {
        EventDataDescCreate(EventDataDescriptor, DataPtr, DataSize);

        actual_call_count++;
    }
}

errno_t mock__tlgWriteTransfer_EventWriteTransfer(TraceLoggingHProvider hProvider, void const* pEventMetadata, LPCGUID pActivityId, LPCGUID pRelatedActivityId, UINT32 cData, EVENT_DATA_DESCRIPTOR* pData)
{
    TLG_STATUS result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE__tlgWriteTransfer_EventWriteTransfer))
    {
        actual_and_expected_match = false;
        result = E_FAIL;
    }
    else
    {
        if (expected_calls[actual_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.override_result)
        {
            result = expected_calls[actual_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.call_result;
        }
        else
        {
            SELF_DESCRIBED_EVENT* expected_self_described_event = expected_calls[actual_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_self_described_event;
            SELF_DESCRIBED_EVENT* actual_self_described_event = (SELF_DESCRIBED_EVENT*)CONTAINING_RECORD(pEventMetadata, SELF_DESCRIBED_EVENT, _tlgChannel);
            if (expected_self_described_event->_tlgLevel != actual_self_described_event->_tlgLevel)
            {
                (void)printf("Expected expected_self_described_event->_tlgLevel=%" PRIu8 ", actual=%" PRIu8 "\r\n",
                    expected_self_described_event->_tlgLevel, actual_self_described_event->_tlgLevel);
                actual_and_expected_match = false;
                result = E_FAIL;
            }
            if (expected_self_described_event->_tlgEvtMetaSize != actual_self_described_event->_tlgEvtMetaSize)
            {
                (void)printf("Expected expected_self_described_event->_tlgEvtMetaSize=%" PRIu16 ", actual=%" PRIu16 "\r\n",
                    expected_self_described_event->_tlgEvtMetaSize, actual_self_described_event->_tlgEvtMetaSize);
                actual_and_expected_match = false;
                result = E_FAIL;
            }
            if (expected_self_described_event->_tlgChannel != actual_self_described_event->_tlgChannel)
            {
                (void)printf("Expected expected_self_described_event->_tlgChannel=%" PRIu8 ", actual=%" PRIu8 "\r\n",
                    expected_self_described_event->_tlgChannel, actual_self_described_event->_tlgChannel);
                actual_and_expected_match = false;
                result = E_FAIL;
            }
            if (expected_self_described_event->_tlgOpcode != actual_self_described_event->_tlgOpcode)
            {
                (void)printf("Expected expected_self_described_event->_tlgOpcode=%" PRIu8 ", actual=%" PRIu8 "\r\n",
                    expected_self_described_event->_tlgOpcode, actual_self_described_event->_tlgOpcode);
                actual_and_expected_match = false;
                result = E_FAIL;
            }
            if (expected_self_described_event->_tlgKeyword != actual_self_described_event->_tlgKeyword)
            {
                (void)printf("Expected expected_self_described_event->_tlgKeyword=%" PRIu64 ", actual=%" PRIu64"\r\n",
                    expected_self_described_event->_tlgKeyword, actual_self_described_event->_tlgKeyword);
                actual_and_expected_match = false;
                result = E_FAIL;
            }
            else if (memcmp(expected_self_described_event->metadata, actual_self_described_event->metadata, actual_self_described_event->_tlgEvtMetaSize - 4) != 0)
            {
                char* expected_bytes_as_string = stringify_bytes(expected_self_described_event->metadata, actual_self_described_event->_tlgEvtMetaSize - 4);
                char* actual_bytes_as_string = stringify_bytes(actual_self_described_event->metadata, actual_self_described_event->_tlgEvtMetaSize - 4);
                (void)printf("Event metadata does not match:\r\n Expected:%s\r\n Actual  :%s\r\n", expected_bytes_as_string, actual_bytes_as_string);
                free(expected_bytes_as_string);
                free(actual_bytes_as_string);
                actual_and_expected_match = false;
                result = E_FAIL;
            }
            else if (cData != expected_calls[actual_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_cData)
            {
                (void)printf("Expected cData=%" PRIu32 ", actual=%" PRIu32 "\r\n",
                    expected_calls[actual_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_cData, cData);
                actual_and_expected_match = false;
                result = E_FAIL;
            }
            else
            {
                // only compare starting at the index 2
                for (uint32_t i = 2; i < cData; i++)
                {
                    if (
                        (expected_calls[actual_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.ignore_file_and_func) &&
                        (
                            (i == FILE_EVENT_DESCRIPTOR_ENTRY) ||
                            (i == FUNC_EVENT_DESCRIPTOR_ENTRY) ||
                            (i == LINE_EVENT_DESCRIPTOR_ENTRY)
                            )
                        )
                    {
                        continue;
                    }

                    if (pData[i].Size != expected_calls[actual_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_pData[i].Size)
                    {
                        (void)printf("Expected pData[%" PRIu32 "].Size=%" PRIu32 ", actual=%" PRIu32 "\r\n",
                            i, expected_calls[actual_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_pData[i].Size, pData[i].Size);
                        break;
                    }
                    if (memcmp((void*)(uintptr_t)pData[i].Ptr, (void*)(uintptr_t)expected_calls[actual_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_pData[i].Ptr, pData[i].Size) != 0)
                    {
                        (void)printf("Event descriptor memory at index %" PRIu32 " does not match\r\n",
                            i);
                        break;
                    }
                }

                (void)hProvider;
                (void)pActivityId;
                (void)pRelatedActivityId;
                result = S_OK;
            }
        }

        actual_call_count++;
    }

    return result;
}

LOG_CONTEXT_PROPERTY_TYPE mock_log_context_property_if_get_type(void)
{
    LOG_CONTEXT_PROPERTY_TYPE result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_context_property_if_get_type))
    {
        actual_and_expected_match = false;
        result = (LOG_CONTEXT_PROPERTY_TYPE)0xFF;
    }
    else
    {
        if (expected_calls[actual_call_count].u.log_context_property_if_get_type_call.override_result)
        {
            result = expected_calls[actual_call_count].u.log_context_property_if_get_type_call.call_result;
        }
        else
        {
            result = (LOG_CONTEXT_PROPERTY_TYPE)0xFF;
        }

        actual_call_count++;
    }

    return result;
}

int mock_vsnprintf(char* restrict s, size_t n, const char* restrict format, va_list args)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_vsnprintf))
    {
        actual_and_expected_match = false;
        result = -1;
    }
    else
    {
        if (expected_calls[actual_call_count].u.printf_call.override_result)
        {
            result = expected_calls[actual_call_count].u.printf_call.call_result;
        }
        else
        {
            // also capture the result in a variable for comparisons in tests
            (void)vsnprintf(expected_calls[actual_call_count].u.printf_call.captured_output, sizeof(expected_calls[actual_call_count].u.printf_call.captured_output),
                format, args);

            result = vsnprintf(s, n, format, args);
        }

        actual_call_count++;
    }

    return result;
}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        asserts_failed++; \
    } \

static void setup_enabled_provider(UINT32 level)
{
    ((struct _tlgProvider_t*)test_provider)->LevelPlus1 = level + 1;
}

static void setup_printf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_printf;
    expected_calls[expected_call_count].u.printf_call.override_result = false;
    expected_call_count++;
}

static void setup_InterlockedCompareExchange_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_InterlockedCompareExchange;
    expected_calls[expected_call_count].u.InterlockedCompareExchange_call.override_result = false;
    expected_call_count++;
}

static void setup_InterlockedExchange_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_InterlockedExchange;
    expected_calls[expected_call_count].u.InterlockedExchange_call.override_result = false;
    expected_call_count++;
}

static void setup_log_context_get_property_value_pair_count_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_context_get_property_value_pair_count;
    expected_calls[expected_call_count].u.log_context_get_property_value_pair_count_call.override_result = false;
    expected_call_count++;
}

static void setup_log_context_get_property_value_pairs_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_context_get_property_value_pairs;
    expected_calls[expected_call_count].u.log_context_get_property_value_pairs_call.override_result = false;
    expected_call_count++;
}

static void setup_TraceLoggingRegister_EventRegister_EventSetInformation_call(const char* expected_provider_id_as_string, uint8_t enable_provider_level)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_TraceLoggingRegister_EventRegister_EventSetInformation;
    expected_calls[expected_call_count].u.TraceLoggingRegister_EventRegister_EventSetInformation_call.override_result = false;
    expected_calls[expected_call_count].u.TraceLoggingRegister_EventRegister_EventSetInformation_call.enable_provider_level = enable_provider_level;
    expected_calls[expected_call_count].u.TraceLoggingRegister_EventRegister_EventSetInformation_call.expected_provider_id_as_string = expected_provider_id_as_string;
    expected_call_count++;
}

static void setup__get_pgmptr_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE__get_pgmptr;
    expected_calls[expected_call_count].u._get_pgmptr_call.override_result = false;
    expected_call_count++;
}

static void setup__tlgCreate1Sz_char(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE__tlgCreate1Sz_char;
    expected_call_count++;
}

static void setup_EventDataDescCreate(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_EventDataDescCreate;
    expected_call_count++;
}

static void setup__tlgWriteTransfer_EventWriteTransfer(void* event_metadata, uint32_t cData, const EVENT_DATA_DESCRIPTOR* pData)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE__tlgWriteTransfer_EventWriteTransfer;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.override_result = false;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_self_described_event = event_metadata;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_cData = cData;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_pData = pData;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.ignore_file_and_func = false;
    expected_call_count++;
}

static void setup__tlgWriteTransfer_EventWriteTransfer_with_ignore_file_and_func(void* event_metadata, uint32_t cData, const EVENT_DATA_DESCRIPTOR* pData, bool ignore_file_and_func)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE__tlgWriteTransfer_EventWriteTransfer;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.override_result = false;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_self_described_event = event_metadata;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_cData = cData;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_pData = pData;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.ignore_file_and_func = ignore_file_and_func;
    expected_call_count++;
}

static void setup_log_context_property_if_get_type(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_context_property_if_get_type;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.override_result = false;
    expected_call_count++;
}

static void setup_vsnprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_vsnprintf;
    expected_calls[expected_call_count].u.vsnprintf_call.override_result = false;
    expected_call_count++;
}

/* log_sink_etw.log_sink_log */

/* Tests_SRS_LOG_SINK_ETW_01_001: [ If message_format is NULL, log_sink_etw.log_sink_log shall return. ]*/
static void log_sink_etw_log_with_NULL_message_format_returns(void)
{
    // arrange
    setup_mocks();
    setup_printf_call();

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, __LINE__, NULL);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_SINK_ETW_01_002: [ log_sink_etw_log shall maintain the state of whether TraceLoggingRegister was called in a variable accessed via InterlockedXXX APIs, which shall have 3 possible values: NOT_REGISTERED (1), REGISTERING (2), REGISTERED(3). ]*/
/* Tests_SRS_LOG_SINK_ETW_01_003: [ log_sink_etw_log shall perform the below actions until the provider is registered or an error is encountered: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_004: [ If the state is NOT_REGISTERED (1): ]*/
/* Tests_SRS_LOG_SINK_ETW_01_005: [ log_sink_etw_log shall switch the state to REGISTERING (2). ]*/
/* Tests_SRS_LOG_SINK_ETW_01_006: [ log_sink_etw_log shall register the ETW TraceLogging provider by calling TraceLoggingRegister (TraceLoggingRegister_EventRegister_EventSetInformation). ]*/
/* Tests_SRS_LOG_SINK_ETW_01_007: [ log_sink_etw_log shall switch the state to REGISTERED. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_008: [ log_sink_etw_log shall emit a LOG_LEVEL_INFO event as a self test , printing the fact that the provider was registered and from which executable (as obtained by calling _get_pgmptr). ]*/
/* Tests_SRS_LOG_SINK_ETW_01_009: [ Checking and changing the variable that maintains whether TraceLoggingRegister was called shall be done using InterlockedCompareExchange and InterlockedExchange. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_010: [ log_sink_etw_log shall emit a self described event that shall have the name of the event as follows: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_012: [ If log_level is LOG_LEVEL_CRITICAL the event name shall be LogCritical. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_084: [ log_sink_etw_log shall use as provider GUID DAD29F36-0A48-4DEF-9D50-8EF9036B92B4. ]*/
static void log_sink_etw_log_registers_the_provider_if_not_registered_already(void)
{
    // arrange
    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_TraceLoggingRegister_EventRegister_EventSetInformation_call("DAD29F36-0A48-4DEF-9D50-8EF9036B92B4", TRACE_LEVEL_INFORMATION);
    setup_InterlockedExchange_call();

    // self test event
    setup__get_pgmptr_call();

    setup_InterlockedCompareExchange_call();
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes[256];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = TRACE_LEVEL_INFORMATION;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, "LogInfo", strlen("LogInfo") + 1);
    pos += strlen("LogInfo") + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    expected_calls[3].u._get_pgmptr_call.override_result = true;
    expected_calls[3].u._get_pgmptr_call.call_result = 0;
    expected_calls[3].u._get_pgmptr_call.injected_pValue = "some_test_executable.exe";

    static const char expected_event_message[] = "ETW provider was registered succesfully (self test). Executable file full path name = some_test_executable.exe";

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR expected_event_data_descriptors[6] =
    {
        { 0 },
        { 0 },
        {.Size = (ULONG)strlen(expected_event_message) + 1, .Ptr = (ULONGLONG)expected_event_message },
        {.Size = (ULONG)strlen(__FILE__) + 1, .Ptr = (ULONGLONG)__FILE__ },
        {.Size = (ULONG)strlen(__FUNCTION__) + 1, .Ptr = (ULONGLONG)__FUNCTION__ },
        {.Size = sizeof(int32_t), .Ptr = (ULONGLONG)&captured_line}
    };

    setup__tlgWriteTransfer_EventWriteTransfer_with_ignore_file_and_func(expected_event_metadata, 6, expected_event_data_descriptors, /* ignore file and func data */true);


    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, __LINE__, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_SINK_ETW_01_087: [ If the state is REGISTERING (2) log_sink_etw_log shall wait until the state is not REGISTERING (2). ]*/
static void log_sink_log_retries_until_state_is_not_registering(void)
{
    // arrange
    setup_mocks();
    setup_InterlockedCompareExchange_call();
    expected_calls[0].u.InterlockedCompareExchange_call.override_result = true;
    expected_calls[0].u.InterlockedCompareExchange_call.call_result = 2;
    setup_InterlockedCompareExchange_call();
    expected_calls[1].u.InterlockedCompareExchange_call.override_result = true;
    expected_calls[1].u.InterlockedCompareExchange_call.call_result = 2;
    setup_InterlockedCompareExchange_call();
    expected_calls[2].u.InterlockedCompareExchange_call.override_result = true;
    expected_calls[2].u.InterlockedCompareExchange_call.call_result = 2;
    setup_InterlockedCompareExchange_call();
    expected_calls[3].u.InterlockedCompareExchange_call.override_result = true;
    expected_calls[3].u.InterlockedCompareExchange_call.call_result = 3;

    // the actual event
    setup_vsnprintf_call(); // message formatting
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes_2[256];
    uint8_t expected_event_bytes_2[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes_2)];
    SELF_DESCRIBED_EVENT* expected_event_metadata_2 = (SELF_DESCRIBED_EVENT*)&expected_event_bytes_2[0];
    (void)memset(expected_event_metadata_2, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata_2->_tlgLevel = TRACE_LEVEL_CRITICAL;
    expected_event_metadata_2->_tlgChannel = 11;
    expected_event_metadata_2->_tlgOpcode = 0;
    expected_event_metadata_2->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes_2 + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, "LogCritical", strlen("LogCritical") + 1);
    pos += strlen("LogCritical") + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    expected_event_metadata_2->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes_2 + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR expected_event_data_descriptors_2[6] =
    {
        { 0 },
        { 0 },
        {.Size = (ULONG)strlen("test") + 1, .Ptr = (ULONGLONG)"test" },
        {.Size = (ULONG)strlen(__FILE__) + 1, .Ptr = (ULONGLONG)__FILE__ },
        {.Size = (ULONG)strlen(__FUNCTION__) + 1, .Ptr = (ULONGLONG)__FUNCTION__ },
        {.Size = sizeof(int32_t), .Ptr = (ULONGLONG)&captured_line}
    };

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata_2, 6, expected_event_data_descriptors_2);

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, captured_line, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_SINK_ETW_01_002: [ log_sink_etw_log shall maintain the state of whether TraceLoggingRegister was called in a variable accessed via InterlockedXXX APIs, which shall have 3 possible values: NOT_REGISTERED (1), REGISTERING (2), REGISTERED(3). ]*/
/* Tests_SRS_LOG_SINK_ETW_01_003: [ log_sink_etw_log shall perform the below actions until the provider is registered or an error is encountered: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_011: [ If the state is REGISTERED (3), log_sink_etw_log shall proceed to log the ETW event. ]*/
// Note the reason this is already registered is because previous tests have done the registration
/* Tests_SRS_LOG_SINK_ETW_01_010: [ log_sink_etw_log shall emit a self described event that shall have the name of the event as follows: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_025: [ If log_context is NULL only the fields content, file, func and line shall be added to the ETW event. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_042: [ log_sink_etw.log_sink_log shall compute the metadata size for the self described event metadata as follows: ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_043: [ Length of the event name (determined at compile time, excluding zero terminator) + 1. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_044: [ Length of the content field name (determined at compile time, excluding zero terminator) + 1. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_045: [ Length of the file field name (determined at compile time, excluding zero terminator) + 1. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_046: [ Length of the func field name (determined at compile time, excluding zero terminator) + 1. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_047: [ Length of the line field name (determined at compile time, excluding zero terminator) + 1. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_026: [ log_sink_etw.log_sink_log shall fill a SELF_DESCRIBED_EVENT structure, setting the following fields: ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_027: [ _tlgBlobTyp shall be set to _TlgBlobEvent4. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_028: [ _tlgChannel shall be set to 11. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_029: [ _tlgLevel shall be set to the appropriate logging level. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_030: [ _tlgOpcode shall be set to 0. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_031: [ _tlgKeyword shall be set to 0. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_032: [ _tlgEvtMetaSize shall be set to the computed metadata size + 4. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_033: [ _tlgEvtTag shall be set to 128. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_034: [ log_sink_etw.log_sink_log shall fill the event metadata: ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_035: [ The string content (as field name, excluding zero terminator), followed by one byte with the value TlgInANSISTRING. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_036: [ The string file (as field name, excluding zero terminator), followed by one byte with the value TlgInANSISTRING. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_037: [ The string func (as field name, excluding zero terminator), followed by one byte with the value TlgInANSISTRING. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_038: [ The string line (as field name, excluding zero terminator), followed by one byte with the value TlgInINT32. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_039: [ log_sink_etw.log_sink_log shall fill an EVENT_DATA_DESCRIPTOR array of size 2 + 1 + 1 + 1 + 1 + property count. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_040: [ log_sink_etw.log_sink_log shall set event data descriptor at index 2 by calling _tlgCreate1Sz_char with the value of the formatted message as obtained by using printf with the messages format message_format and the arguments in .... ]*/
/* Tests_SRS_LOG_SINK_ETW_01_041: [ log_sink_etw.log_sink_log shall emit the event by calling _tlgWriteTransfer_EventWriteTransfer passing the provider, channel, number of event data descriptors and the data descriptor array. ]*/
static void test_message_with_level(LOG_LEVEL log_level, uint8_t expected_tlg_level, const char* expected_event_name)
{
    // arrange
    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes[256];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = expected_tlg_level;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, expected_event_name, strlen(expected_event_name) + 1);
    pos += strlen(expected_event_name) + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;
    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR expected_event_data_descriptors[6] =
    {
        { 0 },
        { 0 },
        {.Size = (ULONG)strlen("test") + 1, .Ptr = (ULONGLONG)"test" },
        {.Size = (ULONG)strlen(__FILE__) + 1, .Ptr = (ULONGLONG)__FILE__ },
        {.Size = (ULONG)strlen(__FUNCTION__) + 1, .Ptr = (ULONGLONG)__FUNCTION__ },
        {.Size = sizeof(int32_t), .Ptr = (ULONGLONG) & captured_line}
    };

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6, expected_event_data_descriptors);

    // act
    log_sink_etw.log_sink_log(log_level, NULL, __FILE__, __FUNCTION__, captured_line, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_SINK_ETW_01_012: [ If log_level is LOG_LEVEL_CRITICAL the event name shall be LogCritical. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_018: [ Logging level: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_019: [ If log_level is LOG_LEVEL_CRITICAL the ETW logging level shall be TRACE_LEVEL_CRITICAL. ]*/
static void log_sink_etw_log_does_not_register_when_already_registered(void)
{
    test_message_with_level(LOG_LEVEL_CRITICAL, TRACE_LEVEL_CRITICAL, "LogCritical");
}

/* Tests_SRS_LOG_SINK_ETW_01_013: [ If log_level is LOG_LEVEL_ERROR the event name shall be LogError. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_020: [ If log_level is LOG_LEVEL_ERROR the ETW logging level shall be TRACE_LEVEL_ERROR. ]*/
static void log_sink_etw_log_with_LOG_LEVEL_ERROR_succeeds(void)
{
    test_message_with_level(LOG_LEVEL_ERROR, TRACE_LEVEL_ERROR, "LogError");
}

/* Tests_SRS_LOG_SINK_ETW_01_014: [ If log_level is LOG_LEVEL_WARNING the event name shall be LogWarning. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_021: [ If log_level is LOG_LEVEL_WARNING the ETW logging level shall be TRACE_LEVEL_WARNING. ]*/
static void log_sink_etw_log_with_LOG_LEVEL_WARNING_succeeds(void)
{
    test_message_with_level(LOG_LEVEL_WARNING, TRACE_LEVEL_WARNING, "LogWarning");
}

/* Tests_SRS_LOG_SINK_ETW_01_015: [ If log_level is LOG_LEVEL_INFO the event name shall be LogInfo. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_022: [ If log_level is LOG_LEVEL_INFO the ETW logging level shall be TRACE_LEVEL_INFO. ]*/
static void log_sink_etw_log_with_LOG_LEVEL_INFO_succeeds(void)
{
    test_message_with_level(LOG_LEVEL_INFO, TRACE_LEVEL_INFORMATION, "LogInfo");
}

/* Tests_SRS_LOG_SINK_ETW_01_016: [ If log_level is LOG_LEVEL_VERBOSE the event name shall be LogVerbose. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_023: [ If log_level is LOG_LEVEL_VERBOSE the ETW logging level shall be TRACE_LEVEL_VERBOSE. ]*/
static void log_sink_etw_log_with_LOG_LEVEL_VERBOSE_succeeds(void)
{
    test_message_with_level(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose");
}

/* Tests_SRS_LOG_SINK_ETW_01_017: [ Otherwise the event name shall be Unknown. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_024: [ Otherwise the ETW logging level shall be TRACE_LEVEL_NONE. ]*/
static void log_sink_etw_log_with_unknown_LOG_LEVEL_succeeds(void)
{
    test_message_with_level((LOG_LEVEL)0xFF, TRACE_LEVEL_NONE, "Unknown");
}

/* Tests_SRS_LOG_SINK_ETW_01_040: [ log_sink_etw.log_sink_log shall set event data descriptor at index 2 by calling _tlgCreate1Sz_char with the value of the formatted message as obtained by using printf with the messages format message_format and the arguments in .... ]*/
/* Tests_SRS_LOG_SINK_ETW_01_058: [ log_sink_etw.log_sink_log shall set event data descriptor at index 3 by calling _tlgCreate1Sz_char with file. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_059: [ log_sink_etw.log_sink_log shall set event data descriptor at index 4 by calling _tlgCreate1Sz_char with func. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_060: [ log_sink_etw.log_sink_log shall set event data descriptor at index 5 by calling EventDataDescCreate with line. ]*/
#define TEST_FORMATTED_MESSAGE_WITH_LEVEL(log_level, expected_tlg_level, expected_event_name, expected_message, message_format, ...) \
{                                                                                                                                    \
    /* arrange */                                                                                                                    \
    setup_enabled_provider(TRACE_LEVEL_VERBOSE);                                                                                     \
                                                                                                                                     \
    setup_mocks();                                                                                                                   \
    setup_InterlockedCompareExchange_call();                                                                                         \
    setup_vsnprintf_call(); /* formatting message */                                                                                 \
    setup__tlgCreate1Sz_char(); /* message */                                                                                        \
    setup__tlgCreate1Sz_char(); /* file */                                                                                           \
    setup__tlgCreate1Sz_char(); /* func */                                                                                           \
    setup_EventDataDescCreate(); /* line */                                                                                          \
    uint8_t extra_metadata_bytes[256];                                                                                               \
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];                                       \
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];                                 \
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));                                                          \
    expected_event_metadata->_tlgLevel = expected_tlg_level;                                                                         \
    expected_event_metadata->_tlgChannel = 11;                                                                                       \
    expected_event_metadata->_tlgOpcode = 0;                                                                                         \
    expected_event_metadata->_tlgKeyword = 0;                                                                                        \
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));                                                            \
    /* event name */                                                                                                                 \
    (void)memcpy(pos, expected_event_name, strlen(expected_event_name) + 1);                                                         \
    pos += strlen(expected_event_name) + 1;                                                                                          \
    /* content field */                                                                                                              \
    (void)memcpy(pos, "content", strlen("content") + 1);                                                                             \
    pos += strlen("content") + 1;                                                                                                    \
    *pos = TlgInANSISTRING;                                                                                                          \
    pos++;                                                                                                                           \
    /* content field */                                                                                                              \
    (void)memcpy(pos, "file", strlen("file") + 1);                                                                                   \
    pos += strlen("file") + 1;                                                                                                       \
    *pos = TlgInANSISTRING;                                                                                                          \
    pos++;                                                                                                                           \
    /* content field */                                                                                                              \
    (void)memcpy(pos, "func", strlen("func") + 1);                                                                                   \
    pos += strlen("func") + 1;                                                                                                       \
    *pos = TlgInANSISTRING;                                                                                                          \
    pos++;                                                                                                                           \
    /* content field */                                                                                                              \
    (void)memcpy(pos, "line", strlen("line") + 1);                                                                                   \
    pos += strlen("line") + 1;                                                                                                       \
    *pos = TlgInINT32;                                                                                                               \
    pos++;                                                                                                                           \
    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;          \
                                                                                                                                     \
    int captured_line = __LINE__;                                                                                                    \
                                                                                                                                     \
    /* construct event data descriptor array */                                                                                      \
    EVENT_DATA_DESCRIPTOR expected_event_data_descriptors[6] =                                                                       \
    {                                                                                                                                \
        { 0 },                                                                                                                       \
        { 0 },                                                                                                                       \
        { .Size = (ULONG)strlen(expected_message) + 1, .Ptr = (ULONGLONG)expected_message},                                          \
        {.Size = (ULONG)strlen(__FILE__) + 1, .Ptr = (ULONGLONG)__FILE__ },                                                          \
        {.Size = (ULONG)strlen(__FUNCTION__) + 1, .Ptr = (ULONGLONG)__FUNCTION__ },                                                  \
        {.Size = sizeof(int32_t), .Ptr = (ULONGLONG)&captured_line}                                                                  \
    };                                                                                                                               \
                                                                                                                                     \
    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6, expected_event_data_descriptors);                         \
                                                                                                                                     \
    /* act */                                                                                                                        \
    log_sink_etw.log_sink_log(log_level, NULL, __FILE__, __FUNCTION__, captured_line, message_format, __VA_ARGS__);                  \
                                                                                                                                     \
    /* assert */                                                                                                                     \
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);                                                                      \
    POOR_MANS_ASSERT(actual_and_expected_match);                                                                                     \
}

/* Tests_SRS_LOG_SINK_ETW_01_040: [ log_sink_etw.log_sink_log shall set event data descriptor at index 2 by calling _tlgCreate1Sz_char with the value of the formatted message as obtained by using printf with the messages format message_format and the arguments in .... ]*/
static void log_sink_etw_log_with_LOG_LEVEL_CRITICAL_format_message_succeeds(void)
{
    TEST_FORMATTED_MESSAGE_WITH_LEVEL(LOG_LEVEL_CRITICAL, TRACE_LEVEL_CRITICAL, "LogCritical", "test_value=42", "test_value=%d", 42);
}

/* Tests_SRS_LOG_SINK_ETW_01_040: [ log_sink_etw.log_sink_log shall set event data descriptor at index 2 by calling _tlgCreate1Sz_char with the value of the formatted message as obtained by using printf with the messages format message_format and the arguments in .... ]*/
static void log_sink_etw_log_with_LOG_LEVEL_CRITICAL_format_message_succeeds_2(void)
{
    TEST_FORMATTED_MESSAGE_WITH_LEVEL(LOG_LEVEL_CRITICAL, TRACE_LEVEL_CRITICAL, "LogCritical", "test_value_string=blah, test_value=43", "test_value_string=%s, test_value=%d", "blah", 43);
}

static void test_message_with_context(LOG_LEVEL log_level, uint8_t expected_tlg_level, const char* expected_event_name, LOG_CONTEXT_HANDLE log_context, const char* expected_context_name)
{
    // arrange
    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes[256];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = expected_tlg_level;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, expected_event_name, strlen(expected_event_name) + 1);
    pos += strlen(expected_event_name) + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    // content field
    (void)memcpy(pos, expected_context_name, strlen(expected_context_name) + 1);
    pos += strlen(expected_context_name) + 1;
    *pos = _TlgInSTRUCT | _TlgInChain;
    pos++;
    *pos = 0; // struct field count
    pos++;

    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR expected_event_data_descriptors[6] =
    {
        { 0 },
        { 0 },
        {.Size = (ULONG)strlen("test") + 1, .Ptr = (ULONGLONG)"test" },
        {.Size = (ULONG)strlen(__FILE__) + 1, .Ptr = (ULONGLONG)__FILE__ },
        {.Size = (ULONG)strlen(__FUNCTION__) + 1, .Ptr = (ULONGLONG)__FUNCTION__ },
        {.Size = sizeof(int32_t), .Ptr = (ULONGLONG)&captured_line}
    };

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6, expected_event_data_descriptors);

    // act
    log_sink_etw.log_sink_log(log_level, log_context, __FILE__, __FUNCTION__, captured_line, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_SINK_ETW_01_048: [ If log_context is not NULL: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_049: [ log_sink_etw.log_sink_log shall call log_context_get_property_value_pair_count to obtain the count of properties that are to be added to the ETW event. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_050: [ log_sink_etw.log_sink_log shall call log_context_get_property_value_pairs to obtain the properties that are to be added to the ETW event. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_051: [ For each property in log_context, the length of the property name + 1 and one extra byte for the type of the field. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_052: [ For struct properties one extra byte shall be added for the field count. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_053: [ For each property in log_context the following shall be added to the event metadata: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_054: [ A string with the property name (including zero terminator) ]*/
/* Tests_SRS_LOG_SINK_ETW_01_055: [ A byte with the type of property, as follows: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_056: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_struct, a byte with the value _TlgInSTRUCT | _TlgInChain shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_057: [ If the property is a struct, an extra byte shall be added in the metadata containing the number of fields in the structure. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_061: [ For each property in log_context: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_062: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_struct, no event data descriptor shall be used. ]*/
static void log_sink_etw_log_with_context_with_no_properties_succeeds(void)
{
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL);

    test_message_with_context(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose", log_context, "");

    LOG_CONTEXT_DESTROY(log_context);
}

static void test_message_with_context_with_one_property(LOG_LEVEL log_level, uint8_t expected_tlg_level, const char* expected_event_name, LOG_CONTEXT_HANDLE log_context, const char* expected_context_name, uint32_t expected_property_size)
{
    // arrange
    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes[256];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = expected_tlg_level;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, expected_event_name, strlen(expected_event_name) + 1);
    pos += strlen(expected_event_name) + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* log_context_property_value_pairs = log_context_get_property_value_pairs(log_context);

    // content field
    (void)memcpy(pos, expected_context_name, strlen(expected_context_name) + 1);
    pos += strlen(expected_context_name) + 1;
    *pos = _TlgInSTRUCT | _TlgInChain;
    pos++;
    *pos = 1;
    pos++;

    (void)memcpy(pos, log_context_property_value_pairs[1].name, strlen(log_context_property_value_pairs[1].name) + 1);
    pos += strlen(log_context_property_value_pairs[1].name) + 1;
    switch (log_context_property_value_pairs[1].type->get_type())
    {
        default:
            break;
        case LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr:
        {
            setup__tlgCreate1Sz_char();
            *pos = TlgInANSISTRING;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int64_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT64;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint64_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT64;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int32_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT32;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint32_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT32;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int16_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT16;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint16_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT16;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int8_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT8;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint8_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT8;
            break;
        }
    }
    pos++;

    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR expected_event_data_descriptors[7] =
    {
        { 0 },
        { 0 },
        {.Size = (ULONG)strlen("test") + 1, .Ptr = (ULONGLONG)"test" },
        {.Size = (ULONG)strlen(__FILE__) + 1, .Ptr = (ULONGLONG)__FILE__ },
        {.Size = (ULONG)strlen(__FUNCTION__) + 1, .Ptr = (ULONGLONG)__FUNCTION__ },
        {.Size = sizeof(int32_t), .Ptr = (ULONGLONG)&captured_line},
        {.Size = expected_property_size, .Ptr = (ULONGLONG)log_context_property_value_pairs[1].value}
    };

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 7, expected_event_data_descriptors);

    // act
    log_sink_etw.log_sink_log(log_level, log_context, __FILE__, __FUNCTION__, captured_line, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_SINK_ETW_01_063: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr, a byte with the value TlgInANSISTRING shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_067: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr, the event data descriptor shall be filled with the value of the property by calling _tlgCreate1Sz_char. ]*/
static void log_sink_etw_log_with_context_with_one_ascii_property_succeeds(void)
{
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(gigi, "duru")
        );

    test_message_with_context_with_one_property(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose", log_context, "",
        5 // expected property value size
    );

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_064: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int64_t, a byte with the value TlgInINT64 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_066: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int64_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
static void log_sink_etw_log_with_context_with_one_int64_t_property_succeeds(void)
{
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(int64_t, gigi, 42)
    );

    test_message_with_context_with_one_property(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose", log_context, "",
        sizeof(int64_t) // expected property value size
    );

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_065: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint64_t, a byte with the value TlgInUINT64 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_068: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint64_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
static void log_sink_etw_log_with_context_with_one_uint64_t_property_succeeds(void)
{
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint64_t, gigi, 42)
    );

    test_message_with_context_with_one_property(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose", log_context, "",
        sizeof(uint64_t) // expected property value size
    );

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_069: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int32_t, a byte with the value TlgInINT32 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_070: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int32_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
static void log_sink_etw_log_with_context_with_one_int32_t_property_succeeds(void)
{
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, gigi, 42)
    );

    test_message_with_context_with_one_property(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose", log_context, "",
        sizeof(int32_t) // expected property value size
    );

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_071: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint32_t, a byte with the value TlgInUINT32 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_072: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint32_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
static void log_sink_etw_log_with_context_with_one_uint32_t_property_succeeds(void)
{
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint32_t, gigi, 42)
    );

    test_message_with_context_with_one_property(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose", log_context, "",
        sizeof(uint32_t) // expected property value size
    );

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_073: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int16_t, a byte with the value TlgInINT16 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_074: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int16_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
static void log_sink_etw_log_with_context_with_one_int16_t_property_succeeds(void)
{
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(int16_t, gigi, 42)
    );

    test_message_with_context_with_one_property(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose", log_context, "",
        sizeof(int16_t) // expected property value size
    );

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_075: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint16_t, a byte with the value TlgInUINT16 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_076: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint16_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
static void log_sink_etw_log_with_context_with_one_uint16_t_property_succeeds(void)
{
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint16_t, gigi, 42)
    );

    test_message_with_context_with_one_property(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose", log_context, "",
        sizeof(uint16_t) // expected property value size
    );

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_077: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int8_t, a byte with the value TlgInINT8 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_078: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int8_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
static void log_sink_etw_log_with_context_with_one_int8_t_property_succeeds(void)
{
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(int8_t, gigi, 42)
    );

    test_message_with_context_with_one_property(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose", log_context, "",
        sizeof(int8_t) // expected property value size
    );

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_079: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint8_t, a byte with the value TlgInUINT8 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_080: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint8_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
static void log_sink_etw_log_with_context_with_one_uint8_t_property_succeeds(void)
{
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint8_t, gigi, 42)
    );

    test_message_with_context_with_one_property(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose", log_context, "",
        sizeof(uint8_t) // expected property value size
    );

    LOG_CONTEXT_DESTROY(log_context);
}

static void test_message_with_context_with_multiple_properties(LOG_LEVEL log_level, uint8_t expected_tlg_level, const char* expected_event_name, LOG_CONTEXT_HANDLE log_context, const char* expected_context_name, uint8_t property_count)
{
    // arrange
    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes[256];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = expected_tlg_level;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, expected_event_name, strlen(expected_event_name) + 1);
    pos += strlen(expected_event_name) + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* log_context_property_value_pairs = log_context_get_property_value_pairs(log_context);

    // content field
    (void)memcpy(pos, expected_context_name, strlen(expected_context_name) + 1);
    pos += strlen(expected_context_name) + 1;
    *pos = _TlgInSTRUCT | _TlgInChain;
    pos++;
    *pos = property_count;
    pos++;

    for (uint32_t i = 0; i < property_count; i++)
    {
        (void)memcpy(pos, log_context_property_value_pairs[i + 1].name, strlen(log_context_property_value_pairs[i + 1].name) + 1);
        pos += strlen(log_context_property_value_pairs[i + 1].name) + 1;
        switch (log_context_property_value_pairs[i + 1].type->get_type())
        {
        default:
            break;
        case LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr:
        {
            setup__tlgCreate1Sz_char();
            *pos = TlgInANSISTRING;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int64_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT64;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint64_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT64;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int32_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT32;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint32_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT32;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int16_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT16;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint16_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT16;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int8_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT8;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint8_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT8;
            break;
        }
        }
        pos++;
    }

    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR* expected_event_data_descriptors = malloc(sizeof(EVENT_DATA_DESCRIPTOR) * (6 + property_count));
    POOR_MANS_ASSERT(expected_event_data_descriptors != NULL);
    (void)memset(expected_event_data_descriptors, 0, sizeof(EVENT_DATA_DESCRIPTOR) * (6 + property_count));
    expected_event_data_descriptors[2].Size = (ULONG)strlen("test") + 1;
    expected_event_data_descriptors[2].Ptr = (ULONGLONG)"test";
    expected_event_data_descriptors[3].Size = (ULONG)strlen(__FILE__) + 1;
    expected_event_data_descriptors[3].Ptr = (ULONGLONG)__FILE__;
    expected_event_data_descriptors[4].Size = (ULONG)strlen(__FUNCTION__) + 1;
    expected_event_data_descriptors[4].Ptr = (ULONGLONG)__FUNCTION__;
    expected_event_data_descriptors[5].Size = sizeof(int32_t);
    expected_event_data_descriptors[5].Ptr = (ULONGLONG)&captured_line;

    for (uint32_t i = 0; i < property_count; i++)
    {
        ULONG expected_property_size;
        switch (log_context_property_value_pairs[i + 1].type->get_type())
        {
        default:
            expected_property_size = 0;
            break;

        case LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr:
        {
            expected_property_size = (ULONG)(strlen(log_context_property_value_pairs[i + 1].value) + 1);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int64_t:
        {
            expected_property_size = sizeof(int64_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint64_t:
        {
            expected_property_size = sizeof(uint64_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int32_t:
        {
            expected_property_size = sizeof(int32_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint32_t:
        {
            expected_property_size = sizeof(uint32_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int16_t:
        {
            expected_property_size = sizeof(int16_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint16_t:
        {
            expected_property_size = sizeof(uint16_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int8_t:
        {
            expected_property_size = sizeof(int8_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint8_t:
        {
            expected_property_size = sizeof(uint8_t);
            break;
        }
        }
        expected_event_data_descriptors[6 + i].Size = expected_property_size;
        expected_event_data_descriptors[6 + i].Ptr = (ULONGLONG)log_context_property_value_pairs[i + 1].value;
    }

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6 + property_count, expected_event_data_descriptors);

    // act
    log_sink_etw.log_sink_log(log_level, log_context, __FILE__, __FUNCTION__, captured_line, "test");

    free(expected_event_data_descriptors);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_SINK_ETW_01_063: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr, a byte with the value TlgInANSISTRING shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_067: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr, the event data descriptor shall be filled with the value of the property by calling _tlgCreate1Sz_char. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_064: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int64_t, a byte with the value TlgInINT64 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_066: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int64_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_065: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint64_t, a byte with the value TlgInUINT64 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_068: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint64_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_071: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint32_t, a byte with the value TlgInUINT32 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_072: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint32_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_069: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int32_t, a byte with the value TlgInINT32 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_070: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int32_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_073: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int16_t, a byte with the value TlgInINT16 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_074: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int16_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_075: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint16_t, a byte with the value TlgInUINT16 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_076: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint16_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_077: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int8_t, a byte with the value TlgInINT8 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_078: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int8_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_079: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint8_t, a byte with the value TlgInUINT8 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_080: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint8_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
static void log_sink_etw_log_with_context_with_all_property_types_succeeds(void)
{
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(prop1, "haha"),
        LOG_CONTEXT_PROPERTY(int64_t, prop2, 42),
        LOG_CONTEXT_PROPERTY(uint64_t, prop3, 43),
        LOG_CONTEXT_PROPERTY(int32_t, prop4, 44),
        LOG_CONTEXT_PROPERTY(uint32_t, prop5, 45),
        LOG_CONTEXT_PROPERTY(int16_t, prop6, 46),
        LOG_CONTEXT_PROPERTY(uint16_t, prop7, 47),
        LOG_CONTEXT_PROPERTY(int8_t, prop8, 48),
        LOG_CONTEXT_PROPERTY(uint8_t, prop9, 49)
        );

    test_message_with_context_with_multiple_properties(LOG_LEVEL_VERBOSE, TRACE_LEVEL_VERBOSE, "LogVerbose", log_context, "", 9);

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_081: [ If the property type is any other value, no property data shall be added to the event. ]*/
static void when_unknown_property_type_is_encountered_log_sink_etw_log_with_context_does_not_place_any_properties_in_the_event(void)
{
    // arrange
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint8_t, gigi, 42)
    );

    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_property_if_get_type(); // one mocked call to get_type
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes[256];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = TRACE_LEVEL_VERBOSE;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, "LogVerbose", strlen("LogVerbose") + 1);
    pos += strlen("LogVerbose") + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* log_context_property_value_pairs = log_context_get_property_value_pairs(log_context);
    LOG_CONTEXT_PROPERTY_VALUE_PAIR* mocked_property_value_pairs = malloc(sizeof(LOG_CONTEXT_PROPERTY_VALUE_PAIR) * 2);
    POOR_MANS_ASSERT(mocked_property_value_pairs != NULL);

    LOG_CONTEXT_PROPERTY_TYPE_IF mocked_property_type_if;
    mocked_property_type_if.get_type = mock_log_context_property_if_get_type;

    (void)memcpy(mocked_property_value_pairs, log_context_property_value_pairs, sizeof(LOG_CONTEXT_PROPERTY_VALUE_PAIR) * 2);

    // hard injecting a mock here :-)
    mocked_property_value_pairs[1].type = &mocked_property_type_if;

    expected_calls[1].u.log_context_get_property_value_pairs_call.override_result = true;
    expected_calls[1].u.log_context_get_property_value_pairs_call.call_result = mocked_property_value_pairs;

    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR expected_event_data_descriptors[7] =
    {
        { 0 },
        { 0 },
        {.Size = (ULONG)strlen("test") + 1, .Ptr = (ULONGLONG)"test" },
        {.Size = (ULONG)strlen(__FILE__) + 1, .Ptr = (ULONGLONG)__FILE__ },
        {.Size = (ULONG)strlen(__FUNCTION__) + 1, .Ptr = (ULONGLONG)__FUNCTION__ },
        {.Size = sizeof(int32_t), .Ptr = (ULONGLONG)&captured_line}
    };

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6, expected_event_data_descriptors);

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, log_context, __FILE__, __FUNCTION__, captured_line, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    LOG_CONTEXT_DESTROY(log_context);

    free(mocked_property_value_pairs);
}

/* Tests_SRS_LOG_SINK_ETW_01_082: [ If more than 64 properties are given in log_context, log_sink_etw.log_sink_log shall not add any properties to the event. ]*/
static void when_more_than_64_properties_are_passed_in_context_log_sink_etw_log_with_context_does_not_place_any_properties_in_the_event(void)
{
    // arrange
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint8_t, prop1, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, prop2, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, prop3, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, prop4, 4),
        LOG_CONTEXT_PROPERTY(uint8_t, prop5, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, prop6, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, prop7, 7),
        LOG_CONTEXT_PROPERTY(uint8_t, prop8, 8),
        LOG_CONTEXT_PROPERTY(uint8_t, prop9, 9),
        LOG_CONTEXT_PROPERTY(uint8_t, prop10, 10),
        LOG_CONTEXT_PROPERTY(uint8_t, prop11, 11),
        LOG_CONTEXT_PROPERTY(uint8_t, prop12, 12),
        LOG_CONTEXT_PROPERTY(uint8_t, prop13, 13),
        LOG_CONTEXT_PROPERTY(uint8_t, prop14, 14),
        LOG_CONTEXT_PROPERTY(uint8_t, prop15, 15),
        LOG_CONTEXT_PROPERTY(uint8_t, prop16, 16),
        LOG_CONTEXT_PROPERTY(uint8_t, prop17, 17),
        LOG_CONTEXT_PROPERTY(uint8_t, prop18, 18),
        LOG_CONTEXT_PROPERTY(uint8_t, prop19, 19),
        LOG_CONTEXT_PROPERTY(uint8_t, prop20, 20),
        LOG_CONTEXT_PROPERTY(uint8_t, prop21, 21),
        LOG_CONTEXT_PROPERTY(uint8_t, prop22, 22),
        LOG_CONTEXT_PROPERTY(uint8_t, prop23, 23),
        LOG_CONTEXT_PROPERTY(uint8_t, prop24, 24),
        LOG_CONTEXT_PROPERTY(uint8_t, prop25, 25),
        LOG_CONTEXT_PROPERTY(uint8_t, prop26, 26),
        LOG_CONTEXT_PROPERTY(uint8_t, prop27, 27),
        LOG_CONTEXT_PROPERTY(uint8_t, prop28, 28),
        LOG_CONTEXT_PROPERTY(uint8_t, prop29, 29),
        LOG_CONTEXT_PROPERTY(uint8_t, prop30, 30),
        LOG_CONTEXT_PROPERTY(uint8_t, prop31, 31),
        LOG_CONTEXT_PROPERTY(uint8_t, prop32, 32),
        LOG_CONTEXT_PROPERTY(uint8_t, prop33, 33),
        LOG_CONTEXT_PROPERTY(uint8_t, prop34, 34),
        LOG_CONTEXT_PROPERTY(uint8_t, prop35, 35),
        LOG_CONTEXT_PROPERTY(uint8_t, prop36, 36),
        LOG_CONTEXT_PROPERTY(uint8_t, prop37, 37),
        LOG_CONTEXT_PROPERTY(uint8_t, prop38, 38),
        LOG_CONTEXT_PROPERTY(uint8_t, prop39, 39),
        LOG_CONTEXT_PROPERTY(uint8_t, prop40, 40),
        LOG_CONTEXT_PROPERTY(uint8_t, prop41, 41),
        LOG_CONTEXT_PROPERTY(uint8_t, prop42, 42),
        LOG_CONTEXT_PROPERTY(uint8_t, prop43, 43),
        LOG_CONTEXT_PROPERTY(uint8_t, prop44, 44),
        LOG_CONTEXT_PROPERTY(uint8_t, prop45, 45),
        LOG_CONTEXT_PROPERTY(uint8_t, prop46, 46),
        LOG_CONTEXT_PROPERTY(uint8_t, prop47, 47),
        LOG_CONTEXT_PROPERTY(uint8_t, prop48, 48),
        LOG_CONTEXT_PROPERTY(uint8_t, prop49, 49),
        LOG_CONTEXT_PROPERTY(uint8_t, prop50, 50),
        LOG_CONTEXT_PROPERTY(uint8_t, prop51, 51),
        LOG_CONTEXT_PROPERTY(uint8_t, prop52, 52),
        LOG_CONTEXT_PROPERTY(uint8_t, prop53, 53),
        LOG_CONTEXT_PROPERTY(uint8_t, prop54, 54),
        LOG_CONTEXT_PROPERTY(uint8_t, prop55, 55),
        LOG_CONTEXT_PROPERTY(uint8_t, prop56, 56),
        LOG_CONTEXT_PROPERTY(uint8_t, prop57, 57),
        LOG_CONTEXT_PROPERTY(uint8_t, prop58, 58),
        LOG_CONTEXT_PROPERTY(uint8_t, prop59, 59),
        LOG_CONTEXT_PROPERTY(uint8_t, prop60, 60),
        LOG_CONTEXT_PROPERTY(uint8_t, prop61, 61),
        LOG_CONTEXT_PROPERTY(uint8_t, prop62, 62),
        LOG_CONTEXT_PROPERTY(uint8_t, prop63, 63),
        LOG_CONTEXT_PROPERTY(uint8_t, prop64, 64) // 64 because there is one extra property for the context which is of type struct
        );

    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes[256];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = TRACE_LEVEL_VERBOSE;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, "LogVerbose", strlen("LogVerbose") + 1);
    pos += strlen("LogVerbose") + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR expected_event_data_descriptors[7] =
    {
        { 0 },
        { 0 },
        {.Size = (ULONG)strlen("test") + 1, .Ptr = (ULONGLONG)"test" },
        {.Size = (ULONG)strlen(__FILE__) + 1, .Ptr = (ULONGLONG)__FILE__ },
        {.Size = (ULONG)strlen(__FUNCTION__) + 1, .Ptr = (ULONGLONG)__FUNCTION__ },
        {.Size = sizeof(int32_t), .Ptr = (ULONGLONG)&captured_line}
    };

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6, expected_event_data_descriptors);

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, log_context, __FILE__, __FUNCTION__, captured_line, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_082: [ If more than 64 properties are given in log_context, log_sink_etw.log_sink_log shall not add any properties to the event. ]*/
static void when_exactly_64_properties_are_passed_in_context_log_sink_etw_log_with_context_succeeds_and_emits_fields_for_each_property(void)
{
    // arrange
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint8_t, prop1, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, prop2, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, prop3, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, prop4, 4),
        LOG_CONTEXT_PROPERTY(uint8_t, prop5, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, prop6, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, prop7, 7),
        LOG_CONTEXT_PROPERTY(uint8_t, prop8, 8),
        LOG_CONTEXT_PROPERTY(uint8_t, prop9, 9),
        LOG_CONTEXT_PROPERTY(uint8_t, prop10, 10),
        LOG_CONTEXT_PROPERTY(uint8_t, prop11, 11),
        LOG_CONTEXT_PROPERTY(uint8_t, prop12, 12),
        LOG_CONTEXT_PROPERTY(uint8_t, prop13, 13),
        LOG_CONTEXT_PROPERTY(uint8_t, prop14, 14),
        LOG_CONTEXT_PROPERTY(uint8_t, prop15, 15),
        LOG_CONTEXT_PROPERTY(uint8_t, prop16, 16),
        LOG_CONTEXT_PROPERTY(uint8_t, prop17, 17),
        LOG_CONTEXT_PROPERTY(uint8_t, prop18, 18),
        LOG_CONTEXT_PROPERTY(uint8_t, prop19, 19),
        LOG_CONTEXT_PROPERTY(uint8_t, prop20, 20),
        LOG_CONTEXT_PROPERTY(uint8_t, prop21, 21),
        LOG_CONTEXT_PROPERTY(uint8_t, prop22, 22),
        LOG_CONTEXT_PROPERTY(uint8_t, prop23, 23),
        LOG_CONTEXT_PROPERTY(uint8_t, prop24, 24),
        LOG_CONTEXT_PROPERTY(uint8_t, prop25, 25),
        LOG_CONTEXT_PROPERTY(uint8_t, prop26, 26),
        LOG_CONTEXT_PROPERTY(uint8_t, prop27, 27),
        LOG_CONTEXT_PROPERTY(uint8_t, prop28, 28),
        LOG_CONTEXT_PROPERTY(uint8_t, prop29, 29),
        LOG_CONTEXT_PROPERTY(uint8_t, prop30, 30),
        LOG_CONTEXT_PROPERTY(uint8_t, prop31, 31),
        LOG_CONTEXT_PROPERTY(uint8_t, prop32, 32),
        LOG_CONTEXT_PROPERTY(uint8_t, prop33, 33),
        LOG_CONTEXT_PROPERTY(uint8_t, prop34, 34),
        LOG_CONTEXT_PROPERTY(uint8_t, prop35, 35),
        LOG_CONTEXT_PROPERTY(uint8_t, prop36, 36),
        LOG_CONTEXT_PROPERTY(uint8_t, prop37, 37),
        LOG_CONTEXT_PROPERTY(uint8_t, prop38, 38),
        LOG_CONTEXT_PROPERTY(uint8_t, prop39, 39),
        LOG_CONTEXT_PROPERTY(uint8_t, prop40, 40),
        LOG_CONTEXT_PROPERTY(uint8_t, prop41, 41),
        LOG_CONTEXT_PROPERTY(uint8_t, prop42, 42),
        LOG_CONTEXT_PROPERTY(uint8_t, prop43, 43),
        LOG_CONTEXT_PROPERTY(uint8_t, prop44, 44),
        LOG_CONTEXT_PROPERTY(uint8_t, prop45, 45),
        LOG_CONTEXT_PROPERTY(uint8_t, prop46, 46),
        LOG_CONTEXT_PROPERTY(uint8_t, prop47, 47),
        LOG_CONTEXT_PROPERTY(uint8_t, prop48, 48),
        LOG_CONTEXT_PROPERTY(uint8_t, prop49, 49),
        LOG_CONTEXT_PROPERTY(uint8_t, prop50, 50),
        LOG_CONTEXT_PROPERTY(uint8_t, prop51, 51),
        LOG_CONTEXT_PROPERTY(uint8_t, prop52, 52),
        LOG_CONTEXT_PROPERTY(uint8_t, prop53, 53),
        LOG_CONTEXT_PROPERTY(uint8_t, prop54, 54),
        LOG_CONTEXT_PROPERTY(uint8_t, prop55, 55),
        LOG_CONTEXT_PROPERTY(uint8_t, prop56, 56),
        LOG_CONTEXT_PROPERTY(uint8_t, prop57, 57),
        LOG_CONTEXT_PROPERTY(uint8_t, prop58, 58),
        LOG_CONTEXT_PROPERTY(uint8_t, prop59, 59),
        LOG_CONTEXT_PROPERTY(uint8_t, prop60, 60),
        LOG_CONTEXT_PROPERTY(uint8_t, prop61, 61),
        LOG_CONTEXT_PROPERTY(uint8_t, prop62, 62),
        LOG_CONTEXT_PROPERTY(uint8_t, prop63, 63) // 63 because there is one extra property for the context which is of type struct
    );

    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes[8192];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = TRACE_LEVEL_VERBOSE;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, "LogVerbose", strlen("LogVerbose") + 1);
    pos += strlen("LogVerbose") + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* log_context_property_value_pairs = log_context_get_property_value_pairs(log_context);
    uint8_t property_count = (uint8_t)log_context_get_property_value_pair_count(log_context) - 1;

    // content field
    (void)memcpy(pos, "", strlen("") + 1);
    pos += strlen("") + 1;
    *pos = _TlgInSTRUCT | _TlgInChain;
    pos++;
    *pos = property_count;
    pos++;

    for (uint32_t i = 0; i < property_count; i++)
    {
        (void)memcpy(pos, log_context_property_value_pairs[i + 1].name, strlen(log_context_property_value_pairs[i + 1].name) + 1);
        pos += strlen(log_context_property_value_pairs[i + 1].name) + 1;
        switch (log_context_property_value_pairs[i + 1].type->get_type())
        {
        default:
            break;
        case LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr:
        {
            setup__tlgCreate1Sz_char();
            *pos = TlgInANSISTRING;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int64_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT64;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint64_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT64;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int32_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT32;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint32_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT32;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int16_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT16;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint16_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT16;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int8_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT8;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint8_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT8;
            break;
        }
        }
        pos++;
    }

    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR* expected_event_data_descriptors = malloc(sizeof(EVENT_DATA_DESCRIPTOR) * (6 + property_count));
    POOR_MANS_ASSERT(expected_event_data_descriptors != NULL);
    (void)memset(expected_event_data_descriptors, 0, sizeof(EVENT_DATA_DESCRIPTOR) * (6 + property_count));
    expected_event_data_descriptors[2].Size = (ULONG)strlen("test") + 1;
    expected_event_data_descriptors[2].Ptr = (ULONGLONG)"test";
    expected_event_data_descriptors[3].Size = (ULONG)strlen(__FILE__) + 1;
    expected_event_data_descriptors[3].Ptr = (ULONGLONG)__FILE__;
    expected_event_data_descriptors[4].Size = (ULONG)strlen(__FUNCTION__) + 1;
    expected_event_data_descriptors[4].Ptr = (ULONGLONG)__FUNCTION__;
    expected_event_data_descriptors[5].Size = sizeof(int32_t);
    expected_event_data_descriptors[5].Ptr = (ULONGLONG)&captured_line;

    for (uint32_t i = 0; i < property_count; i++)
    {
        ULONG expected_property_size;
        switch (log_context_property_value_pairs[i + 1].type->get_type())
        {
        default:
            expected_property_size = 0;
            break;

        case LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr:
        {
            expected_property_size = (ULONG)(strlen(log_context_property_value_pairs[i + 1].value) + 1);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int64_t:
        {
            expected_property_size = sizeof(int64_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint64_t:
        {
            expected_property_size = sizeof(uint64_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int32_t:
        {
            expected_property_size = sizeof(int32_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint32_t:
        {
            expected_property_size = sizeof(uint32_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int16_t:
        {
            expected_property_size = sizeof(int16_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint16_t:
        {
            expected_property_size = sizeof(uint16_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int8_t:
        {
            expected_property_size = sizeof(int8_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint8_t:
        {
            expected_property_size = sizeof(uint8_t);
            break;
        }
        }
        expected_event_data_descriptors[6 + i].Size = expected_property_size;
        expected_event_data_descriptors[6 + i].Ptr = (ULONGLONG)log_context_property_value_pairs[i + 1].value;
    }

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6 + property_count, expected_event_data_descriptors);

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, log_context, __FILE__, __FUNCTION__, captured_line, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    LOG_CONTEXT_DESTROY(log_context);

    free(expected_event_data_descriptors);
}

/* Tests_SRS_LOG_SINK_ETW_01_085: [ If the size of the metadata and the formatted message exceeds 4096 bytes, log_sink_etw.log_sink_log shall not add any properties to the event. ]*/
static void when_size_of_metadata_exceeds_4096_log_sink_etw_log_with_context_does_not_place_any_properties_in_the_event(void)
{
    // arrange
    LOG_CONTEXT_HANDLE log_context;

    // 38 bytes are in metadata by default with the setting below, 3 extra bytes for the struct that holds all properties, makes 41 bytes
    // Need 4055 bytes of properties metadata to go over
    // 
    // Each line below adds 150 bytes, 27 * 150 = 4050, last line needs 5 bytes to get to exactly the limit, 6 bytes goes over the limit
    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________1, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________2, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________3, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________4, 4),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________5, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________6, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________7, 7),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________8, 8),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________9, 9),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________10, 10),
        // 1500
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________11, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________12, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________13, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________14, 4),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________15, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________16, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________17, 7),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________18, 8),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________19, 9),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________20, 10),
        // 3000
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________21, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________22, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________23, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________24, 4),
        // 3600
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________25, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________26, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________27, 7),
        // 4050
        LOG_CONTEXT_PROPERTY(uint8_t, prop, 8) // 6 bytes
        // 4056
    );

    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes[4096];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = TRACE_LEVEL_VERBOSE;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, "LogVerbose", strlen("LogVerbose") + 1);
    pos += strlen("LogVerbose") + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR expected_event_data_descriptors[6] =
    {
        { 0 },
        { 0 },
        {.Size = (ULONG)strlen("") + 1, .Ptr = (ULONGLONG)"" },
        {.Size = (ULONG)strlen(__FILE__) + 1, .Ptr = (ULONGLONG)__FILE__ },
        {.Size = (ULONG)strlen(__FUNCTION__) + 1, .Ptr = (ULONGLONG)__FUNCTION__ },
        {.Size = sizeof(int32_t), .Ptr = (ULONGLONG)&captured_line}
    };

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6, expected_event_data_descriptors);

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, log_context, __FILE__, __FUNCTION__, captured_line, "");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_085: [ If the size of the metadata and the formatted message exceeds 4096 bytes, log_sink_etw.log_sink_log shall not add any properties to the event. ]*/
static void when_size_of_metadata_and_formatted_messages_exceeds_4096_log_sink_etw_log_with_context_does_not_place_any_properties_in_the_event(void)
{
    // arrange
    LOG_CONTEXT_HANDLE log_context;

    // 38 bytes are in metadata by default with the setting below, 3 extra bytes for the struct that holds all properties, makes 41 bytes
    // 1 byte of formatted message
    // Need 4054 bytes to go over
    // 
    // Each line below adds 150 bytes, 27 * 150 = 4050, last line needs 4 bytes to get to exactly the limit, 5 bytes goes over the limit
    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________1, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________2, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________3, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________4, 4),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________5, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________6, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________7, 7),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________8, 8),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________9, 9),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________10, 10),
        // 1500
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________11, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________12, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________13, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________14, 4),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________15, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________16, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________17, 7),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________18, 8),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________19, 9),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________20, 10),
        // 3000
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________21, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________22, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________23, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________24, 4),
        // 3600
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________25, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________26, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________27, 7),
        // 4050
        LOG_CONTEXT_PROPERTY(uint8_t, pro, 8) // 5 bytes
        // 4056
    );

    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_vsnprintf_call(); // formatting message
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes[4096];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = TRACE_LEVEL_VERBOSE;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, "LogVerbose", strlen("LogVerbose") + 1);
    pos += strlen("LogVerbose") + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR expected_event_data_descriptors[6] =
    {
        { 0 },
        { 0 },
        {.Size = (ULONG)strlen("") + 1, .Ptr = (ULONGLONG)"" },
        {.Size = (ULONG)strlen(__FILE__) + 1, .Ptr = (ULONGLONG)__FILE__ },
        {.Size = (ULONG)strlen(__FUNCTION__) + 1, .Ptr = (ULONGLONG)__FUNCTION__ },
        {.Size = sizeof(int32_t), .Ptr = (ULONGLONG)&captured_line}
    };

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6, expected_event_data_descriptors);

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, log_context, __FILE__, __FUNCTION__, captured_line, "");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_085: [ If the size of the metadata and the formatted message exceeds 4096 bytes, log_sink_etw.log_sink_log shall not add any properties to the event. ]*/
static void when_size_of_metadata_of_exactly_4096_log_sink_etw_log_with_context_places_properties_in_the_event(void)
{
    // arrange
    LOG_CONTEXT_HANDLE log_context;

    // 38 bytes are in metadata by default with the setting below, 3 extra bytes for the struct that holds all properties, makes 41 bytes
    // 1 byte of formatted message
    // Need 4054 bytes to go over
    // 
    // Each line below adds 150 bytes, 27 * 150 = 4050, last line needs 4 bytes to get to exactly the limit, 5 bytes goes over the limit
    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________1, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________2, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________3, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________4, 4),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________5, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________6, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________7, 7),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________8, 8),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________9, 9),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________10, 10),
        // 1500
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________11, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________12, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________13, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________14, 4),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________15, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________16, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________17, 7),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________18, 8),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________19, 9),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________20, 10),
        // 3000
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________21, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________22, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________23, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________24, 4),
        // 3600
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________25, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________26, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________27, 7),
        // 4050
        LOG_CONTEXT_PROPERTY(uint8_t, pr, 8) // 4 bytes
        // 4056
    );

    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    uint8_t extra_metadata_bytes[4096];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = TRACE_LEVEL_VERBOSE;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, "LogVerbose", strlen("LogVerbose") + 1);
    pos += strlen("LogVerbose") + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* log_context_property_value_pairs = log_context_get_property_value_pairs(log_context);
    uint8_t property_count = (uint8_t)log_context_get_property_value_pair_count(log_context) - 1;

    // content field
    (void)memcpy(pos, "", strlen("") + 1);
    pos += strlen("") + 1;
    *pos = _TlgInSTRUCT | _TlgInChain;
    pos++;
    *pos = property_count;
    pos++;

    for (uint32_t i = 0; i < property_count; i++)
    {
        (void)memcpy(pos, log_context_property_value_pairs[i + 1].name, strlen(log_context_property_value_pairs[i + 1].name) + 1);
        pos += strlen(log_context_property_value_pairs[i + 1].name) + 1;
        switch (log_context_property_value_pairs[i + 1].type->get_type())
        {
        default:
            break;
        case LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr:
        {
            setup__tlgCreate1Sz_char();
            *pos = TlgInANSISTRING;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int64_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT64;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint64_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT64;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int32_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT32;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint32_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT32;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int16_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT16;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint16_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT16;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int8_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInINT8;
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint8_t:
        {
            setup_EventDataDescCreate();
            *pos = TlgInUINT8;
            break;
        }
        }
        pos++;
    }

    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR* expected_event_data_descriptors = malloc(sizeof(EVENT_DATA_DESCRIPTOR) * (6 + property_count));
    POOR_MANS_ASSERT(expected_event_data_descriptors != NULL);
    (void)memset(expected_event_data_descriptors, 0, sizeof(EVENT_DATA_DESCRIPTOR)* (6 + property_count));
    expected_event_data_descriptors[2].Size = (ULONG)strlen("") + 1;
    expected_event_data_descriptors[2].Ptr = (ULONGLONG)"";
    expected_event_data_descriptors[3].Size = (ULONG)strlen(__FILE__) + 1;
    expected_event_data_descriptors[3].Ptr = (ULONGLONG)__FILE__;
    expected_event_data_descriptors[4].Size = (ULONG)strlen(__FUNCTION__) + 1;
    expected_event_data_descriptors[4].Ptr = (ULONGLONG)__FUNCTION__;
    expected_event_data_descriptors[5].Size = sizeof(int32_t);
    expected_event_data_descriptors[5].Ptr = (ULONGLONG)&captured_line;

    for (uint32_t i = 0; i < property_count; i++)
    {
        ULONG expected_property_size;
        switch (log_context_property_value_pairs[i + 1].type->get_type())
        {
        default:
            expected_property_size = 0;
            break;

        case LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr:
        {
            expected_property_size = (ULONG)(strlen(log_context_property_value_pairs[i + 1].value) + 1);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int64_t:
        {
            expected_property_size = sizeof(int64_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint64_t:
        {
            expected_property_size = sizeof(uint64_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int32_t:
        {
            expected_property_size = sizeof(int32_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint32_t:
        {
            expected_property_size = sizeof(uint32_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int16_t:
        {
            expected_property_size = sizeof(int16_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint16_t:
        {
            expected_property_size = sizeof(uint16_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_int8_t:
        {
            expected_property_size = sizeof(int8_t);
            break;
        }
        case LOG_CONTEXT_PROPERTY_TYPE_uint8_t:
        {
            expected_property_size = sizeof(uint8_t);
            break;
        }
        }
        expected_event_data_descriptors[6 + i].Size = expected_property_size;
        expected_event_data_descriptors[6 + i].Ptr = (ULONGLONG)log_context_property_value_pairs[i + 1].value;
    }

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6 + property_count, expected_event_data_descriptors);

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, log_context, __FILE__, __FUNCTION__, captured_line, "");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    LOG_CONTEXT_DESTROY(log_context);

    free(expected_event_data_descriptors);
}

/* Tests_SRS_LOG_SINK_ETW_01_086: [ If any error occurs log_sink_etw.log_sink_log shall print Error emitting ETW event and return. ]*/
static void when_vsnprintf_fails_an_error_is_printed(void)
{
    // arrange
    LOG_CONTEXT_HANDLE log_context;

    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint8_t, prop1, 1)
    );

    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_vsnprintf_call(); // formatting message
    expected_calls[3].u.vsnprintf_call.override_result = true;
    expected_calls[3].u.vsnprintf_call.call_result = -1;
    setup_printf_call(); // spew error

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, log_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_086: [ If any error occurs log_sink_etw.log_sink_log shall print Error emitting ETW event and return. ]*/
static void when_size_of_metadata_and_formatted_messages_exceeds_4096_and_2nd_vsnprintf_fails_an_error_is_printed(void)
{
    // arrange
    LOG_CONTEXT_HANDLE log_context;

    // 38 bytes are in metadata by default with the setting below, 3 extra bytes for the struct that holds all properties, makes 41 bytes
    // 1 byte of formatted message
    // Need 4054 bytes to go over
    // 
    // Each line below adds 150 bytes, 27 * 150 = 4050, last line needs 4 bytes to get to exactly the limit, 5 bytes goes over the limit
    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________1, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________2, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________3, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________4, 4),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________5, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________6, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________7, 7),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________8, 8),
        LOG_CONTEXT_PROPERTY(uint8_t, property___________________________________________________________________________________________________________________________________________9, 9),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________10, 10),
        // 1500
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________11, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________12, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________13, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________14, 4),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________15, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________16, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________17, 7),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________18, 8),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________19, 9),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________20, 10),
        // 3000
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________21, 1),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________22, 2),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________23, 3),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________24, 4),
        // 3600
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________25, 5),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________26, 6),
        LOG_CONTEXT_PROPERTY(uint8_t, property__________________________________________________________________________________________________________________________________________27, 7),
        // 4050
        LOG_CONTEXT_PROPERTY(uint8_t, pro, 8) // 5 bytes
        // 4056
    );

    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_vsnprintf_call(); // formatting message
    setup_vsnprintf_call(); // formatting message 2nd attempt
    expected_calls[4].u.vsnprintf_call.override_result = true;
    expected_calls[4].u.vsnprintf_call.call_result = -1;
    setup_printf_call();

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, log_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    LOG_CONTEXT_DESTROY(log_context);
}

/* Tests_SRS_LOG_SINK_ETW_01_079: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint8_t, a byte with the value TlgInUINT8 shall be added in the metadata. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_080: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint8_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
static void when_a_parent_context_is_used_all_properties_are_emitted(void)
{
    // arrange
    LOG_CONTEXT_HANDLE log_context_1;
    LOG_CONTEXT_HANDLE log_context_2;

    LOG_CONTEXT_CREATE(log_context_1, NULL,
        LOG_CONTEXT_PROPERTY(uint8_t, prop1, 42)
    );

    LOG_CONTEXT_CREATE(log_context_2, log_context_1,
        LOG_CONTEXT_PROPERTY(uint8_t, prop2, 43)
    );

    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_vsnprintf_call(); // formatting message
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // line
    setup_EventDataDescCreate(); // prop1
    setup_EventDataDescCreate(); // prop2
    uint8_t extra_metadata_bytes[8192];
    uint8_t expected_event_bytes[sizeof(SELF_DESCRIBED_EVENT) + sizeof(extra_metadata_bytes)];
    SELF_DESCRIBED_EVENT* expected_event_metadata = (SELF_DESCRIBED_EVENT*)&expected_event_bytes[0];
    (void)memset(expected_event_metadata, 0, sizeof(SELF_DESCRIBED_EVENT));
    expected_event_metadata->_tlgLevel = TRACE_LEVEL_VERBOSE;
    expected_event_metadata->_tlgChannel = 11;
    expected_event_metadata->_tlgOpcode = 0;
    expected_event_metadata->_tlgKeyword = 0;
    uint8_t* pos = (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT));
    // event name
    (void)memcpy(pos, "LogVerbose", strlen("LogVerbose") + 1);
    pos += strlen("LogVerbose") + 1;
    // content field
    (void)memcpy(pos, "content", strlen("content") + 1);
    pos += strlen("content") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "file", strlen("file") + 1);
    pos += strlen("file") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "func", strlen("func") + 1);
    pos += strlen("func") + 1;
    *pos = TlgInANSISTRING;
    pos++;
    // content field
    (void)memcpy(pos, "line", strlen("line") + 1);
    pos += strlen("line") + 1;
    *pos = TlgInINT32;
    pos++;

    // content field for log_context_2
    (void)memcpy(pos, "", strlen("") + 1);
    pos += strlen("") + 1;
    *pos = _TlgInSTRUCT | _TlgInChain;
    pos++;
    *pos = 3;
    pos++;

    // content field for log_context_1
    (void)memcpy(pos, "", strlen("") + 1);
    pos += strlen("") + 1;
    *pos = _TlgInSTRUCT | _TlgInChain;
    pos++;
    *pos = 1;
    pos++;

    // prop1
    (void)memcpy(pos, "prop1", strlen("prop1") + 1);
    pos += strlen("prop1") + 1;
    *pos = TlgInUINT8;
    pos++;

    // prop2
    (void)memcpy(pos, "prop2", strlen("prop2") + 1);
    pos += strlen("prop2") + 1;
    *pos = TlgInUINT8;
    pos++;

    expected_event_metadata->_tlgEvtMetaSize = (uint16_t)(pos - (expected_event_bytes + sizeof(SELF_DESCRIBED_EVENT))) + 4;

    int captured_line = __LINE__;

    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* log_context_property_value_pairs = log_context_get_property_value_pairs(log_context_2);

    // construct event data descriptor array
    EVENT_DATA_DESCRIPTOR* expected_event_data_descriptors = malloc(sizeof(EVENT_DATA_DESCRIPTOR) * 8);
    POOR_MANS_ASSERT(expected_event_data_descriptors != NULL);
    (void)memset(expected_event_data_descriptors, 0, sizeof(EVENT_DATA_DESCRIPTOR) * 8);
    expected_event_data_descriptors[2].Size = (ULONG)strlen("test") + 1;
    expected_event_data_descriptors[2].Ptr = (ULONGLONG)"test";
    expected_event_data_descriptors[3].Size = (ULONG)strlen(__FILE__) + 1;
    expected_event_data_descriptors[3].Ptr = (ULONGLONG)__FILE__;
    expected_event_data_descriptors[4].Size = (ULONG)strlen(__FUNCTION__) + 1;
    expected_event_data_descriptors[4].Ptr = (ULONGLONG)__FUNCTION__;
    expected_event_data_descriptors[5].Size = sizeof(int32_t);
    expected_event_data_descriptors[5].Ptr = (ULONGLONG)&captured_line;
    expected_event_data_descriptors[6].Size = sizeof(uint8_t);
    expected_event_data_descriptors[6].Ptr = (ULONGLONG)log_context_property_value_pairs[2].value;
    expected_event_data_descriptors[7].Size = sizeof(uint8_t);
    expected_event_data_descriptors[7].Ptr = (ULONGLONG)log_context_property_value_pairs[3].value;

    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 8, expected_event_data_descriptors);

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, log_context_2, __FILE__, __FUNCTION__, captured_line, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    LOG_CONTEXT_DESTROY(log_context_1);
    LOG_CONTEXT_DESTROY(log_context_2);

    free(expected_event_data_descriptors);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    log_sink_etw_log_with_NULL_message_format_returns();
    log_sink_etw_log_registers_the_provider_if_not_registered_already();
    log_sink_log_retries_until_state_is_not_registering();
    log_sink_etw_log_does_not_register_when_already_registered();
    log_sink_etw_log_with_LOG_LEVEL_ERROR_succeeds();
    log_sink_etw_log_with_LOG_LEVEL_WARNING_succeeds();
    log_sink_etw_log_with_LOG_LEVEL_INFO_succeeds();
    log_sink_etw_log_with_LOG_LEVEL_VERBOSE_succeeds();
    log_sink_etw_log_with_unknown_LOG_LEVEL_succeeds();
    log_sink_etw_log_with_LOG_LEVEL_CRITICAL_format_message_succeeds();
    log_sink_etw_log_with_LOG_LEVEL_CRITICAL_format_message_succeeds_2();
    
    log_sink_etw_log_with_context_with_no_properties_succeeds();
    log_sink_etw_log_with_context_with_one_ascii_property_succeeds();
    log_sink_etw_log_with_context_with_one_int64_t_property_succeeds();
    log_sink_etw_log_with_context_with_one_uint64_t_property_succeeds();
    log_sink_etw_log_with_context_with_one_int32_t_property_succeeds();
    log_sink_etw_log_with_context_with_one_uint32_t_property_succeeds();
    log_sink_etw_log_with_context_with_one_int16_t_property_succeeds();
    log_sink_etw_log_with_context_with_one_uint16_t_property_succeeds();
    log_sink_etw_log_with_context_with_one_int8_t_property_succeeds();
    log_sink_etw_log_with_context_with_one_uint8_t_property_succeeds();
    log_sink_etw_log_with_context_with_all_property_types_succeeds();
    when_unknown_property_type_is_encountered_log_sink_etw_log_with_context_does_not_place_any_properties_in_the_event();
    when_more_than_64_properties_are_passed_in_context_log_sink_etw_log_with_context_does_not_place_any_properties_in_the_event();
    when_exactly_64_properties_are_passed_in_context_log_sink_etw_log_with_context_succeeds_and_emits_fields_for_each_property();
    when_size_of_metadata_exceeds_4096_log_sink_etw_log_with_context_does_not_place_any_properties_in_the_event();
    when_size_of_metadata_and_formatted_messages_exceeds_4096_log_sink_etw_log_with_context_does_not_place_any_properties_in_the_event();
    when_size_of_metadata_of_exactly_4096_log_sink_etw_log_with_context_places_properties_in_the_event();
    when_vsnprintf_fails_an_error_is_printed();
    when_size_of_metadata_and_formatted_messages_exceeds_4096_and_2nd_vsnprintf_fails_an_error_is_printed();
    when_a_parent_context_is_used_all_properties_are_emitted();

    return asserts_failed;
}

