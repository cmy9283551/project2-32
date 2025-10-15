#pragma once

#include <string>

#include "tool/container.hpp"
#include "complex_tool/script_tool/scope_visitor.h"
#include "ses_midcode.h"

namespace ses {
	//存放模组配置信息
	
	struct ModuleConfig;

	class ModuleFunction {
	public:
	private:
		std::string name_;
	};

	//单个模组,对应一个SESModule文件中的内容
	//存储module的实例
	class Module {
	public:
		using ScopeNotFound = ScopeVisitor::ScopeNotFound;
		using ScopeVector = ScopeVisitor::ScopeVector;
		//注意:此处对function_container使用了std::move
		Module(
			const std::string& name,
			std::unique_ptr<StructTemplateContainer> struct_template_container,
			IndexedMap<std::string, ModuleFunction>& function_container,
			std::unique_ptr<ModuleConfig> module_config
		);

		const std::string& name()const;

		std::optional<std::size_t> find_function(const std::string& identifier)const;
		std::optional<StructProxy> find_type(const std::string& identifier)const;

		//用于检查脚本作用域是否包含该模组的作用域
		//若缺乏作用域,会返回缺乏信息
		std::optional<ScopeNotFound> check_scope(
			const ScopeVisitor& scope
		)const;

		enum class IdentifierType {
			Function,
			TypeName
		};
		std::optional<IdentifierType> identify(const std::string& identifier)const;
	private:
		std::string name_;
		std::unique_ptr<StructTemplateContainer> struct_template_container_ = nullptr;
		IndexedMap<std::string, ModuleFunction> function_container_;
		std::unique_ptr<ModuleConfig> module_config_ = nullptr;
	};

	class ModuleVisitor;

	//提供对模组中组件(函数,结构体)的访问
	//实际储存模组组件
	//该结构只能由对应编译器生成,且在编译后不允许改动
	//对其他部分提供只读访问,因此保证线程安全
	//该组件由不同的模块分别创建实例,使得不同模块的脚本部分相互独立
	// 
	//对名称唯一性的保证:
	//插入时检查是否存在同名模组,若存在则覆盖
	//因此保证ModuleManager中不会存在同名模组
	//插入时检查模组中函数和类型名称是否与已有名称冲突,若冲突则插入失败
	//因此保证ModuleManager中不会存在同名函数和类型名称	
	//由于一个模块的模组只能使用该模块提供的作用域
	//且一个模块只能存在一个ModuleManager
	//因此保证不会存在作用域冲突
	//又因为编译模组时会检查模组中成员名称是否合法(即是否与作用域中名称冲突)
	//因此保证模组中成员名称不会与作用域中名称冲突
	class ModuleManager {
	public:
		class FunctionPtr {
		public:
			FunctionPtr(std::size_t module_index, std::size_t pointer);
		private:
			std::size_t module_index_, pointer_;
		};

		std::optional<std::vector<std::string>> init_sub_visitor(
			std::vector<std::string>& init_vector,
			ModuleVisitor& sub_visitor
		)const;

		const std::string& name()const;
	private:
		//插入module只能让ModuleManager的友元类调用,即手动给与许可的类使用
		//插入时检查是否存在同名模组,若存在则覆盖
		//因此保证ModuleManager中不会存在同名模组
		//插入时检查模组中函数和类型名称是否与已有名称冲突,若冲突则插入失败
		//因此保证ModuleManager中不会存在同名函数和类型名称
		bool insert_module(
			const std::string& name,
			std::unique_ptr<StructTemplateContainer> struct_template_container,
			IndexedMap<std::string, ModuleFunction>& function_container,
			std::unique_ptr<ModuleConfig> module_config
		);

		std::string name_;
		//用于检查模组内容是否存在名称冲突
		//由于一切module在编译时会生成作用域,而作用域由模块总作用域生成
		// 因此保证不会存在作用域冲突
		std::set<std::string> name_space_;
		IndexedMap<std::string, Module> modules_;
	};

	//子模组管理器,专门用于为脚本提供查询服务
	//为各个脚本分别创建,使不同脚本能包含不同组合的模组
	//与SESModuleManager的关系类似于VariableManager/FunctionManager和ScopeVisitor
	//只能由一个ModuleManager初始化,因此保证不会有模组间名称冲突
	class ModuleVisitor {
		friend ModuleManager;
	public:
		using FunctionPtr = ModuleManager::FunctionPtr;
		using ScopeNotFound = ScopeVisitor::ScopeNotFound;
		using IdentifierType = Module::IdentifierType;

		void get_module_vector(std::vector<std::string>& module_vector)const;

		std::optional<FunctionPtr> find_function(
			const std::string& identifier
		)const;
		std::optional<StructProxy> find_type(
			const std::string& identifier
		)const;

		std::optional<std::vector<std::string>> init_sub_visitor(
			std::vector<std::string>& init_vector,
			ModuleVisitor& sub_visitor
		)const;

		struct InvalidModule {
			std::vector<std::pair<std::string, ScopeNotFound>> invalid_vector;
		};
		//用于检查脚本作用域是否包含该模组的作用域
		std::optional<InvalidModule> check_scope(
			const ScopeVisitor& scope
		)const;
		//用于移除作用域大于传入作用域的模组
		void remove(const std::vector<std::string>& remove_vector);

		std::optional<IdentifierType> identify(const std::string& identifier)const;
	private:
		IndexedMap<std::string, const Module*> modules_;
	};

	struct ModuleConfig {
		ScopeVisitor scope_visitor;
		ModuleVisitor module_visitor;
	};
}