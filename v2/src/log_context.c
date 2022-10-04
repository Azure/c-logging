// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_context_property_type_if.h"

uint32_t log_context_get_property_value_pair_count(LOG_CONTEXT_HANDLE log_context)
{
    uint32_t result;

    if (log_context == NULL)
    {
        /* Codes_SRS_LOG_CONTEXT_01_020: [ If `log_context` is `NULL`, `log_context_get_property_value_pair_count` shall return `UINT32_MAX`. ]*/
        result = UINT32_MAX;
    }
    else
    {
        /* Codes_SRS_LOG_CONTEXT_01_021: [ Otherwise, `log_context_get_property_value_pair_count` shall return the number of property/value pairs stored by `log_context`. ]*/
        result = log_context->property_value_pair_count;
    }

    return result;
}

const LOG_CONTEXT_PROPERTY_VALUE_PAIR* log_context_get_property_value_pairs(LOG_CONTEXT_HANDLE log_context)
{
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* result;

    if (log_context == NULL)
    {
        /* Codes_SRS_LOG_CONTEXT_01_022: [ If `log_context` is `NULL`, `log_context_get_property_value_pairs` shall fail and return `NULL`. ]*/
        result = NULL;
    }
    else
    {
        /* Codes_SRS_LOG_CONTEXT_01_023: [ Otherwise, `log_context_get_property_value_pairs` shall return the array of property/value pairs stored by the context. ]*/
        result = log_context->property_value_pairs_ptr;
    }

    return result;
}

uint32_t internal_log_context_get_values_data_length_or_zero(LOG_CONTEXT_HANDLE log_context)
{
    return (log_context == NULL) ? 0 : log_context->values_data_length;
}

int internal_log_context_init_from_parent(LOG_CONTEXT_HANDLE dest_log_context, LOG_CONTEXT_HANDLE parent_log_context)
{
    int result;

    /* Copy all the pairs from the parent */
    if (parent_log_context != NULL)
    {
        size_t prop_copy_index;
        for (prop_copy_index = 0; prop_copy_index < parent_log_context->property_value_pair_count; prop_copy_index++)
        {
            dest_log_context->property_value_pairs_ptr[prop_copy_index + 1].name = parent_log_context->property_value_pairs_ptr[prop_copy_index].name;
            dest_log_context->property_value_pairs_ptr[prop_copy_index + 1].type = parent_log_context->property_value_pairs_ptr[prop_copy_index].type;

            dest_log_context->property_value_pairs_ptr[prop_copy_index + 1].value = (void*)(dest_log_context->values_data + 1 + ((uint8_t*)parent_log_context->property_value_pairs_ptr[prop_copy_index].value - parent_log_context->values_data));
            if (dest_log_context->property_value_pairs_ptr[prop_copy_index + 1].type->copy(dest_log_context->property_value_pairs_ptr[prop_copy_index + 1].value, parent_log_context->property_value_pairs_ptr[prop_copy_index].value) != 0)
            {
                (void)printf("Error copying property value/pair %zu\r\n", prop_copy_index);
                break;
            }
        }

        if (prop_copy_index < parent_log_context->property_value_pair_count)
        {
            result = MU_FAILURE;
        }
        else
        {
            result = 0;
        }
    }
    else
    {
        result = 0;
    }

    return result;
}

LOG_CONTEXT_HANDLE log_context_create(LOG_CONTEXT_HANDLE parent_context, uint32_t properties_count, uint32_t data_size)
{
    /* Codes_SRS_LOG_CONTEXT_01_001: [ LOG_CONTEXT_CREATE shall allocate memory for the log context. ]*/
    LOG_CONTEXT_HANDLE result =  malloc(sizeof(LOG_CONTEXT) + (sizeof(LOG_CONTEXT_PROPERTY_VALUE_PAIR) * properties_count) + data_size);
    if (result == NULL)
    {
        /* Codes_SRS_LOG_CONTEXT_01_002: [ If any error occurs, LOG_CONTEXT_CREATE shall fail and return NULL. ]*/
        (void)printf("malloc(sizeof(LOG_CONTEXT)) failed\r\n");
    }
    else
    {
        result->property_value_pairs_ptr = (void*)((uint8_t*)result + sizeof(LOG_CONTEXT));
        result->property_value_pair_count = properties_count;
        result->values_data = (void*)(result->property_value_pairs_ptr + properties_count);
        result->values_data_length = data_size;

        /* Codes_SRS_LOG_CONTEXT_01_014: [ If parent_context is non-NULL, the created context shall include all the property/value pairs of parent_context. ]*/
        internal_log_context_init_from_parent(result, parent_context);
        /* return as is */
    }

    return result;
}

void log_context_destroy(LOG_CONTEXT_HANDLE log_context)
{
    /* Codes_SRS_LOG_CONTEXT_01_006: [ LOG_CONTEXT_DESTROY shall free the memory and resources associated with log_context that were allocated by LOG_CONTEXT_CREATE. ]*/
    free(log_context);
}
