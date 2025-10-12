#include "complex_tool/script_tool/SES_implementation/ses_compiler.h"

namespace ses {
	Compiler::Compiler(const CompilerDependence& dependence)
		:dependence_(&dependence) {
	}
	MultiThreadCompiler::MultiThreadCompiler(const CompilerDependence& dependence)
		: Compiler(dependence) {
	}
}