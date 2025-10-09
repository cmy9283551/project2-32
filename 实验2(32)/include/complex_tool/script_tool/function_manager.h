#pragma once

#include <functional>

#include "tool/container.hpp"
#include "tool/script_tool/script_data.h"

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

	//���ȫ������,�������к�����,���ڼ����ֵ�ռ�
	virtual void get_name_vector(std::vector<std::string>& name_vector)const = 0;

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

	void get_name_vector(std::vector<std::string>& name_vector)const override;
private:
	ScriptPackage call(std::size_t index, const ScriptPackage& data)override;

	IndexedMap<std::string, Function> function_container_;
};