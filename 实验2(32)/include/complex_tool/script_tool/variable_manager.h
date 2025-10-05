#pragma once

#include "tool/container.hpp"
#include "tool/script_tool/script_debug_tool.h"
#include "tool/script_tool/script_data.h"

#define VARIABLE_MANAGER_LOG
//����VariableManager����־���

//VariableManager�ṩ�ű������Ĵ������
//��Ϊȫ�ֱ������ṩ��,�ò����ڳ�ʼ��֮�������ӻ�ɾ������
//����Ϊ������,�����Ժ�֧�ֶ��̵߳ȳ���
class VariableManager {
public:
	virtual ~VariableManager() = default;

	//���ڶԻ������͵�ʶ��
	enum class BasicDataType {
		Int = 0,//size = sizeof(ScriptInt)
		Float = 1,//size = sizeof(ScriptFloat)
		Char = 2,//size = size(ScriptChar)
		String = 3,//size = sizeof(std::size_t)
		VectorInt = 4,//size = sizeof(std::size_t)
		VectorFloat = 5,//size = sizeof(std::size_t)
		Package = 6,//size = sizeof(std::size_t)
		Struct = 7
	};

	//���ݴ���,ָ��heap(������һ��std::vector<unsigned char>)�����ݵĵ�һλ��λ��
	//�ṩ����ת��Ϊc++���͵ķ���,��������ʹ��
	//ע��:
	//����String,VectorInt,VectorFloat,Package,��ָ��heap��������һ��ָ��,����ʵ���ݴ����ڱ�
	//����Struct,�������ɳ�Ա���������ݹ���
	//���ʳ�Ա����Ҫͨ����Ӧ��StructTemplate�ҳ���Ա����ƫ����
	class DataPtr {
	public:
		DataPtr(
			std::size_t pointer, std::size_t type_code, VariableManager& variable_manager
		);

		std::optional<ScriptInt> int_data()const;
		bool modify_int(ScriptInt data);
		std::optional<ScriptFloat> float_data()const;
		bool modify_float(ScriptFloat data);
		std::optional<ScriptChar> char_data()const;
		bool modify_char(ScriptChar data);
		std::optional<ScriptString*> string_data();
		std::optional<ScriptVectorInt*> vector_int_data();
		std::optional<ScriptVectorFloat*> vector_float_data();
		//Package����
		std::optional<DataPtr> create_member(
			const std::string& type_name, const std::string& var_name
		);
		//Package,Struct����:Ѱ�ҳ�Ա����
		std::optional<DataPtr> find(const std::string& var_name)const;
		BasicDataType type_code_translate()const;
	private:
		//�򻯺���,�洢���������ͻ�ʹ��
		std::size_t get_size_t()const;

		std::size_t pointer_, type_code_;
		VariableManager* variable_manager_;
	};

	//��ָ��
	//ӵ�к�DataPtr��ͬ�ķ��ʹ���,���޷��޸�ָ���Ӧ��ֵ
	class ConstDataPtr {
	public:
		ConstDataPtr(
			std::size_t pointer, std::size_t type_code, const VariableManager& variable_manager
		);

		std::optional<ScriptInt> int_data()const;
		std::optional<ScriptFloat> float_data()const;
		std::optional<ScriptChar> char_data()const;
		std::optional<const ScriptString*> string_data()const;
		std::optional<const ScriptVectorInt*> vector_int_data()const;
		std::optional<const ScriptVectorFloat*> vector_float_data()const;

		//Package,Struct����:Ѱ�ҳ�Ա����
		std::optional<ConstDataPtr> find(const std::string& var_name)const;
		BasicDataType type_code_translate()const;
	private:
		std::size_t get_size_t()const;

		std::size_t pointer_, type_code_;
		const VariableManager* variable_manager_;
	};

	class StructTemplateContainer;

	class StructTemplate {
	public:
		StructTemplate(
			const StructTemplateContainer& struct_template_container,
			const std::string& name,
			std::size_t size = 0
		);
		std::size_t size()const;

		//->type_code
		std::optional<std::size_t> get_member_type_code(const std::string& var_name)const;
		//->offset
		std::optional<std::size_t> get_offset(const std::string& var_name)const;
		//->offset
		std::size_t declare_variable(std::size_t type_code, const std::string& var_name);
		const std::string& name()const;
		const IndexedMap<std::string, std::size_t>& members()const;

