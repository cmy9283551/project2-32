#pragma once

#include <functional>

#include "tool/container.hpp"
#include "tool/script_tool/script_data.h"

class VariableManager;

//Ϊ�ű��ṩ�ڲ������ĵ����ýӿ�
//FunctionManagerΪ������,�����Ժ�֧�ֶ��̵߳ȳ���
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

	//ͨ����ΪVariableManager�е����Ʊ�FunctionManager�е����ƶ�
	//���ֱ�ӱ���FunctionManager������,���Ӷ�O(m log(n))
	virtual bool has_name_conflict(const VariableManager& vm)const = 0;
	virtual bool has_name_conflict(const FunctionManager& fm)const = 0;

	const std::string& name()const;
private:
	virtual ScriptPackage call(std::size_t index, const ScriptPackage& data) = 0;

	const std::string name_;
};

//�������FunctionManager,��Ҫ�ڵ���ʱʹ��
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