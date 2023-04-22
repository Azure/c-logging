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
    MOCK_CALL_TYPE__tlgWriteTransfer_EventWriteTransfer

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
} _tlgWriteTransfer_EventWriteTransfer_CALL;

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
    } u;
} MOCK_CALL;

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
        if (expected_calls[actual_call_count].u._get_pgmptr_call.override_result)
        {
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
            else if (memcmp(expected_self_described_event->metadata, actual_self_described_event->metadata, actual_self_described_event->_tlgEvtMetaSize) != 0)
            {
                (void)printf("Event metadata does not match\r\n");
                actual_and_expected_match = false;
                result = E_FAIL;
            }
            else if (cData != expected_calls[actual_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_cData)
            {
                (void)printf("Expected cData=%" PRIu32 ", actual=%" PRIu32"\r\n",
                    expected_calls[actual_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_cData, cData);
                actual_and_expected_match = false;
                result = E_FAIL;
            }
            else
            {
                (void)hProvider;
                (void)pActivityId;
                (void)pRelatedActivityId;
                (void)cData;
                (void)pData;
                result = S_OK;
            }
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

static void setup_TraceLoggingRegister_EventRegister_EventSetInformation_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_TraceLoggingRegister_EventRegister_EventSetInformation;
    expected_calls[expected_call_count].u.TraceLoggingRegister_EventRegister_EventSetInformation_call.override_result = false;
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

static void setup__tlgWriteTransfer_EventWriteTransfer(void* event_metadata, uint32_t cData)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE__tlgWriteTransfer_EventWriteTransfer;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.override_result = false;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_self_described_event = event_metadata;
    expected_calls[expected_call_count].u._tlgWriteTransfer_EventWriteTransfer_call.expected_cData = cData;
    expected_call_count++;
}

/* log_sink_etw.log_sink_log */

/* Tests_SRS_LOG_SINK_ETW_01_001: [ If `message_format` is `NULL`, `log_sink_etw.log_sink_log` shall return. ]*/
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

/* Tests_SRS_LOG_SINK_ETW_01_002: [ `log_sink_etw_log` shall maintain the state of whether `TraceLoggingRegister` was called in a variable accessed via `InterlockedXXX` APIs, which shall have 3 possible values: `NOT_REGISTERED` (1), `REGISTERING` (2), `REGISTERED`(3). ]*/
/* Tests_SRS_LOG_SINK_ETW_01_003: [ `log_sink_etw_log` shall perform the below actions until the provider is registered or an error is encountered: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_004: [ If the state is `NOT_REGISTERED`: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_005: [ `log_sink_etw_log` shall switch the state to `REGISTERING`. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_006: [ `log_sink_etw_log` shall register the ETW TraceLogging provider by calling `TraceLoggingRegister` (`TraceLoggingRegister_EventRegister_EventSetInformation`). ]*/
/* Tests_SRS_LOG_SINK_ETW_01_007: [ `log_sink_etw_log` shall switch the state to `REGISTERED`. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_008: [ `log_sink_etw_log` shall emit a `LOG_LEVEL_INFO` event as a self test , printing the fact that the provider was registered and from which executable (as obtained by calling `_get_pgmptr`). ]*/
/* Tests_SRS_LOG_SINK_ETW_01_009: [ Checking and changing the variable that maintains whether `TraceLoggingRegister` was called shall be done using `InterlockedCompareExchange` and `InterlockedExchange`. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_010: [ `log_sink_etw_log` shall emit a self described event that shall have the name of the event as follows: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_012: [ If `log_level` is `LOG_LEVEL_CRITICAL` the event name shall be `LogCritical`. ]*/
static void log_sink_etw_log_registers_the_provider_if_not_registered_already(void)
{
    // arrange
    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup_TraceLoggingRegister_EventRegister_EventSetInformation_call();
    setup_InterlockedExchange_call();

    // self test event
    setup__get_pgmptr_call();

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, __LINE__, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_SINK_ETW_01_002: [ `log_sink_etw_log` shall maintain the state of whether `TraceLoggingRegister` was called in a variable accessed via `InterlockedXXX` APIs, which shall have 3 possible values: `NOT_REGISTERED` (1), `REGISTERING` (2), `REGISTERED`(3). ]*/
/* Tests_SRS_LOG_SINK_ETW_01_003: [ `log_sink_etw_log` shall perform the below actions until the provider is registered or an error is encountered: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_011: [ If the state is `REGISTERED`, `log_sink_etw_log` shall proceed to log the ETW event. ]*/
// Note the reason this is already registered is because previous tests have done the registration
/* Tests_SRS_LOG_SINK_ETW_01_010: [ `log_sink_etw_log` shall emit a self described event that shall have the name of the event as follows: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_025: [ If log_context is NULL only the fields content, file, func and line shall be added to the ETW event. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_042: [ log_sink_etw.log_sink_log shall compute the metadata size for the self described event metadata as follows: ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_043: [ Length of the event name (determined at compile time, excluding zero terminator) + 1. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_044: [ Length of the content field name (determined at compile time, excluding zero terminator) + 1. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_046: [ Length of the func field name (determined at compile time, excluding zero terminator) + 1. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_047: [ Length of the line field name (determined at compile time, excluding zero terminator) + 1. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_026: [ log_sink_etw.log_sink_log shall fill a SELF_DESCRIBED_EVENT structure, setting the following fields: ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_027: [ _tlgBlobTyp shall be set to _TlgBlobEvent4. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_028: [ _tlgChannel shall be set to 11. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_029: [ _tlgLevel shall be set to the appropriate logging level. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_030: [ _tlgOpcode shall be set to 0. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_031: [ _tlgKeyword shall be set to 0. ]*/
    /* Tests_SRS_LOG_SINK_ETW_01_032: [ _tlgEvtMetaSize shall be set to the computed metadata size + 4. ]*/
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
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // func
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
    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6);

    // act
    log_sink_etw.log_sink_log(log_level, NULL, __FILE__, __FUNCTION__, __LINE__, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_SINK_ETW_01_012: [ If `log_level` is `LOG_LEVEL_CRITICAL` the event name shall be `LogCritical`. ]*/
/* Tests_SRS_LOG_SINK_ETW_01_018: [ Logging level: ]*/
/* Tests_SRS_LOG_SINK_ETW_01_019: [ If log_level is LOG_LEVEL_CRITICAL the ETW logging level shall be TRACE_LEVEL_CRITICAL. ]*/
static void log_sink_etw_log_does_not_register_when_already_registered(void)
{
    test_message_with_level(LOG_LEVEL_CRITICAL, TRACE_LEVEL_CRITICAL, "LogCritical");
}

/* Tests_SRS_LOG_SINK_ETW_01_013: [ If `log_level` is `LOG_LEVEL_ERROR` the event name shall be `LogError`. ]*/
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
static void test_formatted_message_with_level(LOG_LEVEL log_level, uint8_t expected_tlg_level, const char* expected_event_name, const char* expected_message, const char* message_format, ...)
{
    (void)expected_message;
    (void)message_format;

    // arrange
    setup_enabled_provider(TRACE_LEVEL_VERBOSE);

    setup_mocks();
    setup_InterlockedCompareExchange_call();
    setup__tlgCreate1Sz_char(); // message
    setup__tlgCreate1Sz_char(); // file 
    setup__tlgCreate1Sz_char(); // func
    setup_EventDataDescCreate(); // func
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
    setup__tlgWriteTransfer_EventWriteTransfer(expected_event_metadata, 6);

    // act
    log_sink_etw.log_sink_log(log_level, NULL, __FILE__, __FUNCTION__, __LINE__, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_SINK_ETW_01_040: [ log_sink_etw.log_sink_log shall set event data descriptor at index 2 by calling _tlgCreate1Sz_char with the value of the formatted message as obtained by using printf with the messages format message_format and the arguments in .... ]*/
static void log_sink_etw_log_with_LOG_LEVEL_CRITICAL_format_message_succeeds(void)
{
    test_formatted_message_with_level(LOG_LEVEL_CRITICAL, TRACE_LEVEL_CRITICAL, "LogCritical", "test_value=42", "test_value=%d", 42);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    log_sink_etw_log_with_NULL_message_format_returns();
    log_sink_etw_log_registers_the_provider_if_not_registered_already();
    log_sink_etw_log_does_not_register_when_already_registered();
    log_sink_etw_log_with_LOG_LEVEL_ERROR_succeeds();
    log_sink_etw_log_with_LOG_LEVEL_WARNING_succeeds();
    log_sink_etw_log_with_LOG_LEVEL_INFO_succeeds();
    log_sink_etw_log_with_LOG_LEVEL_VERBOSE_succeeds();
    log_sink_etw_log_with_unknown_LOG_LEVEL_succeeds();
    log_sink_etw_log_with_LOG_LEVEL_CRITICAL_format_message_succeeds();

    return asserts_failed;
}

