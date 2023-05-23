// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "windows.h"
#include "TraceLoggingProvider.h"
#include "evntrace.h"

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_level.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/log_context.h"
#include "c_logging/logger.h"

#include "c_logging/log_sink_etw.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_printf, \
    MOCK_CALL_TYPE_log_context_get_property_value_pair_count, \
    MOCK_CALL_TYPE_log_context_get_property_value_pairs, \
    MOCK_CALL_TYPE_TraceLoggingRegister_EventRegister_EventSetInformation, \
    MOCK_CALL_TYPE__get_pgmptr, \
    MOCK_CALL_TYPE__tlgCreate1Sz_char, \
    MOCK_CALL_TYPE_EventDataDescCreate, \
    MOCK_CALL_TYPE__tlgWriteTransfer_EventWriteTransfer, \
    MOCK_CALL_TYPE_log_context_property_if_get_type, \
    MOCK_CALL_TYPE_vsnprintf, \
    MOCK_CALL_TYPE_TraceLoggingUnregister

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

MU_DEFINE_ENUM_STRINGS(LOG_CONTEXT_PROPERTY_TYPE, LOG_CONTEXT_PROPERTY_TYPE_VALUES)

// very poor mans mocks :-(

#define MAX_PRINTF_CAPTURED_OUPUT_SIZE (LOG_MAX_MESSAGE_LENGTH * 2)

#define FILE_EVENT_DESCRIPTOR_ENTRY 3
#define FUNC_EVENT_DESCRIPTOR_ENTRY 4
#define LINE_EVENT_DESCRIPTOR_ENTRY 5

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
} TraceLoggingRegister_EventRegister_EventSetInformation_CALL;

typedef struct _get_pgmptr_CALL_TAG
{
    bool override_result;
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

typedef struct TraceLoggingUnregister_CALL_TAG
{
    TraceLoggingHProvider hProvider;
} TraceLoggingUnregister_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        printf_CALL printf_call;
        log_context_get_property_value_pair_count_CALL log_context_get_property_value_pair_count_call;
        log_context_get_property_value_pairs_CALL log_context_get_property_value_pairs_call;
        TraceLoggingRegister_EventRegister_EventSetInformation_CALL TraceLoggingRegister_EventRegister_EventSetInformation_call;
        _get_pgmptr_CALL _get_pgmptr_call;
        _tlgWriteTransfer_EventWriteTransfer_CALL _tlgWriteTransfer_EventWriteTransfer_call;
        log_context_property_if_get_type_CALL log_context_property_if_get_type_call;
        vsnprintf_CALL vsnprintf_call;
        TraceLoggingUnregister_CALL TraceLoggingUnregister_call;
    };
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
        if (expected_calls[actual_call_count].printf_call.override_result)
        {
            result = expected_calls[actual_call_count].printf_call.call_result;
        }
        else
        {
            va_list args;
            va_start(args, format);
            // also capture the result in a variable for comparisons in tests
            (void)vsnprintf(expected_calls[actual_call_count].printf_call.captured_output, sizeof(expected_calls[actual_call_count].printf_call.captured_output),
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
        if (expected_calls[actual_call_count].log_context_get_property_value_pair_count_call.override_result)
        {
            result = expected_calls[actual_call_count].log_context_get_property_value_pair_count_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].log_context_get_property_value_pair_count_call.captured_log_context = log_context;

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
        if (expected_calls[actual_call_count].log_context_get_property_value_pairs_call.override_result)
        {
            result = expected_calls[actual_call_count].log_context_get_property_value_pairs_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].log_context_get_property_value_pairs_call.captured_log_context = log_context;

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
        if (expected_calls[actual_call_count].TraceLoggingRegister_EventRegister_EventSetInformation_call.override_result)
        {
            result = expected_calls[actual_call_count].TraceLoggingRegister_EventRegister_EventSetInformation_call.call_result;
        }
        else
        {
            // save the provider globally
            test_provider = hProvider;

            // hardcode setup to allow all events
            ((struct _tlgProvider_t*)test_provider)->LevelPlus1 = TRACE_LEVEL_VERBOSE + 1;

            result = TraceLoggingRegister_EventRegister_EventSetInformation(hProvider);
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
        if (expected_calls[actual_call_count]._get_pgmptr_call.override_result)
        {
            result = expected_calls[actual_call_count]._get_pgmptr_call.call_result;
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
        if (expected_calls[actual_call_count]._tlgWriteTransfer_EventWriteTransfer_call.override_result)
        {
            result = expected_calls[actual_call_count]._tlgWriteTransfer_EventWriteTransfer_call.call_result;
        }
        else
        {
            SELF_DESCRIBED_EVENT* expected_self_described_event = expected_calls[actual_call_count]._tlgWriteTransfer_EventWriteTransfer_call.expected_self_described_event;
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
            else if (cData != expected_calls[actual_call_count]._tlgWriteTransfer_EventWriteTransfer_call.expected_cData)
            {
                (void)printf("Expected cData=%" PRIu32 ", actual=%" PRIu32 "\r\n",
                    expected_calls[actual_call_count]._tlgWriteTransfer_EventWriteTransfer_call.expected_cData, cData);
                actual_and_expected_match = false;
                result = E_FAIL;
            }
            else
            {
                // only compare starting at the index 2
                for (uint32_t i = 2; i < cData; i++)
                {
                    if (
                        (expected_calls[actual_call_count]._tlgWriteTransfer_EventWriteTransfer_call.ignore_file_and_func) &&
                        (
                            (i == FILE_EVENT_DESCRIPTOR_ENTRY) ||
                            (i == FUNC_EVENT_DESCRIPTOR_ENTRY) ||
                            (i == LINE_EVENT_DESCRIPTOR_ENTRY)
                        )
                        )
                    {
                        continue;
                    }

                    if (pData[i].Size != expected_calls[actual_call_count]._tlgWriteTransfer_EventWriteTransfer_call.expected_pData[i].Size)
                    {
                        (void)printf("Expected pData[%" PRIu32 "].Size=%" PRIu32 ", actual=%" PRIu32 "\r\n",
                            i, expected_calls[actual_call_count]._tlgWriteTransfer_EventWriteTransfer_call.expected_pData[i].Size, pData[i].Size);
                        break;
                    }
                    if (memcmp((void*)(uintptr_t)pData[i].Ptr, (void*)(uintptr_t)expected_calls[actual_call_count]._tlgWriteTransfer_EventWriteTransfer_call.expected_pData[i].Ptr, pData[i].Size) != 0)
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
        if (expected_calls[actual_call_count].log_context_property_if_get_type_call.override_result)
        {
            result = expected_calls[actual_call_count].log_context_property_if_get_type_call.call_result;
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
        if (expected_calls[actual_call_count].printf_call.override_result)
        {
            result = expected_calls[actual_call_count].printf_call.call_result;
        }
        else
        {
            // also capture the result in a variable for comparisons in tests
            (void)vsnprintf(expected_calls[actual_call_count].printf_call.captured_output, sizeof(expected_calls[actual_call_count].printf_call.captured_output),
                format, args);

            // call "real" function
            result = vsnprintf(s, n, format, args);
        }

        actual_call_count++;
    }

    return result;
}

void mock_TraceLoggingUnregister(TraceLoggingHProvider hProvider)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_TraceLoggingUnregister))
    {
        actual_and_expected_match = false;
    }
    else
    {
        expected_calls[actual_call_count].TraceLoggingUnregister_call.hProvider = hProvider;

        // call "real" function
        TraceLoggingUnregister(hProvider);

        actual_call_count++;
    }
}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

static void setup_printf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_printf;
    expected_calls[expected_call_count].printf_call.override_result = false;
    expected_call_count++;
}

static void setup_log_context_get_property_value_pair_count_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_context_get_property_value_pair_count;
    expected_calls[expected_call_count].log_context_get_property_value_pair_count_call.override_result = false;
    expected_call_count++;
}

static void setup_log_context_get_property_value_pairs_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_context_get_property_value_pairs;
    expected_calls[expected_call_count].log_context_get_property_value_pairs_call.override_result = false;
    expected_call_count++;
}

