/*
 * Copyright (C) 2016-2022 T-Head Semiconductor Co., Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* CSI-NN2 version 2.0.x */

#include "csi_nn.h"
#include "shl_utils.h"

int csinn_fsmn_init(struct csinn_tensor *frame, struct csinn_tensor *l_filter,
                    struct csinn_tensor *r_filter, struct csinn_tensor *frame_sequence,
                    struct csinn_tensor *frame_counter, struct csinn_tensor *output,
                    struct csinn_fsmn_params *params)
{
    shl_op_callback_map(&params->base, CSINN_OP_FSMN, frame->dtype);
    struct csinn_callback *cb = params->base.cb;
    int (*func)() = shl_get_init_cb(&params->base);
    if (func != NULL) {
        func(frame, l_filter, r_filter, frame_sequence, frame_counter, output, params);
    }
    return CSINN_TRUE;
}

int csinn_fsmn(struct csinn_tensor *frame, struct csinn_tensor *l_filter,
               struct csinn_tensor *r_filter, struct csinn_tensor *frame_sequence,
               struct csinn_tensor *frame_counter, struct csinn_tensor *output,
               struct csinn_fsmn_params *params)
{
    SHL_DEBUG_CALL(shl_fsmn_debug_info(frame, l_filter, r_filter, frame_sequence, frame_counter,
                                       output, params, __func__));
    int (*func)() = shl_get_p0_cb(&params->base);
    if (func != NULL) {
        func(frame, l_filter, r_filter, frame_sequence, frame_counter, output, params);
    } else {
        return CSINN_CALLBACK_UNSET;
    }
    return CSINN_TRUE;
}