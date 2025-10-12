#pragma once

#include "ses_module.h"
#include "complex_tool/script_tool/scope_visitor.h"

//���ļ��洢����Ľű���ʽ

namespace ses {
	//��Žű�������Ϣ
	struct ScriptConfig {
		//��������Ĳ���
		ScriptParameter input, output;

		ModuleVisitor module_visitor;
		ScopeVisitor scope_visitor;

		//�ڴ���ʱ����ʹ�õ������Զ�����,
		VariableManager::StructTemplateContainer script_stc;

		friend std::ostream& operator<<(std::ostream& os, const ScriptConfig& config);
	};

	//�������ɵĽű���,���ڲ��Ҳ����ýű�
	class ScriptLibrary {
	public:
	private:
	};

	//�������ɵĿ�ִ�нű�,����ָ�,�ڴ��
	//�ͱ���������ָ��(�Ե���ͳһ����),����������ָ��(�Ե���ͳһ����)
	class ScriptInstance {
	public:

	};
}