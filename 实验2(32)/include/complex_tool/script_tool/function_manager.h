#pragma once

#include <functional>

#include "tool/container.hpp"
#include "tool/script_tool/script_data.h"

//Ϊ�ű��ṩ�ڲ������ĵ����ýӿ�
//FunctionManagerΪ������,�����Ժ�֧�ֶ��̵߳ȳ���
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

//�������FunctionManager,��Ҫ�ڵ���ʱʹ��
class BasicFunctionManager :public FunctionManager {
public:
	std::optional<FunctionProxy> find(const std::string& name)override;
private:
	ScriptPackage call(std::size_t index, const ScriptPackage& data)override;

	IndexedMap<std::string, Function> container_;
};