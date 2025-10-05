#pragma once

#include <functional>

#include "tool/container.hpp"
#include "tool/script_tool/script_data.h"

//为脚本提供内部函数的调用用接口
//FunctionManager为抽象类,便于以后支持多线程等场景
class FunctionManager {
public:
	virtual ~FunctionManager() = default;
	typedef std::function<ScriptPackage(const ScriptPackage&)> Function;
	class FunctionProxy {
	public:
		FunctionProxy(std::size_t index, FunctionManager& function_manager);
		ScriptPackage call(const ScriptPackage& data);
	private:
		std::size_t index_;
		FunctionManager& function_manager_;
	};
	virtual std::optional<FunctionProxy> find(const std::string& name) = 0;
private:
	virtual ScriptPackage call(std::size_t index, const ScriptPackage& data) = 0;
};

//最基础的FunctionManager,主要在调试时使用
class BasicFunctionManager :public FunctionManager {
public:
	std::optional<FunctionProxy> find(const std::string& name)override;
private:
	ScriptPackage call(std::size_t index, const ScriptPackage& data)override;

	IndexedMap<std::string, Function> container_;
};