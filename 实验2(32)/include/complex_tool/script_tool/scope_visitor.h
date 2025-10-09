#pragma once

#include "variable_manager.h"
#include "function_manager.h"

#include <set>

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
	ScopeVisitor() = default;
	ScopeVisitor(
		const std::vector<const VariableManager*>& vm_ptr_list,
		const std::vector<const FunctionManager*>& fm_ptr_list
	);
	//������ScopeListһ��,�����г���Ϊ��ʾ
	struct ScopeNotFound {
		std::vector<std::string> variable_scope;
		std::vector<std::string> function_scope;

		//֧�ִ�����Ϣ����
		void operator+=(const ScopeNotFound& that);
	};
	struct ScopeVector {
		std::vector<std::string> variable_scope;
		std::vector<std::string> function_scope;
	};
	//������������
	//ע��,����û�����ⷵ��std::nullopt,�����ⷵ��������Ϣ
	std::optional<ScopeNotFound> init_sub_scope(
		const std::vector<std::string>& vm_vector,
		const std::vector<std::string>& fm_vector,
		ScopeVisitor& sub_scope
	)const;
	std::optional<ScopeNotFound> init_sub_scope(
		const ScopeVector& scope_vector,
		ScopeVisitor& sub_scope
	)const;

	void get_scope_vector(
		std::vector<std::string>& vm_vector,
		std::vector<std::string>& fm_vector
	)const;
	void get_scope_vector(ScopeVector& scope_vector)const;

	std::optional<const VariableManager*> find_vm(
		const std::string& name
	)const;
	std::optional<const FunctionManager*> find_fm(
		const std::string& name
	)const;

	bool insert_vm(const VariableManager* vm_ptr);
	bool insert_fm(const FunctionManager* fm_ptr);

	enum class IdentifierType {
		Variable,
		TypeName,
		Function,
		Null
	};
	std::expected<IdentifierType, std::string> identify(const std::string& name)const;

	bool copy(const ScopeVisitor& that);
private:
	//����¼����������ָ��,������ָ�����Ч��,���Ƴ�ͻ�ļ��
	bool check_new_scope(const VariableManager* vm_ptr);
	//����¼����������ָ��,������ָ�����Ч��,���Ƴ�ͻ�ļ��
	bool check_new_scope(const FunctionManager* fm_ptr);
	bool check_name_conflict(const std::vector<std::string>& name_vector);

	//���ڼ�����Ƴ�ͻ
	std::set<std::string> name_space_;

	IndexedMap<std::string, const VariableManager*> vm_ptr_container_;
	IndexedMap<std::string, const FunctionManager*> fm_ptr_container_;
};