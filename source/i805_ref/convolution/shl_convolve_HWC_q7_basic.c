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

/* ----------------------------------------------------------------------
 * Title:        shl_convolve_HWC_q7_basic.c
 * Description:	 Q7 version of convolution
 *
 * -------------------------------------------------------------------- */

#include "i805_ref_function.h"

/**
 * @brief Basic Q7 convolution function
 * @param[in]       Im_in       pointer to input tensor
 * @param[in]       dim_im_in   input tensor dimention
 * @param[in]       ch_im_in    number of input tensor channels
 * @param[in]       wt          pointer to kernel weights
 * @param[in]       ch_im_out   number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel  filter kernel size
 * @param[in]       padding     padding sizes
 * @param[in]       stride      convolution stride
 * @param[in]       bias        pointer to bias
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in,out]   Im_out      pointer to output tensor
 * @param[in]       dim_im_out  output tensor dimension
 * @param[in,out]   bufferA     pointer to buffer space for input
 * @return     The function returns <code>CSI_MATH_SUCCESS</code>
 *
 * @details
 *
 * <b>Buffer size:</b>
 *
 * bufferA size: 2*ch_im_in*dim_kernel*dim_kernel
 *
 * This basic version is designed to work for any input tensor and weight
 * dimension.
 */

void shl_convolve_HWC_q7_basic(const q7_t* Im_in, const uint16_t dim_im_in, const uint16_t ch_im_in,
                               const q7_t* wt, const uint16_t ch_im_out, const uint16_t dim_kernel,
                               const uint16_t padding, const uint16_t stride, const q7_t* bias,
                               const uint16_t bias_shift, const uint16_t out_shift, q7_t* Im_out,
                               const uint16_t dim_im_out, q15_t* bufferA)
{
    uint16_t i, j, k, l, m, n;
    int conv_out;
    signed char in_row, in_col;

    for (i = 0; i < ch_im_out; i++) {
        for (j = 0; j < dim_im_out; j++) {
            for (k = 0; k < dim_im_out; k++) {
                conv_out = ((q31_t)bias[i] << bias_shift) + NN_ROUND(out_shift);
                for (m = 0; m < dim_kernel; m++) {
                    for (n = 0; n < dim_kernel; n++) {
                        // if-for implementation
                        in_row = stride * j + m - padding;
                        in_col = stride * k + n - padding;
                        if (in_row >= 0 && in_col >= 0 && in_row < dim_im_in &&
                            in_col < dim_im_in) {
                            for (l = 0; l < ch_im_in; l++) {
                                conv_out += Im_in[(in_row * dim_im_in + in_col) * ch_im_in + l] *
                                            wt[i * ch_im_in * dim_kernel * dim_kernel +
                                               (m * dim_kernel + n) * ch_im_in + l];
                            }
                        }
                    }
                }
                Im_out[i + (j * dim_im_out + k) * ch_im_out] =
                    (q7_t)__SSAT((conv_out >> out_shift), 8);
            }
        }
    }

    return;
}
