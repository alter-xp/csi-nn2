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

void op_test_run(struct csinn_tensor **input, struct csinn_tensor *output,
                 struct csinn_concat_params *params, struct csinn_session *sess,
                 struct csinn_tensor **real_input, float *output_data, float diff);

void test_f16(struct csinn_tensor **input, struct csinn_tensor *output,
              struct csinn_concat_params *params, float difference)
{
    printf("test concat f16\n");
    struct csinn_session *sess = csinn_alloc_session();
    sess->base_api = CSINN_C906;
    sess->base_run_mode = CSINN_RM_CPU_GRAPH;
    sess->base_dtype = CSINN_DTYPE_FLOAT16;
    sess->base_quant_type = CSINN_QUANT_FLOAT16;
    // sess->debug_level = CSINN_DEBUG_LEVEL_INFO;
    params->base.sess = sess;
    struct csinn_tensor *qinput[params->inputs_count];
    struct csinn_tensor *real_input[params->inputs_count];
    enum csinn_dtype_enum test_dtype = CSINN_DTYPE_FLOAT16;
    for (int i = 0; i < params->inputs_count; i++) {
        qinput[i] = convert_f32_input(input[i], test_dtype, sess);
        real_input[i] = convert_f32_input(input[i], test_dtype, sess);
    }

    struct csinn_tensor *qoutput = convert_f32_input(output, test_dtype, sess);

    op_test_run(qinput, qoutput, params, sess, real_input, output->data, difference);
}

void test_f32(struct csinn_tensor **input, struct csinn_tensor *output,
              struct csinn_concat_params *params, float difference)
{
    printf("test concat f32\n");
    struct csinn_session *sess = csinn_alloc_session();
    sess->base_api = CSINN_C906;
    sess->base_run_mode = CSINN_RM_CPU_GRAPH;
    sess->base_quant_type = CSINN_QUANT_FLOAT32;
    sess->base_dtype = CSINN_DTYPE_FLOAT32;
    // sess->debug_level = CSINN_DEBUG_LEVEL_INFO;
    params->base.sess = sess;
    struct csinn_tensor *qinput[params->inputs_count];
    struct csinn_tensor *real_input[params->inputs_count];
    enum csinn_dtype_enum test_dtype = CSINN_DTYPE_FLOAT32;
    for (int i = 0; i < params->inputs_count; i++) {
        qinput[i] = convert_f32_input(input[i], test_dtype, sess);
        real_input[i] = convert_f32_input(input[i], test_dtype, sess);
    }

    struct csinn_tensor *qoutput = convert_f32_input(output, test_dtype, sess);

    op_test_run(qinput, qoutput, params, sess, real_input, output->data, difference);
}

void test_concat(struct csinn_tensor **input, struct csinn_tensor *output,
                 struct csinn_concat_params *params, float difference)
{
    params->base.api = CSINN_C906;

    test_f16(input, output, params, difference);
    test_f32(input, output, params, difference);
}
