#pragma once

#include "variable_manager.h"
#include "function_manager.h"

//�����������,���ڹ������VariableManager,FunctionManager�еĶ���
//�ṩ������,������,�������Ƿ���ڵĲ�ѯ
//�������ṩ�����ĳ�ָ��(ConstDataPtr)
// 
//��ģ����ṩֻ������,��˱�֤�̰߳�ȫ
// 
//������Ƶ�Ŀ�����ڶ��̳߳����·ֵ�VariableManager,FunctionManager�ķ���ѹ��
//���ṩ��ȫ�����Ʋ��ҷ�ʽ,����ʵ�ʵ���Դ
//�Լ�Ϊĳ��ģ���ṩ�ɶ��Ƶ�������,Ϊģ��ű�������,������,�����������ṩ������
//ʹѡ�����������,����ģ������Դ�Ĳ�������Ҫ����ģ��ִ��
//������ģ������ѡ��,���ӱ���ģ��Ŀɸ�����
class ScopeVisitor {
public:
	struct ScopeNotFound {
		std::vector<std::string> variable_scope;
		std::vector<std::string> function_scope;
	};
	//������������
	//ע��,����û�����ⷵ��std::nullopt,�����ⷵ��������Ϣ
	std::optional<ScopeNotFound> init_sub_scope(
		const std::vector<std::string>& vm_list,
		const std::vector<std::string>& fm_list,
		ScopeVisitor& sub_scope
	)const;

	void get_scope_list(
		std::vector<std::string>& vm_list,
		std::vector<std::string>& fm_list
	)const;
private:
	IndexedMap<std::string, const VariableManager*> vm_ptr_container_;
	IndexedMap<std::string, const FunctionManager*> fm_ptr_container_;
};