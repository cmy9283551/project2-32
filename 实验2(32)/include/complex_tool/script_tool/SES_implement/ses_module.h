#pragma once

#include <string>

#include "tool/container.hpp"
#include "complex_tool/script_tool/scope_visitor.h"
#include "ses_code.h"

namespace ses {
	struct ModuleConfig;

	class Function {
	public:
	private:
		std::string name_;
	};

	//����ģ��,��Ӧһ��SESModule�ļ��е�����
	//�洢module��ʵ��
	class Module {
	public:
		using StructProxy = VariableManager::StructProxy;
		using StructTemplate = VariableManager::StructTemplate;
		using StructTemplateContainer = VariableManager::StructTemplateContainer;
		using ScopeNotFound = ScopeVisitor::ScopeNotFound;
		using ScopeVector = ScopeVisitor::ScopeVector;
		//ע��:�˴���function_containerʹ����std::move
		Module(
			const std::string& name,
			const std::string& struct_data,
			IndexedMap<std::string, Function>& function_container
		);

		const std::string& name()const;

		std::optional<std::size_t> find_function(const std::string& identifier)const;
		std::optional<std::size_t> find_type(const std::string& identifier)const;

		//���ڼ��ű��������Ƿ������ģ���������
		//��ȱ��������,�᷵��ȱ����Ϣ
		std::optional<ScopeNotFound> check_scope(
			const ScopeVisitor& scope
		)const;
	private:
		std::string name_;
		IndexedMap<std::string, Function> function_container_;
		StructTemplateContainer struct_template_container_;
		std::shared_ptr<ModuleConfig> module_config_;
	};



	class ModuleVisitor;

	//�ṩ��ģ�������(����,�ṹ��)�ķ���
	//ʵ�ʴ���ģ�����
	//�ýṹֻ���ɶ�Ӧ����������,���ڱ��������Ķ�
	//�����������ṩֻ������,��˱�֤�̰߳�ȫ
	//������ɲ�ͬ��ģ��ֱ𴴽�ʵ��,ʹ�ò�ͬģ��Ľű������໥����
	class ModuleManager {
	public:
		class FunctionPtr {
			friend ModuleManager;
		public:
			FunctionPtr(std::size_t module_index, std::size_t pointer);
		private:
			std::size_t module_index_, pointer_;
		};
		class TypePtr {
			friend ModuleManager;
		public:
			TypePtr(std::size_t module_index, std::size_t pointer);
		private:
			std::size_t module_index_, pointer_;
		};

		std::optional<std::vector<std::string>> init_sub_visitor(
			std::vector<std::string>& init_vector,
			ModuleVisitor& sub_visitor
		)const;
	private:
		IndexedMap<std::string, Module> modules_;
	};

	//��ģ�������,ר������Ϊ�ű��ṩ��ѯ����
	//Ϊ�����ű��ֱ𴴽�,ʹ��ͬ�ű��ܰ�����ͬ��ϵ�ģ��
	//��SESModuleManager�Ĺ�ϵ������VariableManager/FunctionManager��ScopeVisitor
	class ModuleVisitor {
		friend ModuleManager;
	public:
		using FunctionPtr = ModuleManager::FunctionPtr;
		using TypePtr = ModuleManager::TypePtr;
		using ScopeNotFound = ScopeVisitor::ScopeNotFound;

		void get_module_vector(std::vector<std::string>& module_vector)const;

		std::optional<std::pair<FunctionPtr, std::string>> find_function(
			const std::string& identifier
		)const;
		std::optional<std::pair<TypePtr, std::string>> find_type(
			const std::string& identifier
		)const;

		std::optional<std::vector<std::string>> init_sub_visitor(
			std::vector<std::string>& init_vector,
			ModuleVisitor& sub_visitor
		)const;

		struct InvalidModule {
			std::vector<std::pair<std::string, ScopeNotFound>> invalid_vector;
		};
		std::optional<InvalidModule> check_scope(
			const ScopeVisitor& scope
		)const;
		//�����Ƴ���������ڴ����������ģ��
		void remove(const std::vector<std::string>& remove_vector);
	private:
		IndexedMap<std::string, const Module*> modules_;
	};
}