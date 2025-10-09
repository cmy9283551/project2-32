#pragma once

#include <functional>

#include "tool/container.hpp"
#include "tool/script_tool/script_data.h"

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

	//获得全部名称,包含所有函数名,用于检查名值空间
	virtual void get_name_vector(std::vector<std::string>& name_vector)const = 0;

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

	void get_name_vector(std::vector<std::string>& name_vector)const override;
private:
	ScriptPackage call(std::size_t index, const ScriptPackage& data)override;

	IndexedMap<std::string, Function> function_container_;
};