#pragma once

#include "variable_manager.h"
#include "function_manager.h"

#include <set>

//作用域管理器,用于管理查找VariableManager,FunctionManager中的对象
//提供对类名,函数名,变量名是否存在的查询
//还可以提供变量的常指针(ConstDataPtr)
// 
//该模块仅提供只读访问,因此保证线程安全
// 
//该类设计的目的是在多线程场景下分担VariableManager,FunctionManager的访问压力
//或提供安全的名称查找方式,保护实际的资源
//以及为某个模块提供可定制的作用域,为模块脚本的类名,函数名,变量名查找提供作用域
//使选择添加作用域,调用模块外资源的操作不需要编译模块执行
//而是由模块自由选择,增加编译模块的可复用性
class ScopeVisitor {
public:
	ScopeVisitor() = default;
	ScopeVisitor(
		const std::vector<const VariableManager*>& vm_ptr_list,
		const std::vector<const FunctionManager*>& fm_ptr_list
	);
	//内容与ScopeList一致,单独列出作为提示
	struct ScopeNotFound {
		std::vector<std::string> variable_scope;
		std::vector<std::string> function_scope;

		//支持错误信息叠加
		void operator+=(const ScopeNotFound& that);
	};
	struct ScopeVector {
		std::vector<std::string> variable_scope;
		std::vector<std::string> function_scope;
	};
	//生成子作用域
	//注意,这里没有问题返回std::nullopt,有问题返回问题信息
	std::optional<ScopeNotFound> init_sub_scope(
		const std::vector<std::string>& vm_vector,
		const std::vector<std::string>& fm_vector,
		ScopeVisitor& sub_scope
	)const;
	std::optional<ScopeNotFound> init_sub_scope(
		const ScopeVector& scope_vector,
		ScopeVisitor& sub_scope
	)const;

	void get_scope_vector(
		std::vector<std::string>& vm_vector,
		std::vector<std::string>& fm_vector
	)const;
	void get_scope_vector(ScopeVector& scope_vector)const;

	std::optional<const VariableManager*> find_vm(
		const std::string& name
	)const;
	std::optional<const FunctionManager*> find_fm(
		const std::string& name
	)const;

	bool insert_vm(const VariableManager* vm_ptr);
	bool insert_fm(const FunctionManager* fm_ptr);

	enum class IdentifierType {
		Variable,
		TypeName,
		Function,
		Null
	};
	std::expected<IdentifierType, std::string> identify(const std::string& name)const;

	bool copy(const ScopeVisitor& that);
private:
	//检查新加入的作用域指针,包括对指针的有效性,名称冲突的检查
	bool check_new_scope(const VariableManager* vm_ptr);
	//检查新加入的作用域指针,包括对指针的有效性,名称冲突的检查
	bool check_new_scope(const FunctionManager* fm_ptr);
	bool check_name_conflict(const std::vector<std::string>& name_vector);

	//用于检查名称冲突
	std::set<std::string> name_space_;

	IndexedMap<std::string, const VariableManager*> vm_ptr_container_;
	IndexedMap<std::string, const FunctionManager*> fm_ptr_container_;
};