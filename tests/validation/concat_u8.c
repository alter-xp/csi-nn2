/*
 * Copyright (C) 2016-2021 C-SKY Limited. All rights reserved.
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

#include "test_utils.h"
#include "csi_nn.h"
#include "math_snr.h"

int main(int argc, char** argv)
{
    init_testsuite("Testing function of concat u8.\n");   
    int in_size = 1;
    int out_size = 1;
    float error = 0.2f;
    int *buffer = read_input_data_f32(argv[1]);
 
    struct concat_params params;

    params.inputs_count = buffer[4];
    
    struct csi_tensor *output = csi_alloc_tensor(NULL);
    struct csi_tensor *reference = csi_alloc_tensor(NULL);
    struct csi_tensor *input[params.inputs_count];

    for(int i = 0; i < params.inputs_count; i++) {
        input[i] = csi_alloc_tensor(NULL);
    }

    float *src_in[params.inputs_count];
    params.axis = buffer[5];
    output->dim_count = 4;


    for(int i = 0; i < output->dim_count; i++) {
        if ( i == params.axis ){
            output->dim[i] = params.inputs_count*buffer[i];
        }
        else {
            output->dim[i] = buffer[i];
        }       
        out_size *= output->dim[i];
    }
    in_size = out_size / params.inputs_count;
    params.base.api = CSINN_API;
    params.base.run_mode = CSINN_RM_LAYER;

    uint8_t *src_tmp[params.inputs_count];
    for(int i = 0; i < params.inputs_count; i++) {
        src_in[i] = (float *)(buffer + 6 + in_size * i); 
        src_tmp[i] = malloc(in_size * sizeof(char));
    }  

    float *ref      = (float *)(buffer + 6 + in_size * params.inputs_count);

    for(int i = 0; i < params.inputs_count; i++) {
        input[i]->qinfo = get_quant_info(src_in[i], in_size);
        for(int j = 0; j < in_size; j++) {
            src_tmp[i][j] = csi_ref_quantize_f32_to_u8(src_in[i][j], input[i]->qinfo);
        }
        input[i]->data = src_tmp[i];
        input[i]->dim[0] = buffer[0];          // batch
        input[i]->dim[1] = buffer[1];          // height
        input[i]->dim[2] = buffer[2];          // width
        input[i]->dim[3] = buffer[3];          // channel
        input[i]->dim_count = 4;
        input[i]->dtype = CSINN_DTYPE_UINT8;
    }

    output->qinfo = get_quant_info(ref, out_size);

    output->dtype = CSINN_DTYPE_UINT8;
    reference->data = ref;  
    output->data  = (uint8_t *)malloc(out_size * sizeof(uint8_t));
    float difference = argc > 2 ? atof(argv[2]) : error;

    if (csi_concat_init((struct csi_tensor **)input, output, &params) == CSINN_TRUE) {
        csi_concat((struct csi_tensor **)input, output, &params);
    }

    result_verify_8(reference->data, output, input[0]->data, difference, out_size, false);

    free(buffer);
	for(int i = 0; i < params.inputs_count; i++) {
        free(src_tmp[i]);
    }
    free(output->data);
    return done_testing();
}