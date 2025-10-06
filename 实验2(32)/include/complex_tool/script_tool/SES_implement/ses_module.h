#pragma once

#include <string>

#include "tool/container.hpp"
#include "complex_tool/script_tool/scope_visitor.h"
#include "ses_code.h"

namespace ses {
	struct ModuleConfig;

	class Function {
	public:
	private:
		std::string name_;
	};

	//单个模组,对应一个SESModule文件中的内容
	//存储module的实例
	class Module {
	public:
		using StructTemplate = VariableManager::StructTemplate;
		using StructTemplateContainer = VariableManager::StructTemplateContainer;
		using ScopeNotFound = ScopeVisitor::ScopeNotFound;
		using ScopeList = ScopeVisitor::ScopeList;
		//注意:此处对function_container使用了std::move
		Module(
			const std::string& name,
			const std::string& struct_data,
			IndexedMap<std::string, Function>& function_container
		);

		const std::string& name()const;

		std::optional<std::size_t> find_function(const std::string& identifier)const;
		std::optional<std::size_t> find_type(const std::string& identifier)const;

		//用于检查脚本作用域是否包含该模组的作用域
		//若缺乏作用域,会返回缺乏信息
		std::optional<ScopeNotFound> check_scope(
			const ScopeVisitor& scope
		)const;
	private:
		std::string name_;
		IndexedMap<std::string, Function> function_container_;
		StructTemplateContainer struct_template_container_;
		std::shared_ptr<ModuleConfig> module_config_;
	};

	class ModuleVisitor;

	//提供对模组中组件(函数,结构体)的访问
	//实际储存模组组件
	//该结构只能由对应编译器生成,且在编译后不允许改动
	//对其他部分提供只读访问,因此保证线程安全
	//该组件由不同的模块分别创建实例,使得不同模块的脚本部分相互独立
	class ModuleManager {
	public:
		class FunctionPtr {
			friend ModuleManager;
		public:
			FunctionPtr(std::size_t module_index, std::size_t pointer);
		private:
			std::size_t module_index_, pointer_;
		};
		class TypePtr {
			friend ModuleManager;
		public:
			TypePtr(std::size_t module_index, std::size_t pointer);
		private:
			std::size_t module_index_, pointer_;
		};

		std::optional<std::vector<std::string>> init_sub_visitor(
			std::vector<std::string>& init_list,
			ModuleVisitor& sub_visitor
		)const;
	private:
		IndexedMap<std::string, Module> container_;
	};

	//子模组管理器,专门用于为脚本提供查询服务
	//为各个脚本分别创建,使不同脚本能包含不同组合的模组
	//与SESModuleManager的关系类似于VariableManager/FunctionManager和ScopeVisitor
	class ModuleVisitor {
		friend ModuleManager;
	public:
		using FunctionPtr = ModuleManager::FunctionPtr;
		using TypePtr = ModuleManager::TypePtr;
		using ScopeNotFound = ScopeVisitor::ScopeNotFound;

		void get_module_list(std::vector<std::string>& module_list)const;

		std::optional<std::pair<FunctionPtr, std::string>> find_function(const std::string& identifier)const;
		std::optional<std::pair<TypePtr, std::string>> find_type(const std::string& identifier)const;

		std::optional<std::vector<std::string>> init_sub_visitor(
			std::vector<std::string>& init_list,
			ModuleVisitor& sub_visitor
		)const;

		struct InvalidModule {
			std::vector<std::pair<std::string, ScopeNotFound>> invalid_list;
		};
		std::optional<InvalidModule> check_scope(
			const ScopeVisitor& scope
		)const;
		//用于移除作用域大于传入作用域的模组
		void remove(const std::vector<std::string>& remove_list);
	private:
		IndexedMap<std::string, const Module*> container_;
	};
}