static void setup_TraceLoggingRegister_EventRegister_EventSetInformation_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_TraceLoggingRegister_EventRegister_EventSetInformation;
    expected_calls[expected_call_count].TraceLoggingRegister_EventRegister_EventSetInformation_call.override_result = false;
    expected_call_count++;
}

static void setup__get_pgmptr_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE__get_pgmptr;
    expected_calls[expected_call_count]._get_pgmptr_call.override_result = false;
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

static void setup__tlgWriteTransfer_EventWriteTransfer(void* event_metadata, uint32_t cData, const EVENT_DATA_DESCRIPTOR* pData, bool ignore_file_and_func)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE__tlgWriteTransfer_EventWriteTransfer;
    expected_calls[expected_call_count]._tlgWriteTransfer_EventWriteTransfer_call.override_result = false;
    expected_calls[expected_call_count]._tlgWriteTransfer_EventWriteTransfer_call.expected_self_described_event = event_metadata;
    expected_calls[expected_call_count]._tlgWriteTransfer_EventWriteTransfer_call.expected_cData = cData;
    expected_calls[expected_call_count]._tlgWriteTransfer_EventWriteTransfer_call.expected_pData = pData;
    expected_calls[expected_call_count]._tlgWriteTransfer_EventWriteTransfer_call.ignore_file_and_func = ignore_file_and_func;
    expected_call_count++;
}

static void setup_log_context_property_if_get_type(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_context_property_if_get_type;
    expected_calls[expected_call_count]._tlgWriteTransfer_EventWriteTransfer_call.override_result = false;
    expected_call_count++;
}

/* log_sink_etw.deinit */

/* Tests_SRS_LOG_SINK_ETW_01_093: [ If the module is not initialized, log_sink_etw.deinit shall return. ]*/
static void when_module_is_not_initialized_deinit_returns(void)
{
    // arrange
    setup_mocks();
    setup_printf_call(); // error output

    // act
    log_sink_etw.deinit();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    when_module_is_not_initialized_deinit_returns();

    return 0;
}
