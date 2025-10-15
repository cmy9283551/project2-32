#pragma once

#include "tool/container.hpp"

#include "tool/script_tool/script_data.h"

class StructTemplateContainer;
class StructProxy;

class StructTemplate {
public:
	StructTemplate(
		const StructTemplateContainer& struct_template_container,
		const std::string& name,
		std::size_t size = 0
	);
	std::size_t size()const;

	struct MemberInfo {
		std::size_t type_code;
		std::size_t offset;
	};
	//->type_code
	std::optional<std::size_t> get_member_type_code(const std::string& var_name)const;
	//->offset
	std::optional<std::size_t> get_offset(const std::string& var_name)const;
	//用stc指针保证只有该st所在的stc能为其添加成员
	bool declare_variable(
		const StructTemplateContainer* struct_template_container,
		std::size_t type_code, const std::string& var_name
	);
	const std::string& name()const;
	const IndexedMap<std::string, MemberInfo>& members()const;
	bool is_equal(const StructTemplate& that)const;

	friend std::ostream& operator<<(std::ostream& os, const StructTemplate& st);
private:
	//单独存储结构体大小,提高代码可读性,避免用offset计算是为了避免考虑边界情况
	std::size_t size_;
	std::string name_;
	const StructTemplateContainer* struct_template_container_ = nullptr;
	IndexedMap<std::string, MemberInfo> member_container_;
};

//struct_data解析规则:
//typename:typename valuename,typename valuename;
//示例:
//TypeA:int data0,float data1;
//TypeB:string str,TypeA type_a;
class StructTemplateContainer {
public:
	struct StructInfo {
		std::string name;
		std::vector<std::pair<std::string, std::string>> members;
	};
	StructTemplateContainer(const std::vector<StructInfo>& type_info);
	StructTemplateContainer(
		const std::string& struct_data = std::string()
	);

	struct CopyErrorMessage {
		std::vector<std::string> error_message;
		void operator+=(CopyErrorMessage& that);
	};

	std::optional<CopyErrorMessage> copy_all_relative_type(
		std::size_t type_code, StructTemplateContainer& that
	)const;
	std::optional<std::size_t> find(const std::string& type_name)const;
	const StructTemplate& find(std::size_t type_code)const;
	const IndexedMap<std::string, StructTemplate>& all_types()const;

	friend std::ostream& operator<<(std::ostream& os, const StructTemplateContainer& stc);
	static std::size_t basic_type_count();
	static std::vector<StructInfo> parse(const std::string& struct_data);
private:
	void initialize(const std::vector<StructInfo>& type_info);

	static const std::size_t basic_type_count_;
	IndexedMap<std::string, StructTemplate> struct_template_container_;
};

//可以通过该结构创建类型
class StructProxy {
public:
	using CopyErrorMessage = StructTemplateContainer::CopyErrorMessage;

	StructProxy() = default;
	StructProxy(
		std::size_t type_code,
		const StructTemplateContainer& struct_template_container
	);

	std::optional<std::size_t> get_offset(const std::string& var_name)const;
	std::optional<StructProxy> get_member(const std::string& var_name)const;
	const std::string& name()const;
	std::optional<CopyErrorMessage> copy_all_relative_type(
		StructTemplateContainer& that
	)const;
	bool is_equal(const StructProxy& that)const;
	bool is_equal(const StructTemplate& that)const;
private:
	std::size_t type_code_ = 0;
	const StructTemplateContainer* struct_template_container_ = nullptr;
};