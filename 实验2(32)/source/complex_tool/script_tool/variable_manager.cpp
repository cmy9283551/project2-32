#include "complex_tool/script_tool/variable_manager.h"

VariableManager::DataPtr::DataPtr(
	std::size_t pointer, std::size_t type_code, VariableManager& variable_manager
) :pointer_(pointer), type_code_(type_code), variable_manager_(&variable_manager) {
}

std::optional<ScriptInt> VariableManager::DataPtr::int_data() const {
	if (type_code_translate() == BasicDataType::Float) {
		return ScriptInt(float_data().value());
	}
	if (type_code_translate() != BasicDataType::Int) {
		return std::nullopt;
	}
	std::size_t size = sizeof(ScriptInt);
	std::vector<unsigned char> data(size);
	variable_manager_->get_data_from_heap(pointer_, size, data);
	ScriptInt ans;
	memcpy(&ans, data.data(), size);
	return ans;
}

bool VariableManager::DataPtr::modify_int(ScriptInt data) {
	if (type_code_translate() == BasicDataType::Float) {
		return modify_float(ScriptFloat(data));
	}
	if (type_code_translate() != BasicDataType::Int) {
		return false;
	}
	std::size_t size = sizeof(ScriptInt);
	std::vector<unsigned char> byte_data(size);
	memcpy(byte_data.data(), &data, size);
	variable_manager_->modify_data(pointer_, byte_data);
	return true;
}

std::optional<ScriptFloat> VariableManager::DataPtr::float_data() const {
	if (type_code_translate() == BasicDataType::Int) {
		return ScriptFloat(int_data().value());
	}
	if (type_code_translate() != BasicDataType::Float) {
		return std::nullopt;
	}
	std::size_t size = sizeof(ScriptFloat);
	std::vector<unsigned char> data(size);
	variable_manager_->get_data_from_heap(pointer_, size, data);
	ScriptFloat ans;
	memcpy(&ans, data.data(), size);
	return ans;
}

bool VariableManager::DataPtr::modify_float(ScriptFloat data) {
	if (type_code_translate() == BasicDataType::Int) {
		return modify_int(ScriptInt(data));
	}
	if (type_code_translate() != BasicDataType::Float) {
		return false;
	}
	std::size_t size = sizeof(ScriptFloat);
	std::vector<unsigned char> byte_data(size);
	memcpy(byte_data.data(), &data, size);
	variable_manager_->modify_data(pointer_, byte_data);
	return true;
}

std::optional<ScriptChar> VariableManager::DataPtr::char_data()const {
	if (type_code_translate() != BasicDataType::Char) {
		return std::nullopt;
	}
	std::size_t size = sizeof(ScriptChar);
	std::vector<unsigned char> data(size);
	variable_manager_->get_data_from_heap(pointer_, size, data);
	ScriptChar ans;
	memcpy(&ans, data.data(), size);
	return ans;
}

bool VariableManager::DataPtr::modify_char(ScriptChar data) {
	if (type_code_translate() != BasicDataType::Char) {
		return false;
	}
	std::size_t size = sizeof(ScriptChar);
	std::vector<unsigned char> byte_data(size);
	memcpy(byte_data.data(), &data, size);
	variable_manager_->modify_data(pointer_, byte_data);
	return true;
}

std::optional<ScriptString*> VariableManager::DataPtr::string_data() {
	if (type_code_ != std::size_t(BasicDataType::String)) {
		return std::nullopt;
	}
	return variable_manager_->get_string(get_size_t());
}

std::optional<ScriptVectorInt*> VariableManager::DataPtr::vector_int_data() {
	if (type_code_translate() != BasicDataType::VectorInt) {
		return std::nullopt;
	}
	return variable_manager_->get_vector_int(get_size_t());
}

std::optional<ScriptVectorFloat*> VariableManager::DataPtr::vector_float_data() {
	if (type_code_translate() != BasicDataType::VectorFloat) {
		return std::nullopt;
	}
	return variable_manager_->get_vector_float(get_size_t());
}

