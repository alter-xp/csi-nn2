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

#include "e804_function.h"
#include "shl_e804.h"

int shl_e804_relu_q7(struct csinn_tensor *input, struct csinn_tensor *output,
                     struct csinn_relu_params *params)
{
    q7_t *input_data = (q7_t *)input->data;
    int size = csinn_tensor_size(input);
    csky_dsp2_relu_q7(input_data, size);
    output->data = input->data;
    return CSINN_TRUE;
}

int shl_e804_relu_q15(struct csinn_tensor *input, struct csinn_tensor *output,
                      struct csinn_relu_params *params)
{
    q15_t *input_data = (q15_t *)input->data;
    int size = csinn_tensor_size(input);
    csky_dsp2_relu_q15(input_data, size);
    output->data = input->data;
    return CSINN_TRUE;
}
