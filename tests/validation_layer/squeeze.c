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
#include "math_snr.h"
#include "test_utils.h"

int main(int argc, char **argv)
{
    init_testsuite("Testing function of squeeze(layer).\n");

    struct csinn_session *sess = csinn_alloc_session();
    sess->base_run_mode = CSINN_RM_LAYER;
    struct csinn_tensor *input = csinn_alloc_tensor(sess);
    struct csinn_tensor *output = csinn_alloc_tensor(sess);
    struct csinn_tensor *reference = csinn_alloc_tensor(sess);
    struct csinn_squeeze_params *params =
        csinn_alloc_params(sizeof(struct csinn_squeeze_params), sess);
    int in_size;

    int *buffer = read_input_data_f32(argv[1]);
    int axis_len = buffer[3];
    int32_t *axis = (int32_t *)malloc(axis_len * sizeof(int32_t));
    for (int i = 0; i < axis_len; i++) {
        axis[i] = buffer[4 + i];
    }

    output->dim[0] = input->dim[0] = buffer[0];  // batch
    output->dim[1] = input->dim[1] = buffer[1];  // height
    output->dim[2] = input->dim[2] = buffer[2];  // width
    input->dim[3] = 1;
    input->dim[4] = 1;
    input->dim[5] = 1;
    input->dim_count = 6;
    output->dim_count = input->dim_count - axis_len;

    input->dtype = CSINN_DTYPE_FLOAT32;
    input->layout = CSINN_LAYOUT_NCHW;
    input->is_const = 0;
    input->quant_channel = 1;

    output->dtype = CSINN_DTYPE_FLOAT32;
    output->layout = CSINN_LAYOUT_NCHW;
    output->is_const = 0;
    output->quant_channel = 1;

    params->axis = axis;
    params->axis_num = axis_len;
    params->base.layout = CSINN_LAYOUT_NCHW;
    in_size = input->dim[0] * input->dim[1] * input->dim[2];
    params->base.api = CSINN_API;

    input->data = (float *)(buffer + 4 + axis_len);
    reference->data = (float *)(buffer + 4 + axis_len + in_size);
    output->data = reference->data;
    float difference = argc > 2 ? atof(argv[2]) : 0.99;

    test_squeeze_CSINN_QUANT_FLOAT32(input, output, params, &difference);
    test_squeeze_CSINN_QUANT_UINT8_ASYM(input, output, params, &difference);
    test_squeeze_CSINN_QUANT_INT8_SYM(input, output, params, &difference);

    return done_testing();
}
