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

int shl_rvv_depthwise_conv2d_init_fp32(struct csinn_tensor *input, struct csinn_tensor *output,
                                       struct csinn_tensor *kernel, struct csinn_tensor *bias,
                                       struct csinn_conv2d_params *params)
{
    int32_t batch = input->dim[0];
    int32_t in_c = input->dim[1];
    int32_t out_c = output->dim[1];
    int32_t kernel_h = kernel->dim[2];
    int32_t kernel_w = kernel->dim[3];
    int32_t stride_h = params->stride_height;
    int32_t stride_w = params->stride_width;
    struct csinn_callback *cb = params->base.cb;

    const int packn = csrr_vlenb() / sizeof(float);

    if (in_c % packn == 0 && out_c % packn == 0) {
        if (kernel_h == 3 && kernel_w == 3 && stride_h == 1 && stride_w == 1) {
            shl_rvv_dwconv_reorder_kernel_packn_fp32(kernel, params);
            cb->exec = shl_rvv_dwconv3x3s1_packn_fp32;

        } else if (kernel_h == 3 && kernel_w == 3 && stride_h == 2 && stride_w == 2) {
            shl_rvv_dwconv_reorder_kernel_packn_fp32(kernel, params);
            cb->exec = shl_rvv_dwconv3x3s2_packn_fp32;
        } else {
            cb->exec = shl_ref_depthwise_conv2d_f32;
        }
    }

    if (in_c % packn != 0 && out_c % packn != 0) {
        if (kernel_h == 3 && kernel_w == 3 && stride_h == 1 && stride_w == 1) {
            cb->exec = shl_rvv_dwconv3x3s1_fp32;
        } else if (kernel_h == 3 && kernel_w == 3 && stride_h == 2 && stride_w == 2) {
            cb->exec = shl_rvv_dwconv3x3s2_fp32;
        } else {
            cb->exec = shl_ref_depthwise_conv2d_f32;
        }
    }
    return CSINN_TRUE;
}

int shl_rvv_depthwise_conv2d_init_fp16(struct csinn_tensor *input, struct csinn_tensor *output,
                                       struct csinn_tensor *kernel, struct csinn_tensor *bias,
                                       struct csinn_conv2d_params *params)
{
    int32_t batch = input->dim[0];
    int32_t in_c = input->dim[1];
    int32_t out_c = output->dim[1];
    int32_t kernel_h = kernel->dim[2];
    int32_t kernel_w = kernel->dim[3];
    int32_t stride_h = params->stride_height;
    int32_t stride_w = params->stride_width;
    struct csinn_callback *cb = params->base.cb;

    const int packn = csrr_vlenb() / sizeof(__fp16);

    if (in_c % packn == 0 && out_c % packn == 0) {
        if (kernel_h == 3 && kernel_w == 3 && stride_h == 1 && stride_w == 1) {
            shl_rvv_dwconv_reorder_kernel_packn_fp16(kernel, params);
            cb->exec = shl_rvv_dwconv3x3s1_packn_fp16;

        } else if (kernel_h == 3 && kernel_w == 3 && stride_h == 2 && stride_w == 2) {
            shl_rvv_dwconv_reorder_kernel_packn_fp16(kernel, params);
            cb->exec = shl_rvv_dwconv3x3s2_packn_fp16;
        } else {
            cb->exec = shl_ref_depthwise_conv2d_quant;
        }
    }

    if (in_c % packn != 0 && out_c % packn != 0) {
        if (kernel_h == 3 && kernel_w == 3 && stride_h == 1 && stride_w == 1) {
            cb->exec = shl_rvv_dwconv3x3s1_fp16;
        } else if (kernel_h == 3 && kernel_w == 3 && stride_h == 2 && stride_w == 2) {
            cb->exec = shl_rvv_dwconv3x3s2_fp16;
        } else {
            cb->exec = shl_ref_depthwise_conv2d_quant;
        }
    }
    return CSINN_TRUE;
}

int shl_rvv_depthwise_conv2d_init_int8(struct csinn_tensor *input, struct csinn_tensor *output,
                                       struct csinn_tensor *kernel, struct csinn_tensor *bias,
                                       struct csinn_conv2d_params *params)
{
    int32_t batch = input->dim[0];
    int32_t in_c = input->dim[1];
    int32_t out_c = output->dim[1];
    int32_t kernel_h = kernel->dim[2];
    int32_t kernel_w = kernel->dim[3];
    int32_t stride_h = params->stride_height;
    int32_t stride_w = params->stride_width;
    struct csinn_callback *cb = params->base.cb;

