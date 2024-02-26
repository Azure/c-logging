// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_value_pair.h"

#include "c_logging/log_context_property_to_string.h"

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

#define UPDATE_BUFFER_AND_SIZE(result, buffer, buffer_size, no_of_bytes) \
    no_of_bytes = MIN(no_of_bytes, (int)buffer_size); \
    buffer += no_of_bytes; \
    buffer_size -= no_of_bytes; \
    result += no_of_bytes;

static int log_n_properties(char* buffer, size_t buffer_size, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs, size_t property_value_pair_count, uint32_t depth, size_t* properties_added)
{
    int result = 0;

    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_016: [ For each property: ]*/
    // Note that for the depth==0 case, we want to iterate over all properties, knowing that we will skip over some for structs
    // In case we are in depth>0, then we are in a struct and only want to iterate over the property_value_pair_count
    // BUT, if we have nested structs, we need to skip over those properties at this depth, but not count them against property_value_pair_count
    size_t properties_added_this_level = 0;
    size_t i = 0;
    for (; (depth == 0 && i < property_value_pair_count) || (depth > 0 && properties_added_this_level < property_value_pair_count); i++)
    {
        properties_added_this_level++;
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_017: [ If the property type is struct (used as a container for context properties): ]*/
        if (property_value_pairs[i].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct)
        {
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_025: [ log_context_property_to_string shall print the struct property name and an opening brace. ]*/
            int snprintf_result = snprintf(buffer, buffer_size, " %s%s{", property_value_pairs[i].name, property_value_pairs[i].name[0] == 0 ? "" : "=");
            if (snprintf_result < 0)
            {
                /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_context_property_to_string shall fail and return a negative value. ]*/
                result = -1;
                break;
            }
            else
            {
                UPDATE_BUFFER_AND_SIZE(result, buffer, buffer_size, snprintf_result);

                /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_018: [ log_context_property_to_string shall obtain the number of fields in the struct. ]*/
                uint8_t struct_properties_count = *(uint8_t*)(property_value_pairs[i].value);

                /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_019: [ log_context_property_to_string shall print the next n properties as being the fields that are part of the struct. ]*/
                size_t increment_i = 0;
                int log_n_properties_result = log_n_properties(buffer, buffer_size, &property_value_pairs[i + 1], struct_properties_count, depth + 1, &increment_i);
                if (log_n_properties_result < 0)
                {
                    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_context_property_to_string shall fail and return a negative value. ]*/
                    result = -1;
                    break;
                }
                else
                {
                    UPDATE_BUFFER_AND_SIZE(result, buffer, buffer_size, log_n_properties_result);

                    i += increment_i;

                    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_026: [ log_context_property_to_string shall print a closing brace as end of the struct. ]*/
                    snprintf_result = snprintf(buffer, buffer_size, " }");
                    if (snprintf_result < 0)
                    {
                        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_context_property_to_string shall fail and return a negative value. ]*/
                        result = -1;
                        break;
                    }
                    else
                    {
                        snprintf_result = MIN(snprintf_result, (int)buffer_size);

                        buffer += snprintf_result;
                        buffer_size -= snprintf_result;
                        result += snprintf_result;
                    }
                }
            }
        }
        else
        {
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_020: [ Otherwise log_context_property_to_string shall call to_string for the property and print its name and value. ]*/
            int snprintf_result = snprintf(buffer, buffer_size, " %s=", property_value_pairs[i].name);
            if (snprintf_result < 0)
            {
                result = -1;
            }
            else
            {
                UPDATE_BUFFER_AND_SIZE(result, buffer, buffer_size, snprintf_result);

                int to_string_result = property_value_pairs[i].type->to_string(property_value_pairs[i].value, buffer, buffer_size);

                if (to_string_result < 0)
                {
                    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_context_property_to_string shall fail and return a negative value. ]*/
                    result = -1;
                    break;
                }
                else
                {
                    UPDATE_BUFFER_AND_SIZE(result, buffer, buffer_size, to_string_result);
                }
            }
        }
    }

    if (properties_added != NULL)
    {
        *properties_added += i;
    }

    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_021: [ log_context_property_to_string shall store at most buffer_size characters including the null terminator in buffer (the rest of the context shall be truncated). ]*/
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_031: [ log_context_property_to_string shall return the number of bytes written to the buffer. ]*/

    return result;
}

int log_context_property_to_string(char* buffer, size_t buffer_size, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs, size_t property_value_pair_count)
{
    int result;

    if (
        /*Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_027: [ If buffer is NULL then log_context_property_to_string shall fail and return a negative value. ]*/
        buffer == NULL ||
        /*Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_028: [ If buffer_size is 0 then log_context_property_to_string shall return 0. ]*/
        buffer_size == 0 ||
        /*Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_029: [ If property_value_pairs is NULL then log_context_property_to_string shall fail and return a negative value. ]*/
        property_value_pairs == NULL ||
        /*Codes_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_030: [ If property_value_pair_count is 0 then log_context_property_to_string shall fail and return a negative value. ]*/
        property_value_pair_count == 0
        )
    {
        result = -1;
    }
    else
    {
        result = log_n_properties(buffer, buffer_size, property_value_pairs, property_value_pair_count, 0, NULL);
    }

    return result;
}