		friend std::ostream& operator<<(std::ostream& os, const StructTemplate& st);
	private:
		std::size_t size_;
		std::string name_;
		const StructTemplateContainer* struct_template_container_;
		IndexedMap<std::string, std::size_t> type_code_container_;
	};

	//struct_data��������:
	//typename:typename valuename,typename valuename;
	//ʾ��:
	//TypeA:Int data0,Float data1;
	//TypeB:String str,TypeA type_a;
	class StructTemplateContainer {
	public:
		StructTemplateContainer(
			const std::string& struct_data
		);
		std::optional<std::size_t> find(const std::string& type_name)const;
		const StructTemplate& find(std::size_t type_code)const;
		const IndexedMap<std::string, StructTemplate>& all_struct()const;

		friend std::ostream& operator<<(std::ostream& os, const StructTemplateContainer& stc);
		static std::size_t basic_type_count();
	private:
		void parse(const std::string& struct_data);
		void create_type(const std::string& struct_data, std::size_t& pointer);

		static const std::size_t basic_type_count_;
		IndexedMap<std::string, StructTemplate> struct_template_container_;
	};

	//ͨ�������ҵ���Ӧ������ָ��
	virtual std::optional<DataPtr> find(const std::string& var_name) = 0;
	//ͨ�������ҵ���Ӧ�����ĳ�ָ��
	virtual std::optional<ConstDataPtr> cfind(const std::string& var_name)const = 0;

	//���Ժ���,���ڵ���ʱ�鿴����
	//�ṩ���Խӿ�,����Ҫ��ǿ��ʵ��,������Ҫ����ʵ��

	virtual void print_struct_data()const;
	virtual void print_heap_data()const;
protected:
	//protected����˵��:
	//���Ǵ���std::size_t type_code�Ĳ�����ȫ���,��Ϊ�ñ���Ϊȷ�Ϻ���
	//(����ȫ���ڲ��������ܻ��type_code)

	//part1:�ض����Ͷѷ���

	virtual ScriptString* get_string(std::size_t ptr) = 0;
	virtual const ScriptString* get_const_string(std::size_t ptr)const = 0;

	virtual ScriptVectorInt* get_vector_int(std::size_t ptr) = 0;
	virtual const ScriptVectorInt* get_const_vector_int(std::size_t ptr)const = 0;

	virtual ScriptVectorFloat* get_vector_float(std::size_t ptr) = 0;
	virtual const ScriptVectorFloat* get_const_vector_float(std::size_t ptr)const = 0;

	//part2:������Ҫbyte_heap�ⴢ�������
	virtual std::size_t gen_string() = 0;
	virtual std::size_t gen_vector_int() = 0;
	virtual std::size_t gen_vector_float() = 0;
	virtual std::size_t gen_package() = 0;

	struct InternalPtr {
		std::size_t pointer, type_code;
	};

	//part3:byte_heap����

	//��ӱ���,�ϸ���˵,���б�����Ӧ���ڹ��캯��������
	//�����ǵ���ʼ��˳��͸��Ӷȵ�����,����������ͨ�����ض�����ϵĽӿ����ض�����������
	//��ӹ���:
	//������ȫ��byte_heap���������,�ú��������ؿռ�ָ��,ֵ�ǲ�ȷ����
	//������Ҫ����byte_heap��ռ䴢�������,�ú���Ӧ���������ռ���������
	//����byte_heap�д����Ӧָ��
	//ע��:��������������ظ��ı���,�����ᵼ���ڴ�й¶,�����������std::nullopt
	virtual std::optional<InternalPtr> declare_variable(
		std::size_t type_code, const std::string& var_name
	) = 0;
	//����һ�������Ŀռ�,������ָ��
	virtual InternalPtr gen_variable_space(std::size_t type_code) = 0;
	//��ʼ��struct���͵ı���,��Ҫ��Ϊ��ʹstruct��byte_heap�ⴢ������͵õ�ʵ�ʿռ�
	virtual void init_struct(
		std::size_t pointer, std::size_t type_code
	) = 0;
	//��heap��ȡ�ó�ʼ����
	virtual void get_data_from_heap(
		std::size_t ptr, std::size_t size, std::vector<unsigned char>& data
	)const = 0;
	//�޸�heap�е�����
	virtual void modify_data(
		std::size_t ptr, const std::vector<unsigned char>& data
	) = 0;

