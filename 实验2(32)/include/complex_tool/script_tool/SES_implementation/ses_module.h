#pragma once

#include <string>

#include "tool/container.hpp"
#include "complex_tool/script_tool/scope_visitor.h"
#include "ses_midcode.h"

namespace ses {
	//���ģ��������Ϣ
	
	struct ModuleConfig;

	class ModuleFunction {
	public:
	private:
		std::string name_;
	};

	//����ģ��,��Ӧһ��SESModule�ļ��е�����
	//�洢module��ʵ��
	class Module {
	public:
		using ScopeNotFound = ScopeVisitor::ScopeNotFound;
		using ScopeVector = ScopeVisitor::ScopeVector;
		//ע��:�˴���function_containerʹ����std::move
		Module(
			const std::string& name,
			std::unique_ptr<StructTemplateContainer> struct_template_container,
			IndexedMap<std::string, ModuleFunction>& function_container,
			std::unique_ptr<ModuleConfig> module_config
		);

		const std::string& name()const;

		std::optional<std::size_t> find_function(const std::string& identifier)const;
		std::optional<StructProxy> find_type(const std::string& identifier)const;

		//���ڼ��ű��������Ƿ������ģ���������
		//��ȱ��������,�᷵��ȱ����Ϣ
		std::optional<ScopeNotFound> check_scope(
			const ScopeVisitor& scope
		)const;

		enum class IdentifierType {
			Function,
			TypeName
		};
		std::optional<IdentifierType> identify(const std::string& identifier)const;
	private:
		std::string name_;
		std::unique_ptr<StructTemplateContainer> struct_template_container_ = nullptr;
		IndexedMap<std::string, ModuleFunction> function_container_;
		std::unique_ptr<ModuleConfig> module_config_ = nullptr;
	};

	class ModuleVisitor;

	//�ṩ��ģ�������(����,�ṹ��)�ķ���
	//ʵ�ʴ���ģ�����
	//�ýṹֻ���ɶ�Ӧ����������,���ڱ��������Ķ�
	//�����������ṩֻ������,��˱�֤�̰߳�ȫ
	//������ɲ�ͬ��ģ��ֱ𴴽�ʵ��,ʹ�ò�ͬģ��Ľű������໥����
	// 
	//������Ψһ�Եı�֤:
	//����ʱ����Ƿ����ͬ��ģ��,�������򸲸�
	//��˱�֤ModuleManager�в������ͬ��ģ��
	//����ʱ���ģ���к��������������Ƿ����������Ƴ�ͻ,����ͻ�����ʧ��
	//��˱�֤ModuleManager�в������ͬ����������������	
	//����һ��ģ���ģ��ֻ��ʹ�ø�ģ���ṩ��������
	//��һ��ģ��ֻ�ܴ���һ��ModuleManager
	//��˱�֤��������������ͻ
	//����Ϊ����ģ��ʱ����ģ���г�Ա�����Ƿ�Ϸ�(���Ƿ��������������Ƴ�ͻ)
	//��˱�֤ģ���г�Ա���Ʋ����������������Ƴ�ͻ
	class ModuleManager {
	public:
		class FunctionPtr {
		public:
			FunctionPtr(std::size_t module_index, std::size_t pointer);
		private:
			std::size_t module_index_, pointer_;
		};

		std::optional<std::vector<std::string>> init_sub_visitor(
			std::vector<std::string>& init_vector,
			ModuleVisitor& sub_visitor
		)const;

		const std::string& name()const;
	private:
		//����moduleֻ����ModuleManager����Ԫ�����,���ֶ�������ɵ���ʹ��
		//����ʱ����Ƿ����ͬ��ģ��,�������򸲸�
		//��˱�֤ModuleManager�в������ͬ��ģ��
		//����ʱ���ģ���к��������������Ƿ����������Ƴ�ͻ,����ͻ�����ʧ��
		//��˱�֤ModuleManager�в������ͬ����������������
		bool insert_module(
			const std::string& name,
			std::unique_ptr<StructTemplateContainer> struct_template_container,
			IndexedMap<std::string, ModuleFunction>& function_container,
			std::unique_ptr<ModuleConfig> module_config
		);

		std::string name_;
		//���ڼ��ģ�������Ƿ�������Ƴ�ͻ
		//����һ��module�ڱ���ʱ������������,����������ģ��������������
		// ��˱�֤��������������ͻ
		std::set<std::string> name_space_;
		IndexedMap<std::string, Module> modules_;
	};

	//��ģ�������,ר������Ϊ�ű��ṩ��ѯ����
	//Ϊ�����ű��ֱ𴴽�,ʹ��ͬ�ű��ܰ�����ͬ��ϵ�ģ��
	//��SESModuleManager�Ĺ�ϵ������VariableManager/FunctionManager��ScopeVisitor
	//ֻ����һ��ModuleManager��ʼ��,��˱�֤������ģ������Ƴ�ͻ
	class ModuleVisitor {
		friend ModuleManager;
	public:
		using FunctionPtr = ModuleManager::FunctionPtr;
		using ScopeNotFound = ScopeVisitor::ScopeNotFound;
		using IdentifierType = Module::IdentifierType;

		void get_module_vector(std::vector<std::string>& module_vector)const;

		std::optional<FunctionPtr> find_function(
			const std::string& identifier
		)const;
		std::optional<StructProxy> find_type(
			const std::string& identifier
		)const;

		std::optional<std::vector<std::string>> init_sub_visitor(
			std::vector<std::string>& init_vector,
			ModuleVisitor& sub_visitor
		)const;

		struct InvalidModule {
			std::vector<std::pair<std::string, ScopeNotFound>> invalid_vector;
		};
		//���ڼ��ű��������Ƿ������ģ���������
		std::optional<InvalidModule> check_scope(
			const ScopeVisitor& scope
		)const;
		//�����Ƴ���������ڴ����������ģ��
		void remove(const std::vector<std::string>& remove_vector);

		std::optional<IdentifierType> identify(const std::string& identifier)const;
	private:
		IndexedMap<std::string, const Module*> modules_;
	};

	struct ModuleConfig {
		ScopeVisitor scope_visitor;
		ModuleVisitor module_visitor;
	};
}