std::optional<VariableManager::DataPtr> VariableManager::DataPtr::create_member(
	const std::string& type_name, const std::string& var_name
) {
	if (type_code_translate() != BasicDataType::Package) {
		return std::nullopt;
	}
	std::size_t package_index = get_size_t();
	//加入heap
	auto type_code = variable_manager_->get_type_code(type_name);
	if (type_code == std::nullopt) {
		return std::nullopt;
	}
	InternalPtr member_ptr = variable_manager_->create_variable_memory(type_code.value());
	//查重,加入成员列表
	auto have = variable_manager_->package_get_member_ptr(package_index, var_name);
	if (have != std::nullopt) {
		std::nullopt;
	}
	variable_manager_->package_push_member_ptr(package_index, var_name, member_ptr);
	return { { member_ptr.pointer,type_code.value(),*variable_manager_} };
}

std::optional<VariableManager::DataPtr> VariableManager::DataPtr::find(
	const std::string& var_name
)const {
	if (type_code_translate() == BasicDataType::Package) {
		auto result = variable_manager_->package_get_member_ptr(get_size_t(), var_name);
		if (result == std::nullopt) {
			return std::nullopt;
		}
		InternalPtr member_ptr = result.value();
		return { { member_ptr.pointer,member_ptr.type_code,*variable_manager_ } };
	}
	if (type_code_translate() != BasicDataType::Struct) {
		return std::nullopt;
	}

	auto result = variable_manager_->struct_get_member_ptr(pointer_, type_code_, var_name);
	if (result == std::nullopt) {
		return std::nullopt;
	}
	InternalPtr member_ptr = result.value();
	return { { member_ptr.pointer,member_ptr.type_code,*variable_manager_ } };
}

VariableManager::BasicDataType VariableManager::DataPtr::type_code_translate() const {
	if (type_code_ >= StructTemplateContainer::basic_type_count()) {
		return BasicDataType::Struct;
	}
	return BasicDataType(type_code_);
}

std::size_t VariableManager::DataPtr::get_size_t() const {
	std::size_t ans, size = sizeof(std::size_t);
	std::vector<unsigned char> data(size);
	variable_manager_->get_data_from_heap(pointer_, size, data);
	memcpy(&ans, data.data(), size);
	return ans;
}

VariableManager::ConstDataPtr::ConstDataPtr(
	std::size_t pointer, std::size_t type_code, const VariableManager& variable_manager
) :pointer_(pointer), type_code_(type_code), variable_manager_(&variable_manager) {
}

std::optional<ScriptInt> VariableManager::ConstDataPtr::int_data() const {
	if (type_code_translate() == BasicDataType::Float) {
		return ScriptInt(float_data().value());
	}
	if (type_code_translate() != BasicDataType::Int) {
		return std::nullopt;
	}
	std::size_t size = sizeof(ScriptInt);
	std::vector<unsigned char> data(size);
	variable_manager_->get_data_from_heap(pointer_, size, data);
	ScriptInt ans;
	memcpy(&ans, data.data(), size);
	return ans;
}

std::optional<ScriptFloat> VariableManager::ConstDataPtr::float_data() const {
	if (type_code_translate() == BasicDataType::Int) {
		return ScriptFloat(int_data().value());
	}
	if (type_code_translate() != BasicDataType::Float) {
		return std::nullopt;
	}
	std::size_t size = sizeof(ScriptFloat);
	std::vector<unsigned char> data(size);
	variable_manager_->get_data_from_heap(pointer_, size, data);
	ScriptFloat ans;
	memcpy(&ans, data.data(), size);
	return ans;
}

std::optional<ScriptChar> VariableManager::ConstDataPtr::char_data() const {
	if (type_code_translate() != BasicDataType::Char) {
		return std::nullopt;
	}
	std::size_t size = sizeof(ScriptChar);
	std::vector<unsigned char> data(size);
	variable_manager_->get_data_from_heap(pointer_, size, data);
	ScriptChar ans;
	memcpy(&ans, data.data(), size);
	return ans;
}

std::optional<const ScriptString*> VariableManager::ConstDataPtr::string_data() const {
	if (type_code_translate() != BasicDataType::String) {
		return std::nullopt;
	}
	return variable_manager_->get_const_string(get_size_t());
}

std::optional<const ScriptVectorInt*> VariableManager::ConstDataPtr::vector_int_data() const {
	if (type_code_translate() != BasicDataType::VectorInt) {
		return std::nullopt;
	}
	return variable_manager_->get_const_vector_int(get_size_t());
}

std::optional<const ScriptVectorFloat*> VariableManager::ConstDataPtr::vector_float_data() const {
	if (type_code_translate() != BasicDataType::VectorFloat) {
		return std::nullopt;
	}
	return variable_manager_->get_const_vector_float(get_size_t());
}

std::optional<VariableManager::ConstDataPtr> VariableManager::ConstDataPtr::find(
	const std::string& var_name
)const {
	if (type_code_translate() == BasicDataType::Package) {
		auto result = variable_manager_->package_get_member_ptr(get_size_t(), var_name);
		if (result == std::nullopt) {
			return std::nullopt;
		}
		InternalPtr member_ptr = result.value();
		return { { member_ptr.pointer,member_ptr.type_code,*variable_manager_ } };
	}
	if (type_code_translate() != BasicDataType::Struct) {
		return std::nullopt;
	}

	auto result = variable_manager_->struct_get_member_ptr(pointer_, type_code_, var_name);
	if (result == std::nullopt) {
		return std::nullopt;
	}
	InternalPtr member_ptr = result.value();
	return { { member_ptr.pointer,member_ptr.type_code,*variable_manager_ } };
}

VariableManager::BasicDataType VariableManager::ConstDataPtr::type_code_translate() const {
	if (type_code_ >= StructTemplateContainer::basic_type_count()) {
		return BasicDataType::Struct;
	}
	return BasicDataType(type_code_);
}

std::size_t VariableManager::ConstDataPtr::get_size_t()const {
	std::size_t ans, size = sizeof(std::size_t);
	std::vector<unsigned char> data(size);
	variable_manager_->get_data_from_heap(pointer_, size, data);
	memcpy(&ans, data.data(), size);
	return ans;
}

void VariableManager::print_struct_data() const {
	SCRIPT_COUT << "这个类不支持输出结构体数据\n";
}

void VariableManager::print_heap_data() const {
	SCRIPT_COUT << "这个类不支持输出堆数据\n";
}

VariableManager::StructTemplate::StructTemplate(
	const StructTemplateContainer& struct_template_container,
	const std::string& name,
	std::size_t size
) :struct_template_container_(&struct_template_container), name_(name), size_(size) {
}

std::size_t VariableManager::StructTemplate::size()const {
	return size_;
}

std::optional<std::size_t> VariableManager::StructTemplate::get_member_type_code(
	const std::string& var_name
) const {
	auto iter = type_code_container_.find(var_name);
	if (iter == type_code_container_.cend()) {
		return std::nullopt;
	}
	return { iter.second() };
}

std::optional<std::size_t> VariableManager::StructTemplate::get_offset(
	const std::string& var_name
) const {
	auto result = type_code_container_.find_serial_number(var_name);
	if (result.second == false) {
		return std::nullopt;
	}
	std::size_t offset = 0;
	for (std::size_t i = 0; i < result.first; i++) {
		offset += struct_template_container_->find(
			type_code_container_.find(i).second()
		).size();
	}
	return offset;
}

std::size_t VariableManager::StructTemplate::declare_variable(
	std::size_t type_code, const std::string& var_name
) {
	size_ += struct_template_container_->find(type_code).size();
	return type_code_container_.insert(var_name, type_code);
}

std::ostream& operator<<(std::ostream& os, const VariableManager::StructTemplate& st) {
	os << "结构体[" << st.name() << "],大小["
		<< st.size() << "]:\n";
	auto iter = st.type_code_container_.cbegin();
	for (; iter != st.type_code_container_.cend(); ++iter) {
		os << "    成员变量<" << st.struct_template_container_->find(iter.second()).name()
			<< "," << iter.first() << ">: 类型代码["
			<< iter.second() << "], 偏移量["
			<< st.get_offset(iter.first()).value() << "]\n";
	}
	return os;
}

const std::size_t VariableManager::StructTemplateContainer::basic_type_count_ = 7;

VariableManager::StructTemplateContainer::StructTemplateContainer(
	const std::string& struct_data
) {
	struct_template_container_.insert("Int", { *this,"Int", sizeof(ScriptInt) });
	struct_template_container_.insert("Float", { *this,"Float", sizeof(ScriptFloat) });
	struct_template_container_.insert("Char", { *this,"Char", sizeof(ScriptChar) });
	//存放数据
	std::size_t size = sizeof(std::size_t);
	struct_template_container_.insert("String", { *this, "String", size });
	struct_template_container_.insert("VectorInt", { *this, "VectorInt", size });
	struct_template_container_.insert("VectorFloat", { *this, "VectorFloat", size });
	struct_template_container_.insert("Package", { *this, "Package", size });
	//存放指针
	//基本类型
	parse(struct_data);
}

void VariableManager::StructTemplateContainer::CopyErrorMessage::operator+=(
	CopyErrorMessage& that
	) {
	error_message.reserve(error_message.size() + that.error_message.size());
	error_message.insert(
		error_message.end(),
		std::make_move_iterator(that.error_message.begin()),
		std::make_move_iterator(that.error_message.begin())
	);
}

std::optional<VariableManager::StructTemplateContainer::CopyErrorMessage>
VariableManager::StructTemplateContainer::copy_all_relative_type(
	std::size_t type_code, StructTemplateContainer& that
)const {
	CopyErrorMessage message;
	const auto& this_type = find(type_code);
	const auto& members = this_type.members();
	std::size_t size = members.size();
	for (std::size_t i = 0; i < size; i++) {
		const std::string& type_name = struct_template_container_[members[i]].name();
		auto that_have_type = that.find(type_name);
		if (that_have_type == std::nullopt) {
			auto result = copy_all_relative_type(members[i], that);
			if (result != std::nullopt) {
				message += result.value();
			}
			continue;
		}
		if (that.find(that_have_type.value()).members()
			!=
			struct_template_container_[members[i]].members()) {
			message.error_message.emplace_back(type_name);
		}
	}
	if (message.error_message.size() == 0) {
		return std::nullopt;
	}
	return message;
}

std::optional<std::size_t> VariableManager::StructTemplateContainer::find(
	const std::string& name
) const {
	auto iter = struct_template_container_.find_serial_number(name);
	if (iter.second == false) {
		return std::nullopt;
	}
	return iter.first;
}

const VariableManager::StructTemplate& VariableManager::StructTemplateContainer::find(
	std::size_t type_code
) const {
	return struct_template_container_.find(type_code).second();
}

const IndexedMap<std::string, VariableManager::StructTemplate>&
VariableManager::StructTemplateContainer::all_types() const {
	return struct_template_container_;
}

const std::string& VariableManager::StructTemplate::name()const {
	return name_;
}

const IndexedMap<std::string, std::size_t>& VariableManager::StructTemplate::members() const {
	return type_code_container_;
}

std::ostream& operator<<(std::ostream& os, const VariableManager::StructTemplateContainer& stc) {
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

std::size_t VariableManager::StructTemplateContainer::basic_type_count() {
	return basic_type_count_;
}

void VariableManager::StructTemplateContainer::parse(const std::string& struct_data) {
	std::size_t length = struct_data.length(), pointer = 0;
	while (pointer < length) {
		create_type(struct_data, pointer);
	}
}

void VariableManager::StructTemplateContainer::create_type(
	const std::string& struct_data, std::size_t& pointer
) {
#ifdef SCRIPT_DEBUG
	auto check = [](const std::string& name) {
		auto is_character = [](char c)->bool {
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
			};
		auto is_number = [](char c)->bool {
			return c = '0' && c <= '9';
			};
		std::size_t length = name.length();
		if (length == 0) {
			SCRIPT_CERR
				<< "初始化VariableManager时,列表中存在空名称" << std::endl;
			ASSERT(false);
		}
		if (is_character(name[0]) == false && name[0] != '_') {
			SCRIPT_CERR
				<< "初始化VariableManager时,列表中存在不合法名称["
				<< name << "]" << std::endl;
			ASSERT(false);
		}
		for (std::size_t i = 0; i < length; i++) {
			if (is_character(
				name[i]) == false &&
				is_number(name[i]) == false &&
				name[i] != '_'
				) {
				SCRIPT_CERR
					<< "初始化VariableManager时,列表中存在不合法名称["
					<< name << "]" << std::endl;
				ASSERT(false);
			}
		}
		};
#else
	auto check = [](const std::string& name) {};
#endif // SCRIPT_DEBUG
	std::string type, name, new_type;
	std::size_t length = struct_data.length();

	auto get_block = [length](const std::string& struct_data, std::size_t& pointer) {
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

	get_block(struct_data, pointer);
	while (struct_data[pointer] != ':' && struct_data[pointer] != ' ') {
		new_type.push_back(struct_data[pointer]);
		pointer++;
		if (pointer >= length) {
			break;
		}
	}
	check(new_type);
	std::size_t index = struct_template_container_.insert(new_type, { *this ,new_type });
#ifdef VARIABLE_MANAGER_LOG
	SCRIPT_CLOG
		<< "VariableManager: 解析到结构体[" << new_type << "]\n";
#endif // VARIABLE_MANAGER_LOG

	get_block(struct_data, pointer);
	pointer++;//跳过":"

	while (struct_data[pointer] != ';') {

		get_block(struct_data, pointer);

		type.clear();
		while (struct_data[pointer] != ';' && struct_data[pointer] != ' ') {
			type.push_back(struct_data[pointer]);
			pointer++;
			if (pointer >= length) {
				break;
			}
		}
		auto result = struct_template_container_.find_serial_number(type);
		if (result.second == false) {
			SCRIPT_CERR
				<< "初始化VariableManager时，使用了不存在类型[" << type << "]" << std::endl;
			ASSERT(false);
		}
		check(type);

		get_block(struct_data, pointer);

		name.clear();
		while (
			struct_data[pointer] != ';' &&
			struct_data[pointer] != ',' &&
			struct_data[pointer] != ' '
			) {
			name.push_back(struct_data[pointer]);
			pointer++;
			if (pointer >= length) {
				break;
			}
		}
		check(name);

		struct_template_container_[index].declare_variable(result.first, name);
#ifdef VARIABLE_MANAGER_LOG
		SCRIPT_CLOG
			<< "VariableManager: 解析到成员变量<" << type << "," << name << ">\n";
#endif // VARIABLE_MANAGER_LOG

		get_block(struct_data, pointer);

		if (struct_data[pointer] == ',') {
			pointer++;//跳过","
		}
		if (pointer >= length) {
			break;
		}
	}
	pointer++;
}

VariableManager::StructProxy::StructProxy(
	std::size_t type_code, const StructTemplateContainer& struct_template_container
) :type_code_(type_code), struct_template_container_(&struct_template_container) {
}

std::optional<std::size_t> VariableManager::StructProxy::get_offset(
	const std::string& var_name
)const {
	return struct_template_container_->find(type_code_).get_offset(var_name);
}

const IndexedMap<std::string, std::size_t>& VariableManager::StructProxy::members()const {
	return struct_template_container_->find(type_code_).members();
}

const std::string& VariableManager::StructProxy::name()const {
	return struct_template_container_->find(type_code_).name();
}

std::optional<VariableManager::StructProxy::CopyErrorMessage>
VariableManager::StructProxy::copy_all_relative_type(StructTemplateContainer& that) {
	return struct_template_container_->copy_all_relative_type(type_code_, that);
}

BasicVariableManager::BasicVariableManager(const std::string& struct_data)
	:struct_template_container_(struct_data) {
}

std::optional<VariableManager::DataPtr> BasicVariableManager::find(
	const std::string& var_name
) {
	auto iter = name_space_.find(var_name);
	if (iter == name_space_.end()) {
		return std::nullopt;
	}
	return { {iter.second().pointer,iter.second().type_code,*this} };
}

std::optional<VariableManager::ConstDataPtr> BasicVariableManager::find(
	const std::string& var_name
) const {
	auto iter = name_space_.find(var_name);
	if (iter == name_space_.cend()) {
		return std::nullopt;
	}
	return { {iter.second().pointer,iter.second().type_code,*this} };
}

std::optional<BasicVariableManager::StructProxy> BasicVariableManager::find_type(
	const std::string& type_name
) const{
	auto result = struct_template_container_.find(type_name);
	if (result == std::nullopt) {
		return std::nullopt;
	}
	return StructProxy(result.value(),struct_template_container_);
}

std::optional<VariableManager::DataPtr> BasicVariableManager::create_variable(
	const std::string& type_name, const std::string& var_name
) {
	std::optional<std::size_t> type_code = get_type_code(type_name);
	if (type_code == std::nullopt) {
		return std::nullopt;
	}
	std::optional<InternalPtr> ptr = declare_variable(type_code.value(), var_name);
	if (ptr == std::nullopt) {
		return std::nullopt;
	}
	return { {ptr.value().pointer,type_code.value(),*this} };
}

void BasicVariableManager::get_name_vector(
	std::vector<std::string>& name_vector
) const{
	auto v_iter = name_space_.cbegin();
	for (; v_iter != name_space_.cend(); ++v_iter) {
		name_vector.emplace_back(v_iter.first());
	}
	const auto& stc = struct_template_container_.all_types();
	auto t_iter = stc.cbegin();
	for (; t_iter != stc.cend(); ++t_iter) {
		name_vector.emplace_back(t_iter.first());
	}
}

void BasicVariableManager::print_struct_data() const {
	SCRIPT_COUT << struct_template_container_;
}

void BasicVariableManager::print_heap_data() const {
	struct PrintFormat {
		std::size_t max_vec_element_per_line = 6;
	}format;

	auto print_int = [](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.int_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				SCRIPT_COUT << "    ";
			}
			SCRIPT_COUT
				<< "基础类型变量<Int," << name << "> = ["
				<< data.value() << "]\n";
		};

	auto print_float = [](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.float_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				SCRIPT_COUT << "    ";
			}
			SCRIPT_COUT
				<< "基础类型变量<Float," << name << "> = ["
				<< data.value() << "]\n";
		};

	auto print_char = [](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.char_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				SCRIPT_COUT << "    ";
			}
			SCRIPT_COUT
				<< "基础类型变量<Char," << name << "> = ["
				<< data.value() << "]\n";
		};

	auto print_string = [](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.string_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				SCRIPT_COUT << "    ";
			}
			SCRIPT_COUT
				<< "基础类型变量<String," << name << "> = ["
				<< *data.value() << "]\n";
		};

	auto print_vector_int = [format](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.vector_int_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				SCRIPT_COUT << "    ";
			}
			SCRIPT_COUT
				<< "基础类型变量<VectorInt," << name << "> = {";
			const ScriptVectorInt& vec = *data.value();
			std::size_t size = vec.size();
			for (std::size_t i = 0; i < size; i++) {
				if (i % format.max_vec_element_per_line == 0) {
					SCRIPT_COUT << "\n";
					for (std::size_t i = 0; i < block_count; i++) {
						SCRIPT_COUT << "    ";
					}
				}
				SCRIPT_COUT
					<< "[" << i << "](" << vec[i] << ") ";
			}
			SCRIPT_COUT << "}\n";
		};

	auto print_vector_float = [format](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.vector_float_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				SCRIPT_COUT << "    ";
			}
			SCRIPT_COUT
				<< "基础类型变量<VectorFloat," << name << "> = {";
			const ScriptVectorFloat& vec = *data.value();
			std::size_t size = vec.size();
			for (std::size_t i = 0; i < size; i++) {
				if (i % format.max_vec_element_per_line == 0) {
					SCRIPT_COUT << "\n";
					for (std::size_t i = 0; i < block_count; i++) {
						SCRIPT_COUT << "    ";
					}
				}
				SCRIPT_COUT
					<< "[" << i << "](" << vec[i] << ") ";
			}
			for (std::size_t i = 0; i < block_count; i++) {
				SCRIPT_COUT << "    ";
			}
			SCRIPT_COUT << "}\n";
		};

	auto print_package = [format, this](
		auto func, const std::string& name, const InternalPtr& ptr, std::size_t block_count
		) {
			for (std::size_t i = 0; i < block_count; i++) {
				SCRIPT_COUT << "    ";
			}
			SCRIPT_COUT
				<< "基础类型变量<Package," << name << "> = {\n";

			std::size_t data;
			memcpy(&data, byte_heap_.data() + ptr.pointer, sizeof(std::size_t));

			const Package& package = package_heap_[data];
			std::size_t size = package.size();
			auto piter = package.cbegin();
			//排序
			std::vector<IndexedMap<std::string, InternalPtr>::const_iterator> piters;
			for (; piter != package.cend(); ++piter) {
				piters.emplace_back(piter);
			}
			std::sort(piters.begin(), piters.end(), [](const auto& x, const auto& y) {
				return x.position() < y.position();
				});

			for (std::size_t i = 0; i < size; i++) {
				func(piters[i].first(), piters[i].second(), block_count + 1);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				SCRIPT_COUT << "    ";
			}
			SCRIPT_COUT << "}\n";
		};

	auto print_struct = [format, this](
		auto func, const std::string& name, const InternalPtr& ptr, std::size_t block_count
		) {
			for (std::size_t i = 0; i < block_count; i++) {
				SCRIPT_COUT << "    ";
			}
			SCRIPT_COUT
				<< "结构体变量<Struct," << name << "> = {\n";

			const IndexedMap<std::string, std::size_t>& members =
				struct_template_container_.find(ptr.type_code).members();
			std::vector<IndexedMap<std::string, std::size_t>::const_iterator> piters;
			auto piter = members.cbegin();
			for (; piter != members.cend(); ++piter) {
				piters.emplace_back(piter);
			}
			std::sort(piters.begin(), piters.end(), [](const auto& x, const auto& y) {
				return x.position() < y.position();
				});

			std::size_t member_count = members.size(), offset = 0, size = 0;
			for (std::size_t i = 0; i < member_count; i++) {
				size = struct_template_container_.find(members[i]).size();
				func(piters[i].first(), { ptr.pointer + offset,members[i] }, block_count + 1);
				offset += size;
			}


			for (std::size_t i = 0; i < block_count; i++) {
				SCRIPT_COUT << "    ";
			}
			SCRIPT_COUT << "}\n";
		};

	auto print_data = [&](
		this auto self, const std::string& name, const InternalPtr& ptr, std::size_t block_count
		)->void {
			ConstDataPtr c_ptr = { ptr.pointer, ptr.type_code,*this };
			switch (c_ptr.type_code_translate())
			{
			case VariableManager::BasicDataType::Int:
				print_int(name, c_ptr, block_count);
				break;
			case VariableManager::BasicDataType::Float:
				print_float(name, c_ptr, block_count);
				break;
			case VariableManager::BasicDataType::Char:
				print_char(name, c_ptr, block_count);
				break;
			case VariableManager::BasicDataType::String:
				print_string(name, c_ptr, block_count);
				break;
			case VariableManager::BasicDataType::VectorInt:
				print_vector_int(name, c_ptr, block_count);
				break;
			case VariableManager::BasicDataType::VectorFloat:
				print_vector_float(name, c_ptr, block_count);
				break;
			case VariableManager::BasicDataType::Package:
				print_package(self, name, ptr, block_count);
				break;
			case VariableManager::BasicDataType::Struct:
				print_struct(self, name, ptr, block_count);
				break;
			default:
				break;
			}
		};

	auto visitors = name_space_.get_visitor();
	std::size_t size = visitors.size();
	for (std::size_t i = 0; i < size; i++) {
		print_data(*visitors[i].first, *visitors[i].second, 0);
	}
}

ScriptString* BasicVariableManager::get_string(std::size_t ptr) {
	return &string_heap_[ptr];
}

const ScriptString* BasicVariableManager::get_const_string(std::size_t ptr) const {
	return &string_heap_[ptr];
}

ScriptVectorInt* BasicVariableManager::get_vector_int(std::size_t ptr) {
	return &vector_int_heap_[ptr];
}

const ScriptVectorInt* BasicVariableManager::get_const_vector_int(std::size_t ptr) const {
	return &vector_int_heap_[ptr];
}

ScriptVectorFloat* BasicVariableManager::get_vector_float(std::size_t ptr) {
	return &vector_float_heap_[ptr];
}

const ScriptVectorFloat* BasicVariableManager::get_const_vector_float(std::size_t ptr) const {
	return &vector_float_heap_[ptr];
}

std::size_t BasicVariableManager::gen_string() {
	string_heap_.emplace_back();
	return string_heap_.size() - 1;
}

std::size_t BasicVariableManager::gen_vector_int() {
	vector_int_heap_.emplace_back();
	return vector_int_heap_.size() - 1;
}

std::size_t BasicVariableManager::gen_vector_float() {
	vector_float_heap_.emplace_back();
	return vector_float_heap_.size() - 1;
}

std::size_t BasicVariableManager::gen_package() {
	package_heap_.emplace_back();
	return package_heap_.size() - 1;
}

std::optional<VariableManager::InternalPtr> BasicVariableManager::declare_variable(
	std::size_t type_code, const std::string& var_name
) {
	//查重
	auto iter = name_space_.find(var_name);
	if (iter != name_space_.end()) {
		return std::nullopt;
	}

	//变量名不能和类名相同
	auto s_iter = struct_template_container_.find(var_name);
	if (s_iter != std::nullopt) {
		return std::nullopt;
	}

	InternalPtr ptr = create_variable_memory(type_code);
	name_space_.insert(var_name, ptr);
	return ptr;
}

VariableManager::InternalPtr BasicVariableManager::create_variable_memory(std::size_t type_code) {
	std::size_t total_size = byte_heap_.size(), size = get_type_size(type_code);
	byte_heap_.resize(total_size + size);
	InternalPtr ptr(total_size, type_code);

	BasicDataType type;
	if (type_code < StructTemplateContainer::basic_type_count()) {
		type = BasicDataType(type_code);
	}
	else {
		type = BasicDataType::Struct;
	}
	std::size_t data_ptr;
	switch (type)
	{
	case VariableManager::BasicDataType::String:
		data_ptr = gen_string();
		break;
	case VariableManager::BasicDataType::VectorInt:
		data_ptr = gen_vector_int();
		break;
	case VariableManager::BasicDataType::VectorFloat:
		data_ptr = gen_vector_float();
		break;
	case VariableManager::BasicDataType::Package:
		data_ptr = gen_package();
		break;
	case VariableManager::BasicDataType::Struct:
		init_struct_memory(ptr.pointer, type_code);
		return ptr;
	default:
		return ptr;
	}
	memcpy(byte_heap_.data() + ptr.pointer, &data_ptr, size);
	return ptr;
}

void BasicVariableManager::init_struct_memory(std::size_t pointer, std::size_t type_code) {
	const IndexedMap<std::string, std::size_t>& members =
		struct_template_container_.find(type_code).members();
	std::size_t member_count = members.size(), offset = 0, size = 0;
	for (std::size_t i = 0, data_ptr; i < member_count; i++) {
		size = struct_template_container_.find(members[i]).size();
		BasicDataType type = BasicDataType(members[i]);
		switch (type)
		{
		case VariableManager::BasicDataType::Int:
			break;
		case VariableManager::BasicDataType::Float:
			break;
		case VariableManager::BasicDataType::Char:
			break;
		case VariableManager::BasicDataType::String:
			data_ptr = gen_string();
			memcpy(byte_heap_.data() + pointer + offset, &data_ptr, size);
			break;
		case VariableManager::BasicDataType::VectorInt:
			data_ptr = gen_vector_int();
			memcpy(byte_heap_.data() + pointer + offset, &data_ptr, size);
			break;
		case VariableManager::BasicDataType::VectorFloat:
			data_ptr = gen_vector_float();
			memcpy(byte_heap_.data() + pointer + offset, &data_ptr, size);
			break;
		case VariableManager::BasicDataType::Package:
			data_ptr = gen_package();
			memcpy(byte_heap_.data() + pointer + offset, &data_ptr, size);
			break;
		case VariableManager::BasicDataType::Struct:
			init_struct_memory(pointer + offset, members[i]);
			break;
		default:
			break;
		}
		offset += size;
	}
}

void BasicVariableManager::get_data_from_heap(
	std::size_t ptr, std::size_t size, std::vector<unsigned char>& data
) const {
	memcpy(data.data(), byte_heap_.data() + ptr, size);
}

void BasicVariableManager::modify_data(std::size_t ptr, const std::vector<unsigned char>& data) {
	memcpy(byte_heap_.data() + ptr, data.data(), data.size());
}

std::optional<VariableManager::InternalPtr> BasicVariableManager::package_get_member_ptr(
	std::size_t package_index, std::size_t member_index
) const {
	const Package& package = package_heap_[package_index];
	if (member_index >= package.size()) {
		return std::nullopt;
	}
	return package.find(member_index).second();
}

std::optional<VariableManager::InternalPtr> BasicVariableManager::package_get_member_ptr(
	std::size_t package_index, const std::string& var_name
) const {
	const Package& package = package_heap_[package_index];
	auto iter = package.find(var_name);
	if (iter == package.cend()) {
		return std::nullopt;
	}
	return { iter.second() };
}

void BasicVariableManager::package_push_member_ptr(
	std::size_t package_index, const std::string& var_name, InternalPtr member_ptr
) {
	package_heap_[package_index].insert(var_name, member_ptr);
}

std::optional<std::size_t> BasicVariableManager::get_type_code(const std::string& type_name) const {
	return struct_template_container_.find(type_name);
}

std::size_t BasicVariableManager::get_type_size(std::size_t type_code) const {
	return struct_template_container_.find(type_code).size();
}

std::optional<VariableManager::InternalPtr> BasicVariableManager::struct_get_member_ptr(
	std::size_t pointer, std::size_t type_code, const std::string& var_name
) const {
	const StructTemplate& struct_template = struct_template_container_.find(type_code);
	auto offset = struct_template.get_offset(var_name);
	auto member_type_code = struct_template.get_member_type_code(var_name);
	if (offset == std::nullopt || member_type_code == std::nullopt) {
		return std::nullopt;
	}
	return { {pointer + offset.value(),member_type_code.value()} };
}