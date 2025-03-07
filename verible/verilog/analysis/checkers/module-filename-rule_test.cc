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

#include "verible/verilog/analysis/checkers/module-filename-rule.h"

#include <initializer_list>
#include <string>
#include <string_view>

#include "gtest/gtest.h"
#include "verible/common/analysis/linter-test-utils.h"
#include "verible/common/analysis/text-structure-linter-test-utils.h"
#include "verible/verilog/analysis/verilog-analyzer.h"
#include "verible/verilog/parser/verilog-token-enum.h"

namespace verilog {
namespace analysis {
namespace {

using verible::LintTestCase;
using verible::RunApplyFixCases;
using verible::RunConfiguredLintTestCases;
using verible::RunLintTestCases;

// Expected token type of findings.
constexpr int kToken = SymbolIdentifier;

// Test that no violations are found with an empty filename.
TEST(ModuleFilenameRuleTest, BlankFilename) {
  const std::initializer_list<LintTestCase> kTestCases = {
      {""},
      {"module m; endmodule"},
      {"class c; endclass"},
  };
  const std::string filename;
  RunLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(kTestCases, filename);
}

// Test that as long as one module matches file name, no violations reported.
TEST(ModuleFilenameRuleTest, ModuleMatchesFilename) {
  const std::initializer_list<LintTestCase> kTestCases = {
      {""},
      {"module m; endmodule"},
      {"module n; endmodule\nmodule m; endmodule"},
      {"module m; endmodule\nmodule n; endmodule"},
  };
  const std::string filename = "/path/to/m.sv";
  RunLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(kTestCases, filename);
}

TEST(ModuleFilenameRuleTest, DashAllowedWhenConfigured) {
  const std::initializer_list<LintTestCase> kOkCases = {
      {"module multi_word_module; endmodule"},
  };
  const std::initializer_list<LintTestCase> kComplaintCases = {
      {"module ", {kToken, "multi_word_module"}, "; endmodule"},
  };

  const std::string f_with_underscore = "/path/to/multi_word_module.sv";
  const std::string f_with_dash = "/path/to/multi-word-module.sv";
  {
    // With dashes not allowed, we only accept the underscore name
    constexpr std::string_view config = "allow-dash-for-underscore:off";
    RunConfiguredLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(
        kOkCases, config, f_with_underscore);
    RunConfiguredLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(
        kComplaintCases, config, f_with_dash);
  }

  {
    // ... But with dashes allowed, dashes are also an ok case.
    constexpr std::string_view config = "allow-dash-for-underscore:on";
    // With dashes not allowed, we only accept the underscore name
    RunConfiguredLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(
        kOkCases, config, f_with_underscore);
    RunConfiguredLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(
        kOkCases, config, f_with_dash);
  }
}

// Test more unusual file names with multiple dots in them.
TEST(ModuleFilenameRuleTest, ModuleMatchesMultiDotComponentFilename) {
  const std::initializer_list<LintTestCase> kTestCases = {
      {""},
      {"package q; endpackage\n"},
      {"module m; endmodule\n"},
      {"module n; endmodule\nmodule m; endmodule"},
      {"module m; endmodule\nmodule n; endmodule"},
  };
  RunLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(kTestCases,
                                                        "/path/to/m");
  RunLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(kTestCases,
                                                        "/path/to/m.v");
  RunLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(kTestCases,
                                                        "/path/to/m.sv");
  RunLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(kTestCases,
                                                        "/path/to/m.stub.sv");
  RunLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(
      kTestCases, "/path/to/m.behavioral.model.sv");
}

// Test that some violations are found checked against a filename.
TEST(ModuleFilenameRuleTest, NoModuleMatchesFilenameAbsPath) {
  const std::initializer_list<LintTestCase> kTestCases = {
      {""},
      {"package q; endpackage\n"},
      {"module ", {kToken, "m"}, "; endmodule"},
      {"module m; endmodule\nmodule ", {kToken, "n"}, "; endmodule"},
      {"module ",
       {kToken, "m"},
       ";\n"
       "  module q;\n"  // Matches, but is not an outermost declaration
       "  endmodule : q\n"
       "endmodule : m"},
      {"module ",
       {kToken, "m"},
       ";\n"
       "  module foo;\n"
       "  endmodule : foo\n"
       "  module q;\n"
       "  endmodule : q\n"
       "endmodule : m"},
      {"module ",
       {kToken, "m"},
       ";\n"
       "  module foo;\n"
       "    module q;\n"
       "    endmodule : q\n"
       "  endmodule : foo\n"
       "endmodule : m"},
  };
  RunLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(kTestCases,
                                                        "/path/to/q.sv");
  RunLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(kTestCases,
                                                        "/path/to/q.stub.sv");
  RunLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(kTestCases,
                                                        "/path/to/q.m.sv");
}

// Test that some violations are found checked against a filename.
TEST(ModuleFilenameRuleTest, NoModuleMatchesFilenameRelPath) {
  const std::initializer_list<LintTestCase> kTestCases = {
      {""},
      {"module ", {kToken, "m"}, "; endmodule"},
      {"module m; endmodule\nmodule ", {kToken, "n"}, "; endmodule"},
      {"module ",
       {kToken, "m"},
       ";\n"
       "  module r;\n"  // Matches, but is not an outermost declaration
       "  endmodule : r\n"
       "endmodule : m"},
      {"module ",
       {kToken, "m"},
       ";\n"
       "  module foo;\n"
       "  endmodule : foo\n"
       "  module r;\n"
       "  endmodule : r\n"
       "endmodule : m"},
      {"module ",
       {kToken, "m"},
       ";\n"
       "  module foo;\n"
       "    module r;\n"
       "    endmodule : r\n"
       "  endmodule : foo\n"
       "endmodule : m"},
  };
  const std::string filename = "path/to/r.sv";
  RunLintTestCases<VerilogAnalyzer, ModuleFilenameRule>(kTestCases, filename);
}

TEST(ModuleFilenameRuleTest, AutoFixModuleFilenameRule) {
  const std::initializer_list<verible::AutoFixInOut> kTestCases = {
      {"module a;\n\nendmodule", "module r;\n\nendmodule"},
      {"module some_name1;\n\nendmodule", "module r;\n\nendmodule"},
      {"module some_name2();\n\nendmodule", "module r();\n\nendmodule"},
      {"module some_name3#()();\n\nendmodule", "module r#()();\n\nendmodule"},
      {"module a;\n\nendmodule : a", "module r;\n\nendmodule : r"},
      {"module some_name1;\n\nendmodule: some_name1",
       "module r;\n\nendmodule: r"},
      {"module some_name2();\n\nendmodule :some_name2",
       "module r();\n\nendmodule :r"},
      {"module some_name3#()();\n\nendmodule:some_name3",
       "module r#()();\n\nendmodule:r"},

  };
  const std::string filename = "path/to/r.sv";
  RunApplyFixCases<VerilogAnalyzer, ModuleFilenameRule>(kTestCases, "",
                                                        filename);
}

TEST(ModuleFilenameRuleTest, AutoFixModuleFilenameRuleWithDashes) {
  const std::initializer_list<verible::AutoFixInOut> kTestCases = {
      {"module a;\n\nendmodule", "module file_with_dashes;\n\nendmodule"},
      {"module some_name1;\n\nendmodule",
       "module file_with_dashes;\n\nendmodule"},
      {"module some_name2();\n\nendmodule",
       "module file_with_dashes();\n\nendmodule"},
      {"module some_name3#()();\n\nendmodule",
       "module file_with_dashes#()();\n\nendmodule"},
      {"module a;\n\nendmodule : a",
       "module file_with_dashes;\n\nendmodule : file_with_dashes"},
      {"module some_name1;\n\nendmodule :some_name1",
       "module file_with_dashes;\n\nendmodule :file_with_dashes"},
      {"module some_name2();\n\nendmodule: some_name2",
       "module file_with_dashes();\n\nendmodule: file_with_dashes"},
      {"module some_name3#()();\n\nendmodule:some_name3",
       "module file_with_dashes#()();\n\nendmodule:file_with_dashes"},
  };
  const std::string filename = "path/to/file-with-dashes.sv";
  RunApplyFixCases<VerilogAnalyzer, ModuleFilenameRule>(
      kTestCases, "allow-dash-for-underscore:on", filename);
}

TEST(ModuleFilenameRuleTest, AutoFixModuleFilenameRuleWithUnderscore) {
  const std::initializer_list<verible::AutoFixInOut> kTestCases = {
      {"module a;\n\nendmodule", "module file_no_dashes;\n\nendmodule"},
      {"module some_name1;\n\nendmodule",
       "module file_no_dashes;\n\nendmodule"},
      {"module some_name2();\n\nendmodule",
       "module file_no_dashes();\n\nendmodule"},
      {"module some_name3#()();\n\nendmodule",
       "module file_no_dashes#()();\n\nendmodule"},
      {"module a;\n\nendmodule : a",
       "module file_no_dashes;\n\nendmodule : file_no_dashes"},
      {"module some_name1;\n\nendmodule :some_name1",
       "module file_no_dashes;\n\nendmodule :file_no_dashes"},
      {"module some_name2();\n\nendmodule: some_name2",
       "module file_no_dashes();\n\nendmodule: file_no_dashes"},
      {"module some_name3#()();\n\nendmodule:some_name3",
       "module file_no_dashes#()();\n\nendmodule:file_no_dashes"},
  };
  const std::string filename = "path/to/file_no_dashes.sv";
  RunApplyFixCases<VerilogAnalyzer, ModuleFilenameRule>(
      kTestCases, "allow-dash-for-underscore:off", filename);
  RunApplyFixCases<VerilogAnalyzer, ModuleFilenameRule>(
      kTestCases, "allow-dash-for-underscore:on", filename);
}

}  // namespace
}  // namespace analysis
}  // namespace verilog
