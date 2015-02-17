/*
 * Copyright (c) 2010, 2012 Aalto University and RWTH Aachen University.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * @brief Transform related functions for HIP
 */

#include <arpa/inet.h>

#include "debug.h"
#include "builder.h"
#include "transform.h"

/**
 * select a HIP transform
 *
 * @param ht HIP_TRANSFORM payload where the transform is selected from
 * @return   the first acceptable Transform-ID or zero if no acceptable
 *           transform was found. The return value is in host byte order.
 */
hip_transform_suite hip_select_hip_transform(const struct hip_tlv_common *ht)
{
    int                        item_number = HIP_TRANSFORM_HIP_MAX;
    hip_transform_suite        tid         = 0;
    const hip_transform_suite *suggestion;
    hip_transform_suite        suite[item_number];

    item_number = hip_get_list_from_param(ht, suite, item_number,
                                          sizeof(hip_transform_suite));

    suggestion = suite;
    while (item_number-- > 0) {
        switch (*suggestion) {
        case HIP_HIP_AES_SHA1:
        case HIP_HIP_3DES_SHA1:
        case HIP_HIP_NULL_SHA1:
            tid = *suggestion;
            goto out;

        default:
            /* Specs don't say what to do when unknown are found.
             * We ignore.
             */
            HIP_ERROR("Unknown HIP suite id suggestion (%u)\n",
                      ntohs(*suggestion));
            break;
        }
        suggestion++;
    }

out:
    if (tid == 0) {
        HIP_ERROR("None HIP transforms accepted\n");
    } else {
        HIP_DEBUG("Chose HIP transform: %d\n", tid);
    }

    return tid;
}

/**
 * select an ESP transform to use
 * @param param ESP_TRANSFORM payload where the transform is selected from
 *
 * @return      the first acceptable Suite-ID or zero if no acceptable
 *              Suite-ID was found.
 */
hip_transform_suite hip_select_esp_transform(const struct hip_tlv_common *param)
{
    int                        item_number = HIP_TRANSFORM_ESP_MAX;
    hip_transform_suite        tid         = 0;
    hip_transform_suite        suite[item_number];
    const hip_transform_suite *suggestion;

    item_number = hip_get_list_from_param(param, suite, item_number,
                                          sizeof(hip_transform_suite));

    /* the first item in the list is a reserved field and should be ignored */
    if (--item_number < 1) {
        HIP_ERROR("No ESP suite id can be found\n");
        return 0;
    }
    suggestion = &suite[1];

    while (item_number-- > 0) {
        switch (*suggestion) {
        case HIP_ESP_AES_SHA1:
        case HIP_ESP_NULL_NULL:
        case HIP_ESP_3DES_SHA1:
        case HIP_ESP_NULL_SHA1:
            tid = *suggestion;
            goto out;
        default:
            /* Specs don't say what to do when unknowns are found.
             * We ignore.
             */
            HIP_ERROR("Unknown ESP suite id suggestion (%u)\n", *suggestion);
            break;
        }
        suggestion++;
    }

out:
    HIP_DEBUG("Took ESP transform %d\n", tid);

    if (tid == 0) {
        HIP_ERROR("Faulty ESP transform\n");
    }

    return tid;
}

/**
 * get transform key length for a transform
 * @param tid transform
 *
 * @return the transform key length based for the chosen transform,
 * or negative on error.
 */
int hip_transform_key_length(int tid)
{
    int ret = -1;

    switch (tid) {
    case HIP_HIP_AES_SHA1:
        ret = 16;
        break;
    case HIP_HIP_3DES_SHA1:
        ret = 24;
        break;
    case HIP_HIP_NULL_SHA1:     // XX FIXME: SHOULD BE NULL_SHA1?
        ret = 0;
        break;
    default:
        HIP_ERROR("unknown tid=%d\n", tid);
        HIP_ASSERT(0);
        break;
    }

    return ret;
}
