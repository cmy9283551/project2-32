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

//����ģ��,��Ӧһ��SESModule�ļ��е�����
//�洢module��ʵ��
class SESModule {
public:
	using StructTemplate = VariableManager::StructTemplate;
	using StructTemplateContainer = VariableManager::StructTemplateContainer;
	//ע��:�˴���function_containerʹ����std::move
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

//�ṩ��ģ�������(����,�ṹ��)�ķ���
//ʵ�ʴ���ģ�����
//�ýṹֻ���ɶ�Ӧ����������,���ڱ��������Ķ�
//�����������ṩֻ������,��˱�֤�̰߳�ȫ
//������ɲ�ͬ��ģ��ֱ𴴽�ʵ��,ʹ�ò�ͬģ��Ľű������໥����
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

//��ģ�������,ר������Ϊ�ű��ṩ��ѯ����
//Ϊ�����ű��ֱ𴴽�,ʹ��ͬ�ű��ܰ�����ͬ��ϵ�ģ��
//��SESModuleManager�Ĺ�ϵ������VariableManager/FunctionManager��ScopeVisitor
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