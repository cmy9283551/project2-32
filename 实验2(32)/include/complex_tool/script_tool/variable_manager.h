#pragma once

#include "tool/container.hpp"
#include "tool/script_tool/script_debug_tool.h"
#include "tool/script_tool/script_data.h"

#define VARIABLE_MANAGER_LOG
//控制VariableManager的日志输出

//VariableManager提供脚本变量的储存服务
//作为全局变量的提供者,该部分在初始化之后不能增加或删除变量
//此类为抽象类,便于以后支持多线程等场景
class VariableManager {
public:
	virtual ~VariableManager() = default;

	//用于对基本类型的识别
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

	//数据代理,指向heap(可视作一个std::vector<unsigned char>)中数据的第一位的位置
	//提供将其转化为c++类型的服务,能在类外使用
	//注意:
	//对于String,VectorInt,VectorFloat,Package,其指向heap的数据是一个指针,其真实数据储存在别处
	//对于Struct,其数据由成员变量的数据构成
	//访问成员变量要通过对应的StructTemplate找出成员变量偏移量
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
		//Package功能
		std::optional<DataPtr> create_member(
			const std::string& type_name, const std::string& var_name
		);
		//Package,Struct功能:寻找成员变量
		std::optional<DataPtr> find(const std::string& var_name)const;
		BasicDataType type_code_translate()const;
	private:
		//简化函数,存储引索的类型会使用
		std::size_t get_size_t()const;

		std::size_t pointer_, type_code_;
		VariableManager* variable_manager_;
	};

	//常指针
	//拥有和DataPtr相同的访问功能,但无法修改指针对应的值
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

		//Package,Struct功能:寻找成员变量
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

	//struct_data解析规则:
	//typename:typename valuename,typename valuename;
	//示例:
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

	//通过名称找到对应变量的指针
	virtual std::optional<DataPtr> find(const std::string& var_name) = 0;
	//通过名称找到对应变量的常指针
	virtual std::optional<ConstDataPtr> cfind(const std::string& var_name)const = 0;

	//调试函数,用于调试时查看数据
	//提供调试接口,但不要求强制实现,仅让需要的类实现

	virtual void print_struct_data()const;
	virtual void print_heap_data()const;
protected:
	//protected函数说明:
	//凡是传入std::size_t type_code的不做安全检查,认为该变量为确认后传入
	//(仅安全的内部函数才能获得type_code)

	//part1:特定类型堆访问

	virtual ScriptString* get_string(std::size_t ptr) = 0;
	virtual const ScriptString* get_const_string(std::size_t ptr)const = 0;

	virtual ScriptVectorInt* get_vector_int(std::size_t ptr) = 0;
	virtual const ScriptVectorInt* get_const_vector_int(std::size_t ptr)const = 0;

	virtual ScriptVectorFloat* get_vector_float(std::size_t ptr) = 0;
	virtual const ScriptVectorFloat* get_const_vector_float(std::size_t ptr)const = 0;

	//part2:声明需要byte_heap外储存的类型
	virtual std::size_t gen_string() = 0;
	virtual std::size_t gen_vector_int() = 0;
	virtual std::size_t gen_vector_float() = 0;
	virtual std::size_t gen_package() = 0;

	struct InternalPtr {
		std::size_t pointer, type_code;
	};

	//part3:byte_heap访问

	//添加变量,严格来说,所有变量都应当在构造函数中声明
	//但考虑到初始化顺序和复杂度的问题,允许派生类通过与特定类耦合的接口让特定类声明变量
	//添加规则:
	//对于完全在byte_heap储存的类型,该函数仅返回空间指针,值是不确定的
	//对于需要借助byte_heap外空间储存的类型,该函数应当在其他空间声明类型
	//并在byte_heap中储存对应指针
	//注意:不允许添加名称重复的变量,这样会导致内存泄露,这种情况返回std::nullopt
	virtual std::optional<InternalPtr> declare_variable(
		std::size_t type_code, const std::string& var_name
	) = 0;
	//分配一个变量的空间,并返回指针
	virtual InternalPtr gen_variable_space(std::size_t type_code) = 0;
	//初始化struct类型的变量,主要是为了使struct中byte_heap外储存的类型得到实际空间
	virtual void init_struct(
		std::size_t pointer, std::size_t type_code
	) = 0;
	//从heap中取得初始数据
	virtual void get_data_from_heap(
		std::size_t ptr, std::size_t size, std::vector<unsigned char>& data
	)const = 0;
	//修改heap中的数据
	virtual void modify_data(
		std::size_t ptr, const std::vector<unsigned char>& data
	) = 0;

	//part4:package操作

	//<name,<pointer,type_code>>
	typedef IndexedMap<std::string, InternalPtr> Package;

	//package的成员变量不连续储存,需要heap外数据来辅助查找
	virtual std::optional<InternalPtr> package_get_member_ptr(
		std::size_t package_index, std::size_t member_index
	)const = 0;
	//package的成员变量不连续储存,需要heap外数据来辅助查找
	virtual std::optional<InternalPtr> package_get_member_ptr(
		std::size_t package_index, const std::string& var_name
	)const = 0;
	//向heap外的package成员列表加入新声明的对象的指针
	virtual void package_push_member_ptr(
		std::size_t package_index, const std::string& var_name, InternalPtr member_ptr
	) = 0;

	//part5:struct操作

	//不允许其他函数对StructTemplateContainer(STC)实例操作,因此由以下函数替代操作

	//通过STC查找名称为name的类的type_code
	virtual std::optional<std::size_t> get_type_code(const std::string& type_name)const = 0;
	//通过STC获得类型在heap中占用的大小(并非总大小)
	virtual std::size_t get_type_size(std::size_t type_code)const = 0;
	//通过STC获得成员变量在heap中的指针
	virtual std::optional<InternalPtr> struct_get_member_ptr(
		std::size_t pointer, std::size_t type_code, const std::string& var_name
	)const = 0;
};

//最基础的VarialbeManager,主要在调试时使用
//该结构也作为一个原型,指导后续类构建
//当然也可以作为单线程部分的一个结构
//该结构无线程安全设计
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