#pragma once

#include "ses_module.h"
#include "complex_tool/script_tool/scope_visitor.h"

//该文件存储具体的脚本格式

namespace ses {
	//存放脚本配置信息
	struct ScriptConfig {
		//输入输出的参数
		ScriptParameter input, output;

		ModuleVisitor module_visitor;
		ScopeVisitor scope_visitor;

		//在创建时根据使用的类型自动构建,
		VariableManager::StructTemplateContainer script_stc;

		friend std::ostream& operator<<(std::ostream& os, const ScriptConfig& config);
	};

	//编译生成的脚本库,用于查找并调用脚本
	class ScriptLibrary {
	public:
	private:
	};

	//最终生成的可执行脚本,包含指令集,内存块
	//和变量管理器指针(以调用统一变量),函数管理器指针(以调用统一函数)
	class ScriptInstance {
	public:

	};
}