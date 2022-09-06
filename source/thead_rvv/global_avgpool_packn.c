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

/*************************************************************
 * note: VLEN = 128/256 ... flexible vlen
 *************************************************************/
int shl_rvv_global_avgpool2d_packn_fp32(struct csinn_tensor *input, struct csinn_tensor *output,
                                        struct csinn_pool_params *params)
{
    float *input_data = (float *)input->data;
    float *output_data = (float *)output->data;

    int batch = input->dim[0];
    int in_c = input->dim[1];
    int in_h = input->dim[2];
    int in_w = input->dim[3];
    int in_hw = in_h * in_w;

    const int packn = csrr_vlenb() / sizeof(float);
    const int vl = vsetvl_e32m1(packn);

    for (int b = 0; b < batch; b++) {
        for (int c = 0; c + packn - 1 < in_c; c += packn) {
            vfloat32m1_t _acc = vle32_v_f32m1(input_data, vl);
            input_data += packn;
            for (int i = 1; i < in_hw; i++) {
                _acc = vfadd_vv_f32m1(_acc, vle32_v_f32m1(input_data, vl), vl);
                input_data += packn;
            }
            vfloat32m1_t _avg = vfmul_vf_f32m1(_acc, 1.0f / (float)in_hw, vl);
            vse32_v_f32m1(output_data, _avg, vl);
            output_data += packn;
        }
    }
    return CSINN_TRUE;
}

int shl_rvv_global_avgpool2d_packn_fp16(struct csinn_tensor *input, struct csinn_tensor *output,
                                        struct csinn_pool_params *params)
{
    __fp16 *input_data = (__fp16 *)input->data;
    __fp16 *output_data = (__fp16 *)output->data;

    int batch = input->dim[0];
    int in_c = input->dim[1];
    int in_h = input->dim[2];
    int in_w = input->dim[3];
    int in_hw = in_h * in_w;

    const int packn = csrr_vlenb() / sizeof(__fp16);
    const int vl = vsetvl_e16m1(packn);

    for (int b = 0; b < batch; b++) {
        for (int c = 0; c + packn - 1 < in_c; c += packn) {
            vfloat16m1_t _acc = vle16_v_f16m1(input_data, vl);
            input_data += packn;
            for (int i = 1; i < in_hw; i++) {
                _acc = vfadd_vv_f16m1(_acc, vle16_v_f16m1(input_data, vl), vl);
                input_data += packn;
            }
            vfloat16m1_t _avg = vfmul_vf_f16m1(_acc, 1.0f / in_hw, vl);
            vse16_v_f16m1(output_data, _avg, vl);
            output_data += packn;
        }
    }
    return CSINN_TRUE;
}

/* int8 --> fp16 acc --> int8 */
int shl_rvv_global_avgpool2d_packn_int8(struct csinn_tensor *input, struct csinn_tensor *output,
                                        struct csinn_pool_params *params)
{
#ifdef RVV_1_0_0
    int8_t *input_data = (int8_t *)input->data;
    int8_t *output_data = (int8_t *)output->data;

    int batch = input->dim[0];
    int in_c = input->dim[1];
    int in_h = input->dim[2];
    int in_w = input->dim[3];
    int in_hw = in_h * in_w;

    const int packn = csrr_vlenb() / sizeof(int8_t) / 2;
    const int vl = vsetvl_e8mf2(packn);

    for (int b = 0; b < batch; b++) {
        for (int c = 0; c + packn - 1 < in_c; c += packn) {
            vint8mf2_t _input = vle8_v_i8mf2(input_data, vl);
            input_data += packn;
            vint16m1_t _tmp = vwsub_vx_i16m1(_input, (int8_t)input->qinfo->zero_point, vl);
            vfloat16m1_t _acc =
                vfmul_vf_f16m1(vfcvt_f_x_v_f16m1(_tmp, vl), input->qinfo->scale, vl);
            for (int i = 1; i < in_hw; i++) {
                _tmp = vwsub_vx_i16m1(vle8_v_i8mf2(input_data, vl),
                                      (int8_t)input->qinfo->zero_point, vl);
                vfloat16m1_t _inputf =
                    vfmul_vf_f16m1(vfcvt_f_x_v_f16m1(_tmp, vl), input->qinfo->scale, vl);
                _acc = vfadd_vv_f16m1(_acc, _inputf, vl);
                input_data += packn;
            }
            vfloat16m1_t _avg = vfmul_vf_f16m1(_acc, 1.0f / in_hw / output->qinfo->scale, vl);
            _avg = vfadd_vf_f16m1(_avg, output->qinfo->zero_point, vl);
            vint16m1_t _output = vfcvt_x_f_v_i16m1(_avg, vl);
            vint8mf2_t _res = vnclip_wx_i8mf2(_output, 0, vl);
            vse8_v_i8mf2(output_data, _res, vl);
            output_data += packn;
        }
    }
    return CSINN_TRUE;
#elif define RVV_0_7_1
    shl_debug_error("unsupport global_avgpool2d packn for int8 on rvv_spec 0.7.1\n");
    return CSINN_FALSE;
#endif
}