    const int packn = csrr_vlenb() / sizeof(int8_t) / 2;

    // enable fuse zeropoint to bias
    if (!params->conv_extra.fuse_zp2bias) {
        int32_t *bias_data = (int32_t *)bias->data;
        int8_t *kernel_data = (int8_t *)kernel->data;
        int32_t input_zp = input->qinfo->zero_point;

        if (bias_data == NULL) {
            // XXX: memory leak
            bias_data = (int32_t *)shl_mem_alloc(out_c * sizeof(int32_t));
            bias->data = bias_data;
        }
        int kernel_inner = 1 * kernel_h * kernel_w;
        for (int oc = 0; oc < out_c; oc++) {
            int32_t tmp = 0;
            for (int j = 0; j < kernel_inner; j++) {
                tmp += kernel_data[oc * kernel_inner + j] * input_zp;
            }
            bias_data[oc] -= tmp;
        }
    }

    if (in_c % packn == 0 && out_c % packn == 0) {
        if (kernel_h == 3 && kernel_w == 3 && stride_h == 1 && stride_w == 1) {
            shl_rvv_dwconv_reorder_kernel_packn_int8(kernel, params);
            cb->exec = shl_rvv_dwconv3x3s1_packn_int8;
        } else if (kernel_h == 3 && kernel_w == 3 && stride_h == 2 && stride_w == 2) {
            shl_rvv_dwconv_reorder_kernel_packn_int8(kernel, params);
            cb->exec = shl_rvv_dwconv3x3s2_packn_int8;
        } else {
            cb->exec = shl_ref_depthwise_conv2d_quant;
        }
    }

    if (in_c % packn != 0 && out_c % packn != 0) {
        if (kernel_h == 3 && kernel_w == 3 && stride_h == 1 && stride_w == 1) {
            cb->exec = shl_rvv_dwconv3x3s1_int8;
        } else if (kernel_h == 3 && kernel_w == 3 && stride_h == 2 && stride_w == 2) {
            cb->exec = shl_rvv_dwconv3x3s2_int8;
        } else {
            cb->exec = shl_ref_depthwise_conv2d_quant;
        }
    }
    // support channel quantization
    for (int i = 0; i < kernel->quant_channel; i++) {
        float real_scale = input->qinfo->scale * kernel->qinfo[i].scale / output->qinfo->scale;
        shl_quantize_multiplier(real_scale, &(kernel->qinfo[i].multiplier),
                                &(kernel->qinfo[i].shift));
    }
    return CSINN_TRUE;
}

int shl_rvv_depthwise_conv2d_init_int4(struct csinn_tensor *input, struct csinn_tensor *output,
                                       struct csinn_tensor *kernel, struct csinn_tensor *bias,
                                       struct csinn_conv2d_params *params)
{
    int32_t batch = input->dim[0];
    int32_t in_ch = input->dim[1];
    int32_t in_h = input->dim[2];
    int32_t in_w = input->dim[3];
    int32_t out_ch = output->dim[1];
    int32_t out_h = output->dim[2];
    int32_t out_w = output->dim[3];
    int32_t kernel_h = kernel->dim[2];
    int32_t kernel_w = kernel->dim[3];
    int32_t stride_h = params->stride_height;
    int32_t stride_w = params->stride_width;
    struct csinn_callback *cb = params->base.cb;

    // xxx: only int4 support nhwc layout now
    if (input->layout == CSINN_LAYOUT_NHWC) {
        out_ch = output->dim[3];
        in_ch = input->dim[3];
        in_h = input->dim[1];
        in_w = input->dim[2];
        kernel_h = kernel->dim[1];
        kernel_w = kernel->dim[2];
        if (kernel_h == 3 && kernel_w == 3 && stride_h == 1 && stride_w == 1) {
            cb->exec = shl_rvv_dwconv3x3s1_int4;
        } else if (kernel_h == 3 && kernel_w == 3 && stride_h == 2 && stride_w == 2) {
            cb->exec = shl_rvv_dwconv3x3s2_int4;
        }
        // support channel quantization
        for (int i = 0; i < kernel->quant_channel; i++) {
            float real_scale = input->qinfo->scale * kernel->qinfo[i].scale / output->qinfo->scale;
            shl_quantize_multiplier(real_scale, &(kernel->qinfo[i].multiplier),
                                    &(kernel->qinfo[i].shift));
        }
        return CSINN_TRUE;
    }
    return CSINN_FALSE;
}
