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

#ifndef VERIBLE_COMMON_ANALYSIS_SYNTAX_TREE_LINTER_TEST_UTILS_H_
#define VERIBLE_COMMON_ANALYSIS_SYNTAX_TREE_LINTER_TEST_UTILS_H_

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "verible/common/analysis/lint-rule-status.h"
#include "verible/common/analysis/linter-test-utils.h"
#include "verible/common/analysis/syntax-tree-lint-rule.h"  // IWYU pragma: export
#include "verible/common/analysis/syntax-tree-linter.h"
#include "verible/common/text/text-structure.h"
#include "verible/common/util/logging.h"

namespace verible {

template <>
class LintRunner<SyntaxTreeLintRule> {
 public:
  explicit LintRunner(std::unique_ptr<SyntaxTreeLintRule> rule) {
    linter_.AddRule(std::move(rule));
  }

  LintRuleStatus Run(const TextStructureView &text_structure,
                     std::string_view) {
    linter_.Lint(*ABSL_DIE_IF_NULL(text_structure.SyntaxTree()));
    // Looking for one type of rule violation at a time.
    CHECK_EQ(linter_.ReportStatus().size(), 1);
    return linter_.ReportStatus()[0];
  }

 private:
  SyntaxTreeLinter linter_;
};

}  // namespace verible

#endif  // VERIBLE_COMMON_ANALYSIS_SYNTAX_TREE_LINTER_TEST_UTILS_H_
