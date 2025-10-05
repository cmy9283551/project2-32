#pragma once

#include <string>

#include "tool/container.hpp"
#include "complex_tool/script_tool/variable_manager.h"
#include "ses_code.h"

class SESFunction {
public:
private:
	std::string name_;
};

//单个模组,对应一个SESModule文件中的内容
//存储module的实例
class SESModule {
public:
	using StructTemplate = VariableManager::StructTemplate;
	using StructTemplateContainer = VariableManager::StructTemplateContainer;
	//注意:此处对function_container使用了std::move
	SESModule(
		const std::string& struct_data,
		IndexedMap<std::string, SESFunction>& function_container
	);

	std::optional<std::size_t> find_function(const std::string& identifier)const;
	std::optional<std::size_t> find_type(const std::string& identifier)const;
private:
	IndexedMap<std::string, SESFunction> function_container_;
	StructTemplateContainer struct_template_container_;
};

class SESModuleVisitor;

//提供对模组中组件(函数,结构体)的访问
//实际储存模组组件
//该结构只能由对应编译器生成,且在编译后不允许改动
//对其他部分提供只读访问,因此保证线程安全
//该组件由不同的模块分别创建实例,使得不同模块的脚本部分相互独立
class SESModuleManager {
public:
	class FunctionPtr {
		friend SESModuleManager;
	public:
		FunctionPtr(std::size_t module_index, std::size_t pointer);
	private:
		std::size_t module_index_, pointer_;
	};
	class TypePtr {
		friend SESModuleManager;
	public:
		TypePtr(std::size_t module_index, std::size_t pointer);
	private:
		std::size_t module_index_, pointer_;
	};

	std::optional<std::vector<std::string>> init_sub_visitor(
		std::vector<std::string>& init_list,
		SESModuleVisitor& sub_visitor
	)const;
private:
	IndexedMap<std::string, SESModule> container_;
};

//子模组管理器,专门用于为脚本提供查询服务
//为各个脚本分别创建,使不同脚本能包含不同组合的模组
//与SESModuleManager的关系类似于VariableManager/FunctionManager和ScopeVisitor
class SESModuleVisitor {
	friend SESModuleManager;
public:
	using FunctionPtr = SESModuleManager::FunctionPtr;
	using TypePtr = SESModuleManager::TypePtr;

	std::optional<std::pair<FunctionPtr, std::string>> find_function(const std::string& identifier)const;
	std::optional<std::pair<TypePtr, std::string>> find_type(const std::string& identifier)const;


	std::optional<std::vector<std::string>> init_sub_visitor(
		std::vector<std::string>& init_list,
		SESModuleVisitor& sub_visitor
	)const;
private:
	IndexedMap<std::string, const SESModule*> container_;
};