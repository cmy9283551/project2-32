#pragma once

#include "ses_builder.h"

namespace ses {

	struct CompilerDependence {
		ModuleManager* module_manager = nullptr;
		ScriptLibrary* script_library = nullptr;
		const ScopeVisitor* scope_visitor = nullptr;
		const ModuleConfig* default_module_config = nullptr;
		const ScriptConfig* default_script_config = nullptr;
	};
	//编译器可以被继承,以适配不同的需求,并允许各部分覆写编译过程
	//初始版本的编译器提供了单线程编译标准SES的实现
	class Compiler {
	public:
		Compiler(const CompilerDependence& dependence);
		virtual ~Compiler() = default;


	protected:
		const CompilerDependence* dependence_ = nullptr;
	};

	class MultiThreadCompiler : public Compiler {
	public:
		MultiThreadCompiler(const CompilerDependence& dependence);
		~MultiThreadCompiler() = default;
	private:
	};
}