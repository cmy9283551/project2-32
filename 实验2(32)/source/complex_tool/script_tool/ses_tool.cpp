#include "complex_tool/script_tool/ses_tool.h"

#include "complex_tool/script_tool/SES_implementation/ses_compiler.h"

SESCompiler::SESCompiler(const SESCompilerDependence& dependence) {
	impl_ = std::make_unique<ses::Compiler>(dependence);
}