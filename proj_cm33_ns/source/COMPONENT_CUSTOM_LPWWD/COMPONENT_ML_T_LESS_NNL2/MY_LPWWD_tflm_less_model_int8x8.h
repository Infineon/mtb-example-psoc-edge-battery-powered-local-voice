// This file is generated. Do not edit.
// Generated on: 30.06.2026 11:36:32
// Build version: 3.3.0.16777

#ifndef MY_LPWWD_GEN_H
#define MY_LPWWD_GEN_H

#include "tensorflow/lite/c/common.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define MY_LPWWD_MODEL_CONST_DATA_SIZE 131442
#define MY_LPWWD_MODEL_INIT_DATA_SIZE 4708
#define MY_LPWWD_MODEL_UNINIT_DATA_SIZE 1396

#define MY_LPWWD_MODEL_ARENA_SIZE 692

// Sets up the model with init and prepare steps.
TfLiteStatus MY_LPWWD_init();
// Returns the input tensor with the given index.
TfLiteTensor *MY_LPWWD_input(int index);
// Returns the output tensor with the given index.
TfLiteTensor *MY_LPWWD_output(int index);
// Runs inference for the model.
TfLiteStatus MY_LPWWD_invoke();
// Resets all variable tensors
TfLiteStatus MY_LPWWD_reset();

// Returns the number of input tensors.
size_t MY_LPWWD_inputs();

// Returns the number of output tensors.
size_t MY_LPWWD_outputs();

// Return the buffer pointer of input tensor
void *MY_LPWWD_input_ptr(int index);

// Return the buffer size of input tensor
size_t MY_LPWWD_input_size(int index);

// Return the dimention size of input tensor
int MY_LPWWD_input_dims_len(int index);

// Return the dimention buffer pointer of input tensor
int *MY_LPWWD_input_dims(int index);

// Return the buffer pointer of output tensor
void *MY_LPWWD_output_ptr(int index);

// Return the buffer size of output tensor
size_t MY_LPWWD_output_size(int index);

// Return the dimention size of output tensor
int MY_LPWWD_output_dims_len(int index);

// Return the dimention buffer pointer of output tensor
int *MY_LPWWD_output_dims(int index);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif
