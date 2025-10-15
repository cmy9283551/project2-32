#include "complex_tool/script_tool/type_manager.h"

StructTemplate::StructTemplate(
	const StructTemplateContainer& struct_template_container,
	const std::string& name,
	std::size_t size
) :struct_template_container_(&struct_template_container), name_(name), size_(size) {
}

std::size_t StructTemplate::size()const {
	return size_;
}

std::optional<std::size_t> StructTemplate::get_member_type_code(
	const std::string& var_name
) const {
	auto iter = member_container_.find(var_name);
	if (iter == member_container_.cend()) {
		return std::nullopt;
	}
	return iter.second().type_code;
}

std::optional<std::size_t> StructTemplate::get_offset(
	const std::string& var_name
) const {
	auto iter = member_container_.find(var_name);
	if (iter == member_container_.cend()) {
		return std::nullopt;
	}
	return iter.second().offset;
}

bool StructTemplate::declare_variable(
	const StructTemplateContainer* struct_template_container,
	std::size_t type_code, const std::string& var_name
) {
	if (struct_template_container != struct_template_container_) {
		return false;
	}
	auto iter = member_container_.find(var_name);
	if (iter != member_container_.end()) {
		return false;
	}
	member_container_.emplace(var_name, MemberInfo{ type_code, size_ });
	size_ += struct_template_container_->find(type_code).size();
	return true;
}

std::ostream& operator<<(std::ostream& os, const StructTemplate& st) {
	os << "结构体[" << st.name() << "],大小["
		<< st.size() << "]:\n";
	const auto& visitor = st.member_container_.get_visitor();
	std::size_t size = visitor.size();
	for (std::size_t i = 0; i < size; i++) {
		const auto& member = visitor[i];
		const auto& type = st.struct_template_container_->find(member.second->type_code);
		os << "\t成员变量[" << *member.first << "],类型["
			<< type.name() << "],偏移[" << member.second->offset << "]\n";
	}
	return os;
}

const std::size_t StructTemplateContainer::basic_type_count_ = 7;

StructTemplateContainer::StructTemplateContainer(
	const std::vector<StructInfo>& type_info
) {
	initialize(type_info);
}

StructTemplateContainer::StructTemplateContainer(
	const std::string& struct_data
) {
	initialize(parse(struct_data));
}

void StructTemplateContainer::CopyErrorMessage::operator+=(
	CopyErrorMessage& that
	) {
	error_message.reserve(error_message.size() + that.error_message.size());
	error_message.insert(
		error_message.end(),
		std::make_move_iterator(that.error_message.begin()),
		std::make_move_iterator(that.error_message.begin())
	);
}

std::optional<StructTemplateContainer::CopyErrorMessage>
StructTemplateContainer::copy_all_relative_type(
	std::size_t type_code, StructTemplateContainer& that
)const {
	if (&that == this) {
		return std::nullopt;
	}
	CopyErrorMessage message;
	const auto& this_type = find(type_code);
	const auto& members = this_type.members();
	std::size_t size = members.size();
	for (std::size_t i = 0; i < size; i++) {
		const std::string& type_name = struct_template_container_[members[i].type_code].name();
		auto that_have_type = that.find(type_name);
		if (that_have_type == std::nullopt) {
			auto result = copy_all_relative_type(members[i].type_code, that);
			if (result != std::nullopt) {
				message += result.value();
			}
			continue;
		}
		if (that.find(that_have_type.value()).is_equal(
			struct_template_container_[members[i].type_code]) == false) {
			message.error_message.emplace_back(type_name);
		}
	}
	if (message.error_message.size() == 0) {
		return std::nullopt;
	}
	return message;
}

std::optional<std::size_t> StructTemplateContainer::find(
	const std::string& name
) const {
	auto iter = struct_template_container_.find_serial_number(name);
	if (iter.second == false) {
		return std::nullopt;
	}
	return iter.first;
}

const StructTemplate& StructTemplateContainer::find(
	std::size_t type_code
) const {
	return struct_template_container_.find(type_code).second();
}

const IndexedMap<std::string, StructTemplate>&StructTemplateContainer::all_types(
) const {
	return struct_template_container_;
}

const std::string& StructTemplate::name()const {
	return name_;
}

const IndexedMap<std::string, StructTemplate::MemberInfo>&StructTemplate::members(
) const {
	return member_container_;
}

bool StructTemplate::is_equal(const StructTemplate& that) const {
	if (that.struct_template_container_ == struct_template_container_) {
		return that.name() == name();
	};
	if (that.size() != size()) {
		return false;
	}
	const auto& that_members = that.members();
	auto that_iter = that_members.cbegin();
	for (; that_iter != that_members.cend(); ++that_iter) {
		//确保成员变量名称相同
		auto iter = member_container_.find(that_iter.first());
		if (iter == member_container_.cend()) {
			return false;
		}
		//顺序有影响,但可以比较offset判断
		if (iter.second().offset != that_iter.second().offset) {
			return false;
		}
		//确保成员变量类型相同
		if (struct_template_container_->find(iter.second().type_code).is_equal(
			that.struct_template_container_->find(that_iter.second().type_code)
		) == false) {
			return false;
		}
	}
	return true;
}

std::ostream& operator<<(std::ostream& os, const StructTemplateContainer& stc) {
	std::size_t size = stc.struct_template_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		if (i < stc.basic_type_count()) {
			os << "类型代码["
				<< i << "]:基础类型[" << stc.struct_template_container_.find(i).second().name()
				<< "],大小["
				<< stc.struct_template_container_.find(i).second().size() << "]\n";
			continue;
		}
		os << "类型代码[" << i << "]" << stc.struct_template_container_[i];
	}
	return os;
}

std::size_t StructTemplateContainer::basic_type_count() {
	return basic_type_count_;
}

std::vector<StructTemplateContainer::StructInfo>StructTemplateContainer::parse(
	const std::string& struct_data
) {
	std::size_t length = struct_data.length(), pointer = 0;
	auto check = [](const std::string& name)->bool
		{
			auto is_character = [](char c)->bool {
				return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
				};
			auto is_number = [](char c)->bool {
				return c = '0' && c <= '9';
				};
			std::size_t length = name.length();
			if (length == 0) {
#ifdef SCRIPT_DEBUG
				SCRIPT_CERR
					<< "初始化VariableManager时,列表中存在空名称" << std::endl;
				ASSERT(false);
#endif // SCRIPT_DEBUG
				return false;
			}
			if (is_character(name[0]) == false && name[0] != '_') {
#ifdef SCRIPT_DEBUG
				SCRIPT_CERR
					<< "初始化VariableManager时,列表中存在不合法名称["
					<< name << "]" << std::endl;
				ASSERT(false);
#endif // SCRIPT_DEBUG
				return false;
			}
			for (std::size_t i = 0; i < length; i++) {
				if (is_character(
					name[i]) == false &&
					is_number(name[i]) == false &&
					name[i] != '_'
					) {
#ifdef SCRIPT_DEBUG
					SCRIPT_CERR
						<< "初始化VariableManager时,列表中存在不合法名称["
						<< name << "]" << std::endl;
					ASSERT(false);
#endif // SCRIPT_DEBUG
					return false;
				}
			}
			return true;
		};

	auto get_block = [&]()
		{
			while (struct_data[pointer] == ' ' ||
				struct_data[pointer] == '\n' ||
				struct_data[pointer] == '\t' ||
				struct_data[pointer] == '\r'
				) {
				pointer++;
				if (pointer >= length) {
					break;
				}
			}
		};

	auto create_type = [&]()->std::optional<StructInfo>
		{
			StructInfo new_type;
			get_block();
			while (struct_data[pointer] != ':' && struct_data[pointer] != ' ') {
				if (pointer >= length) {
					break;
				}
				new_type.name.push_back(struct_data[pointer]);
				pointer++;
			}
			if (check(new_type.name) == false) {
				return std::nullopt;
			}
			get_block();
			pointer++;//跳过":"
			while (struct_data[pointer] != ';' && pointer < length) {
				get_block();
				std::string type;
				while (struct_data[pointer] != ';' && struct_data[pointer] != ' ') {
					if (pointer >= length) {
						break;
					}
					type.push_back(struct_data[pointer]);
					pointer++;
				}
				if (check(type) == false) {
					return std::nullopt;
				}
				get_block();

				std::string name;
				while (
					struct_data[pointer] != ';' &&
					struct_data[pointer] != ',' &&
					struct_data[pointer] != ' '
					) {
					if (pointer >= length) {
						break;
					}
					name.push_back(struct_data[pointer]);
					pointer++;
				}
				if (check(name) == false) {
					return std::nullopt;
				}
				get_block();
				if (struct_data[pointer] == ',') {
					pointer++;//跳过","
				}
				new_type.members.push_back({ type,name });
			}
			pointer++;
			return new_type;
		};

	std::vector<StructInfo> struct_info;
	while (pointer < length) {
		auto result = create_type();
		if (result == std::nullopt) {
			continue;
		}
		struct_info.push_back(result.value());
	}
	return struct_info;
}

void StructTemplateContainer::initialize(
	const std::vector<StructInfo>& struct_info
) {
	struct_template_container_.insert("int", { *this,"int", sizeof(ScriptInt) });
	struct_template_container_.insert("float", { *this,"float", sizeof(ScriptFloat) });
	struct_template_container_.insert("char", { *this,"char", sizeof(ScriptChar) });
	//存放数据
	std::size_t ptr_size = sizeof(std::size_t);
	struct_template_container_.insert("string", { *this, "string", ptr_size });
	struct_template_container_.insert("vector_int", { *this, "vector_int", ptr_size });
	struct_template_container_.insert("vector_float", { *this, "vector_float", ptr_size });
	struct_template_container_.insert("package", { *this, "package", ptr_size });
	//存放指针
	//基本类型
	std::size_t size = struct_info.size();
	for (std::size_t i = 0; i < size; i++) {
		auto iter = struct_template_container_.find(struct_info[i].name);
		if (iter != struct_template_container_.end()) {
#ifdef SCRIPT_DEBUG
			SCRIPT_CERR
				<< "结构体[" << struct_info[i].name << "]重复定义,无法创建该结构体" << std::endl;
			ASSERT(false);
#endif // SCRIPT_DEBUG
			continue;
		}
		std::size_t member_count = struct_info[i].members.size();
		bool success = true;
		//先检查成员变量类型是否存在
		StructTemplate struct_template(*this, struct_info[i].name);
		for (std::size_t j = 0; j < member_count; j++) {
			auto type_code = find(struct_info[i].members[j].first);
			if (type_code == std::nullopt) {
#ifdef SCRIPT_DEBUG
				SCRIPT_CERR << "结构体["
					<< struct_info[i].name << "]的成员变量["
					<< struct_info[i].members[j].second << "]的类型["
					<< struct_info[i].members[j].first
					<< "]未定义,无法创建该结构体" << std::endl;
				ASSERT(false);
#endif // SCRIPT_DEBUG
				success = false;
				break;
			}
			if (struct_template.declare_variable(this, type_code.value(),
				struct_info[i].members[j].second) == false) {
#ifdef SCRIPT_DEBUG
				SCRIPT_CERR << "结构体["
					<< struct_info[i].name << "]的成员变量["
					<< struct_info[i].members[j].second << "]的类型["
					<< struct_info[i].members[j].first
					<< "]重复定义,无法创建该结构体" << std::endl;
				ASSERT(false);
#endif // SCRIPT_DEBUG
				success = false;
				break;
			}
		}
		//不允许不完整或错误的结构体存在
		if (success == false) {
			continue;
		}
		struct_template_container_.emplace(
			struct_info[i].name, std::move(struct_template)
		);
	}
}

StructProxy::StructProxy(
	std::size_t type_code, const StructTemplateContainer& struct_template_container
) :type_code_(type_code), struct_template_container_(&struct_template_container) {
}

std::optional<std::size_t> StructProxy::get_offset(
	const std::string& var_name
)const {
	return struct_template_container_->find(type_code_).get_offset(var_name);
}

std::optional<StructProxy> StructProxy::get_member(
	const std::string& var_name
) const {
	auto type_code = struct_template_container_->find(type_code_).get_member_type_code(var_name);
	if (type_code == std::nullopt) {
		return std::nullopt;
	}
	return StructProxy(type_code.value(), *struct_template_container_);
}

const std::string& StructProxy::name()const {
	return struct_template_container_->find(type_code_).name();
}

std::optional<StructProxy::CopyErrorMessage>
StructProxy::copy_all_relative_type(StructTemplateContainer& that)const {
	return struct_template_container_->copy_all_relative_type(type_code_, that);
}

bool StructProxy::is_equal(const StructProxy& that) const {
	if (that.struct_template_container_ == struct_template_container_) {
		return that.type_code_ == type_code_;
	}
	const auto& that_type = that.struct_template_container_->find(that.type_code_);
	return is_equal(that_type);
}

bool StructProxy::is_equal(const StructTemplate& that) const {
	return struct_template_container_->find(type_code_).is_equal(that);
}
