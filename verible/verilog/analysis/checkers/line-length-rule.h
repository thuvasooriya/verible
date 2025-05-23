// Copyright 2017-2020 The Verible Authors.
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

#ifndef VERIBLE_VERILOG_ANALYSIS_CHECKERS_LINE_LENGTH_RULE_H_
#define VERIBLE_VERILOG_ANALYSIS_CHECKERS_LINE_LENGTH_RULE_H_

#include <set>
#include <string_view>

#include "absl/status/status.h"
#include "verible/common/analysis/lint-rule-status.h"
#include "verible/common/analysis/text-structure-lint-rule.h"
#include "verible/common/text/text-structure.h"
#include "verible/verilog/analysis/descriptions.h"

namespace verilog {
namespace analysis {

// Detects whether any line exceeds the maximum allowed length.
// This uses a TextStructureLintRule instead of a LineLintRule because
// exceptions are granted based on lexical analysis, so line-only
// analysis would be insufficient.
class LineLengthRule : public verible::TextStructureLintRule {
 public:
  enum {
    // Dictated by line-length styleguide rule.
    kDefaultLineLength = 100,
    kMinimumLineLength = 40,
    kMaximumLineLength = 1000,
  };

  using rule_type = verible::TextStructureLintRule;

  static const LintRuleDescriptor &GetDescriptor();

  LineLengthRule() = default;

  absl::Status Configure(std::string_view configuration) final;

  void Lint(const verible::TextStructureView &, std::string_view) final;

  verible::LintRuleStatus Report() const final;

 private:
  int line_length_limit_ = kDefaultLineLength;

  // Collection of found violations.
  std::set<verible::LintViolation> violations_;
};

}  // namespace analysis
}  // namespace verilog

#endif  // VERIBLE_VERILOG_ANALYSIS_CHECKERS_LINE_LENGTH_RULE_H_
