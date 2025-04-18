/* Copyright 2025 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_LITE_MICRO_KERNELS_ACTIVATIONS_H_
#define TENSORFLOW_LITE_MICRO_KERNELS_ACTIVATIONS_H_

#include <cstdint>

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/common.h"
#include "tensorflow/lite/kernels/internal/types.h"

namespace tflite {

extern const int kActivationsInputTensor;
extern const int kActivationsOutputTensor;

struct ReluOpData {
  ReluParams params;
};

struct Relu6OpData {
  int32_t six;
  int32_t zero;
};

template <typename T>
void ReluQuantized(const ReluOpData& data, const RuntimeShape& input_shape,
                   const RuntimeShape& output_shape, const T* input_data,
                   T* output_data) {
  const int flat_size = MatchingFlatSize(input_shape, output_shape);
  for (int i = 0; i < flat_size; ++i) {
    const int32_t val = static_cast<int32_t>(input_data[i]);
    int32_t clamped =
        data.params.output_offset +
        MultiplyByQuantizedMultiplier(val - data.params.input_offset,
                                      data.params.output_multiplier,
                                      data.params.output_shift);
    clamped = std::max(data.params.quantized_activation_min, clamped);
    clamped = std::min(data.params.quantized_activation_max, clamped);
    output_data[i] = static_cast<T>(clamped);
  }
}

template <typename T>
void CalculateReluOpData(const TfLiteTensor* input, TfLiteTensor* output,
                         ReluOpData* data);

void ReluFloat(const RuntimeShape& input_shape, const float* input_data,
               const RuntimeShape& output_shape, float* output_data);

void Relu6Float(const RuntimeShape& input_shape, const float* input_data,
                const RuntimeShape& output_shape, float* output_data);

template <typename T>
void Relu6Quantized(T lower, T upper, const RuntimeShape& input_shape,
                    const T* input_data, const RuntimeShape& output_shape,
                    T* output_data) {
  const int flat_size = MatchingFlatSize(input_shape, output_shape);
  for (int i = 0; i < flat_size; ++i) {
    const T val = input_data[i];
    const T clamped = val > upper ? upper : val < lower ? lower : val;
    output_data[i] = clamped;
  }
}

TfLiteStatus ReluPrepare(TfLiteContext* context, TfLiteNode* node);

TfLiteStatus Relu6Prepare(TfLiteContext* context, TfLiteNode* node);

}  // namespace tflite

#endif  // TENSORFLOW_LITE_MICRO_KERNELS_ACTIVATIONS_H_
