// Copyright 2019 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/tensor.h"
#include "mediapipe/framework/port/ret_check.h"

namespace mediapipe {

// A calculator for converting Tensors to to a float or a float vector.
//
// Input:
//  TENSORS - Vector of Tensors of type kFloat32. Only the first
//            tensor will be used.
// Output:
//  FLOAT(optional) - Converted single float number.
//  FLOATS(optional) - Converted float vector.
//
// Notes: To output FLOAT stream, the input tensor must have size 1, e.g.
//        only 1 float number in the tensor.
//
// Usage example:
// node {
//   calculator: "TensorsToFloatsCalculator"
//   input_stream: "TENSORS:tensors"
//   output_stream: "FLOATS:floats"
// }
class TensorsToFloatsCalculator : public CalculatorBase {
 public:
  static ::mediapipe::Status GetContract(CalculatorContract* cc);

  ::mediapipe::Status Open(CalculatorContext* cc) override;

  ::mediapipe::Status Process(CalculatorContext* cc) override;
};
REGISTER_CALCULATOR(TensorsToFloatsCalculator);

::mediapipe::Status TensorsToFloatsCalculator::GetContract(
    CalculatorContract* cc) {
  RET_CHECK(cc->Inputs().HasTag("TENSORS"));
  RET_CHECK(cc->Outputs().HasTag("FLOATS") || cc->Outputs().HasTag("FLOAT"));

  cc->Inputs().Tag("TENSORS").Set<std::vector<Tensor>>();
  if (cc->Outputs().HasTag("FLOATS")) {
    cc->Outputs().Tag("FLOATS").Set<std::vector<float>>();
  }
  if (cc->Outputs().HasTag("FLOAT")) {
    cc->Outputs().Tag("FLOAT").Set<float>();
  }

  return ::mediapipe::OkStatus();
}

::mediapipe::Status TensorsToFloatsCalculator::Open(CalculatorContext* cc) {
  cc->SetOffset(TimestampDiff(0));

  return ::mediapipe::OkStatus();
}

::mediapipe::Status TensorsToFloatsCalculator::Process(CalculatorContext* cc) {
  RET_CHECK(!cc->Inputs().Tag("TENSORS").IsEmpty());

  const auto& input_tensors =
      cc->Inputs().Tag("TENSORS").Get<std::vector<Tensor>>();
  // TODO: Add option to specify which tensor to take from.
  auto view = input_tensors[0].GetCpuReadView();
  auto raw_floats = view.buffer<float>();
  int num_values = input_tensors[0].shape().num_elements();

  if (cc->Outputs().HasTag("FLOAT")) {
    // TODO: Could add an index in the option to specifiy returning one
    // value of a float array.
    RET_CHECK_EQ(num_values, 1);
    cc->Outputs().Tag("FLOAT").AddPacket(
        MakePacket<float>(raw_floats[0]).At(cc->InputTimestamp()));
  }
  if (cc->Outputs().HasTag("FLOATS")) {
    auto output_floats = absl::make_unique<std::vector<float>>(
        raw_floats, raw_floats + num_values);
    cc->Outputs().Tag("FLOATS").Add(output_floats.release(),
                                    cc->InputTimestamp());
  }

  return ::mediapipe::OkStatus();
}
}  // namespace mediapipe
