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
	//���������Ա��̳�,�����䲻ͬ������,����������ָ�д�������
	//��ʼ�汾�ı������ṩ�˵��̱߳����׼SES��ʵ��
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