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

#include "./valid_data/active_data.dat"
#include "csi_nn.h"
#include "math_snr.h"
#include "test_utils.h"

static void verify_sigmoid_q7(void *input_data, void *ref_data, int32_t size, float input_min,
                              float input_max, float difference)
{
    struct csinn_tensor *reference = csinn_alloc_tensor(NULL);
    int in_size, out_size;

    struct csinn_tensor *input = csinn_alloc_tensor(NULL);
    input->dim[0] = size;
    input->dim_count = 1;
    input->dtype = CSINN_DTYPE_INT8;
    input->name = "input";
    in_size = input->dim[0];
    input->qinfo->min = input_min;
    input->qinfo->max = input_max;

    struct csinn_tensor *output = csinn_alloc_tensor(NULL);
    output->dim[0] = input->dim[0];
    output->dim_count = 1;
    output->dtype = CSINN_DTYPE_INT8;
    output->name = "output";
    out_size = output->dim[0];

    struct csinn_sigmoid_params *params =
        csinn_alloc_params(sizeof(struct csinn_sigmoid_params), NULL);
    params->base.api = CSINN_API;
    params->base.name = "params";
    params->base.layout = CSINN_LAYOUT_NCHW;

    input->data = (uint8_t *)input_data;
    reference->data = (uint8_t *)ref_data;

    if (csinn_sigmoid_init(input, output, params) == CSINN_TRUE) {
        csinn_sigmoid(input, output, params);
    }

    result_verify_q7(reference->data, output->data, input->data, difference, out_size, false);
    free(input);
    free(output);
    free(reference);
}

int main(int argc, char **argv)
{
    init_testsuite("Testing function of sigmoid q7 for xt800.\n");

    verify_sigmoid_q7(q7_relu_input0, q7_sigmoid_result0, 1024, -0.7, 0.5, 0.0f);  // int_width=0
    verify_sigmoid_q7(q7_relu_input1, q7_sigmoid_result1, 1024, -0.7, 0.5, 0.0f);
    verify_sigmoid_q7(q7_relu_input2, q7_sigmoid_result2, 1024, -0.7, 0.5, 0.0f);
    verify_sigmoid_q7(q7_relu_input3, q7_sigmoid_result3, 1024, -0.7, 0.5, 0.0f);
    verify_sigmoid_q7(q7_relu_input4, q7_sigmoid_result4, 1024, -0.7, 0.5, 0.0f);

    verify_sigmoid_q7(q7_relu_input0, q7_sigmoid_result5, 1024, -1.5, 1.5, 0.0f);  // int_width=1
    verify_sigmoid_q7(q7_relu_input1, q7_sigmoid_result6, 1024, -1.5, 1.5, 0.0f);
    verify_sigmoid_q7(q7_relu_input2, q7_sigmoid_result7, 1024, -3.5, 3.5, 0.0f);  // int_width=2
    verify_sigmoid_q7(q7_relu_input3, q7_sigmoid_result8, 1024, -3.5, 3.5, 0.0f);
    verify_sigmoid_q7(q7_relu_input4, q7_sigmoid_result9, 1024, -7.5, 7.5, 0.0f);  // int_width=3

    verify_sigmoid_q7(q7_relu_input5, q7_sigmoid_result0, 1023, -0.7, 0.5, 0.0f);  // int_width=0
    verify_sigmoid_q7(q7_relu_input6, q7_sigmoid_result1, 1023, -0.7, 0.5, 0.0f);
    verify_sigmoid_q7(q7_relu_input7, q7_sigmoid_result2, 1023, -0.7, 0.5, 0.0f);
    verify_sigmoid_q7(q7_relu_input8, q7_sigmoid_result3, 1023, -0.7, 0.5, 0.0f);
    verify_sigmoid_q7(q7_relu_input9, q7_sigmoid_result4, 1023, -0.7, 0.5, 0.0f);

    verify_sigmoid_q7(q7_relu_input5, q7_sigmoid_result5, 1023, -1.5, 1.5, 0.0f);  // int_width=1
    verify_sigmoid_q7(q7_relu_input6, q7_sigmoid_result6, 1023, -1.5, 1.5, 0.0f);
    verify_sigmoid_q7(q7_relu_input7, q7_sigmoid_result7, 1023, -3.5, 3.5, 0.0f);  // int_width=2
    verify_sigmoid_q7(q7_relu_input8, q7_sigmoid_result8, 1023, -3.5, 3.5, 0.0f);
    verify_sigmoid_q7(q7_relu_input9, q7_sigmoid_result9, 1023, -7.5, 7.5, 0.0f);  // int_width=3
}
