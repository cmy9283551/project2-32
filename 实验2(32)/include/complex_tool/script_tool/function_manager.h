#pragma once

#include <functional>

#include "tool/container.hpp"
#include "tool/script_tool/script_data.h"

class VariableManager;

//为脚本提供内部函数的调用用接口
//FunctionManager为抽象类,便于以后支持多线程等场景
class FunctionManager {
public:
	using Function = std::function<ScriptPackage(const ScriptPackage&)>;

	FunctionManager(const std::string& name);
	virtual ~FunctionManager() = default;

	class FunctionPtr {
	public:
		FunctionPtr(std::size_t index, FunctionManager& function_manager);
		ScriptPackage call(const ScriptPackage& data);
	private:
		std::size_t pointer_;
		FunctionManager* function_manager_;
	};

	class ConstFunctionPtr {
	public:
	private:
		std::size_t pointer_;
		const FunctionManager* function_manager_;
	};

	virtual std::optional<FunctionPtr> find(const std::string& name) = 0;
	virtual bool have(const std::string& name)const = 0;

	//通常认为VariableManager中的名称比FunctionManager中的名称多
	//因此直接遍历FunctionManager的名称,复杂度O(m log(n))
	virtual bool has_name_conflict(const VariableManager& vm)const = 0;
	virtual bool has_name_conflict(const FunctionManager& fm)const = 0;

	const std::string& name()const;
private:
	virtual ScriptPackage call(std::size_t index, const ScriptPackage& data) = 0;

	const std::string name_;
};

//最基础的FunctionManager,主要在调试时使用
class BasicFunctionManager :public FunctionManager {
public:
	BasicFunctionManager(const std::string& name);
	~BasicFunctionManager() = default;

	std::optional<FunctionPtr> find(const std::string& name)override;
	bool have(const std::string& name)const override;

	bool has_name_conflict(const VariableManager& vm)const;
	bool has_name_conflict(const FunctionManager& fm)const;

private:
	ScriptPackage call(std::size_t index, const ScriptPackage& data)override;

	IndexedMap<std::string, Function> function_container_;
};