	//part4:package����

	//<name,<pointer,type_code>>
	typedef IndexedMap<std::string, InternalPtr> Package;

	//package�ĳ�Ա��������������,��Ҫheap����������������
	virtual std::optional<InternalPtr> package_get_member_ptr(
		std::size_t package_index, std::size_t member_index
	)const = 0;
	//package�ĳ�Ա��������������,��Ҫheap����������������
	virtual std::optional<InternalPtr> package_get_member_ptr(
		std::size_t package_index, const std::string& var_name
	)const = 0;
	//��heap���package��Ա�б�����������Ķ����ָ��
	virtual void package_push_member_ptr(
		std::size_t package_index, const std::string& var_name, InternalPtr member_ptr
	) = 0;

	//part5:struct����

	//����������������StructTemplateContainer(STC)ʵ������,��������º����������

	//ͨ��STC��������Ϊname�����type_code
	virtual std::optional<std::size_t> get_type_code(const std::string& type_name)const = 0;
	//ͨ��STC���������heap��ռ�õĴ�С(�����ܴ�С)
	virtual std::size_t get_type_size(std::size_t type_code)const = 0;
	//ͨ��STC��ó�Ա������heap�е�ָ��
	virtual std::optional<InternalPtr> struct_get_member_ptr(
		std::size_t pointer, std::size_t type_code, const std::string& var_name
	)const = 0;
};

//�������VarialbeManager,��Ҫ�ڵ���ʱʹ��
//�ýṹҲ��Ϊһ��ԭ��,ָ�������๹��
//��ȻҲ������Ϊ���̲߳��ֵ�һ���ṹ
//�ýṹ���̰߳�ȫ���
class BasicVariableManager :public VariableManager {
public:
	BasicVariableManager(const std::string& struct_data);
	virtual ~BasicVariableManager() = default;

	std::optional<DataPtr> find(const std::string& var_name) override;
	std::optional<ConstDataPtr> cfind(const std::string& var_name) const override;

	std::optional<DataPtr> create_variable(const std::string& type_name, const std::string& var_name);

	void print_struct_data() const override;
	void print_heap_data() const override;
private:

	ScriptString* get_string(std::size_t ptr) override;
	const ScriptString* get_const_string(std::size_t ptr) const override;

	ScriptVectorInt* get_vector_int(std::size_t ptr) override;
	const ScriptVectorInt* get_const_vector_int(std::size_t ptr) const override;

	ScriptVectorFloat* get_vector_float(std::size_t ptr) override;
	const ScriptVectorFloat* get_const_vector_float(std::size_t ptr) const override;

	std::size_t gen_string() override;
	std::size_t gen_vector_int() override;
	std::size_t gen_vector_float() override;
	std::size_t gen_package() override;

	std::optional<InternalPtr> declare_variable(std::size_t type_code, const std::string& var_name) override;
	InternalPtr gen_variable_space(std::size_t type_code) override;
	void init_struct(std::size_t pointer, std::size_t type_code) override;
	void get_data_from_heap(
		std::size_t ptr, std::size_t size, std::vector<unsigned char>& data
	) const override;
	void modify_data(std::size_t ptr, const std::vector<unsigned char>& data) override;

	std::optional<InternalPtr> package_get_member_ptr(
		std::size_t package_index, std::size_t member_index
	) const override;
	std::optional<InternalPtr> package_get_member_ptr(
		std::size_t package_index, const std::string& var_name
	) const override;
	void package_push_member_ptr(
		std::size_t package_index, const std::string& var_name, InternalPtr member_ptr
	) override;

	std::optional<std::size_t> get_type_code(const std::string& type_name) const override;
	std::size_t get_type_size(std::size_t type_code) const override;
	std::optional<InternalPtr> struct_get_member_ptr(
		std::size_t pointer, std::size_t type_code, const std::string& var_name
	) const override;

	std::vector<unsigned char> byte_heap_;
	IndexedMap<std::string, InternalPtr> name_space_;

	std::vector<ScriptString> string_heap_;
	std::vector<ScriptVectorInt> vector_int_heap_;
	std::vector<ScriptVectorFloat> vector_float_heap_;
	std::vector<Package> package_heap_;

	StructTemplateContainer struct_template_container_;
};