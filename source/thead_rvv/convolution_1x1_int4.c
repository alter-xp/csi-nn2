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

#include "shl_thead_rvv.h"
#ifdef XTHEADV
// kernel_layout: [o, h, w, i]
void shl_rvv_conv1x1s1_gemm_reorder_kernel_int4(struct csinn_tensor *kernel,
                                                struct csinn_conv2d_params *params)
{
    int8_t *kernel_data = (int8_t *)kernel->data;
    int group = params->group;

    int n = kernel->dim[0] / group;       // out_ch
    int k = kernel->dim[3];               // in_ch ( kernel->dim[1] = kernel->dim[2] = 1)
    int k_2 = (((k - 1) & -2) + 2) >> 1;  // pair of int4, col of kernel_matrix
    int k4 = ((k_2 - 1) & -4) + 4;        // align of 4 for int8

    params->conv_extra.kernel_tm->data = (int8_t *)shl_mem_alloc(group * n * k4 * sizeof(int8_t));
    int8_t *pa_reorder = (int8_t *)params->conv_extra.kernel_tm->data;

    for (int g = 0; g < group; g++) {
        shl_rvv_reorder_kernel_n8_int8(kernel_data + g * n * k_2, pa_reorder + g * n * k4, n, k_2,
                                       k_2);
    }
    // FIXME: free params->conv_extra.kernel_tm->data
    // memcpy(kernel_data, pa_reorder, group * m * k * sizeof(int8_t));
    // shl_mem_free(pa_reorder);
}

int shl_rvv_conv1x1s1_gemm_int4(struct csinn_tensor *input, struct csinn_tensor *output,
                                struct csinn_tensor *kernel, struct csinn_tensor *bias,
                                struct csinn_conv2d_params *params)
{
    int8_t *input_data = (int8_t *)input->data;
    int8_t *output_data = (int8_t *)output->data;
    int8_t *kernel_data = (int8_t *)params->conv_extra.kernel_tm->data;
    int32_t *bias_data = (int32_t *)bias->data;

    int32_t group = params->group;
    int32_t batch = input->dim[0];  // assert(batch == 1);
    int32_t in_ch = input->dim[3];
    int32_t out_ch = kernel->dim[0];
    int32_t out_h = output->dim[1];
    int32_t out_w = output->dim[2];

    int32_t m = out_h * out_w;
    int32_t k = in_ch / group;
    int32_t n = out_ch / group;

    int32_t k_2 = (((k - 1) & -2) + 2) >> 1;
    int32_t k4 = ((k_2 - 1) & -4) + 4;

    int8_t *pa_reorder = (int8_t *)shl_mem_alloc(m * k4 * sizeof(int8_t));
    int32_t *multiplier = (int32_t *)shl_mem_alloc(n * sizeof(int32_t));
    int32_t *shift = (int32_t *)shl_mem_alloc(n * sizeof(int32_t));

    int j = 0;
    for (int i = 0; i < batch; i++) {
        for (int g = 0; g < group; g++) {
            int8_t *pa = pa_reorder;
            int8_t *pb = kernel_data + g * n * k4;
            int8_t *pc = output_data;

            if (kernel->quant_channel > 1) {
                for (int c = 0; c < n; c++, j++) {
                    multiplier[c] = kernel->qinfo[j].multiplier;
                    shift[c] = kernel->qinfo[j].shift;
                }
            } else if (kernel->quant_channel == 1) {
                for (int c = 0; c < n; c++) {
                    multiplier[c] = kernel->qinfo[0].multiplier;
                    shift[c] = kernel->qinfo[0].shift;
                }
            }

            // pack
            shl_rvv_reorder_input_n8_int4(input_data, pa, m, k_2, k_2);
            // GEMM
            shl_rvv_gemm_8x8_int4(pc, pa, pb, m, k4, n, n / 2, bias_data + g * n,
                                  output->qinfo->zero_point, multiplier, shift);
            input_data += m * k_2;
            output_data += m * n / 2;
        }
    }
    shl_mem_free(pa_reorder);
    shl_mem_free(multiplier);
    shl_mem_free(shift);
    return CSINN_TRUE;
}
#endif
