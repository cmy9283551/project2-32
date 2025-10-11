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
	//����heap
	auto type_code = variable_manager_->get_type_code(type_name);
	if (type_code == std::nullopt) {
		return std::nullopt;
	}
	InternalPtr member_ptr = variable_manager_->create_variable_memory(type_code.value());
	//����,�����Ա�б�
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

VariableManager::VariableManager(const std::string& name)
	:name_(name) {
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

void VariableManager::print_struct_data(std::ostream& os) const {
	os << "[" << name_ << "]:\n";
	os << "����಻֧������ṹ������\n";
}

void VariableManager::print_heap_data(std::ostream& os) const {
	os << "[" << name_ << "]:\n";
	os << "����಻֧�����������\n";
}

const std::string& VariableManager::name()const {
	return name_;
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
	auto iter = member_container_.find(var_name);
	if (iter == member_container_.cend()) {
		return std::nullopt;
	}
	return iter.second().type_code;
}

std::optional<std::size_t> VariableManager::StructTemplate::get_offset(
	const std::string& var_name
) const {
	auto iter = member_container_.find(var_name);
	if (iter == member_container_.cend()) {
		return std::nullopt;
	}
	return iter.second().offset;
}

bool VariableManager::StructTemplate::declare_variable(
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

std::ostream& operator<<(std::ostream& os, const VariableManager::StructTemplate& st) {
	os << "�ṹ��[" << st.name() << "],��С["
		<< st.size() << "]:\n";
	auto iter = st.member_container_.cbegin();
	for (; iter != st.member_container_.cend(); ++iter) {
		os << "    ��Ա����<" << st.struct_template_container_->find(iter.second().type_code).name()
			<< "," << iter.first() << ">: ���ʹ���["
			<< iter.second().type_code << "], ƫ����["
			<< iter.second().offset << "]\n";
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
	//�������
	std::size_t size = sizeof(std::size_t);
	struct_template_container_.insert("String", { *this, "String", size });
	struct_template_container_.insert("VectorInt", { *this, "VectorInt", size });
	struct_template_container_.insert("VectorFloat", { *this, "VectorFloat", size });
	struct_template_container_.insert("Package", { *this, "Package", size });
	//���ָ��
	//��������
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

const IndexedMap<std::string, VariableManager::StructTemplate::MemberInfo>&
VariableManager::StructTemplate::members() const {
	return member_container_;
}

bool VariableManager::StructTemplate::is_equal(const StructTemplate& that) const {
	if (that.struct_template_container_ == struct_template_container_) {
		return that.name() == name();
	};
	if (that.size() != size()) {
		return false;
	}
	const auto& that_members = that.members();
	auto that_iter = that_members.cbegin();
	for (; that_iter != that_members.cend(); ++that_iter) {
		//ȷ����Ա����������ͬ
		auto iter = member_container_.find(that_iter.first());
		if (iter == member_container_.cend()) {
			return false;
		}
		//˳����Ӱ��,�����ԱȽ�offset�ж�
		if (iter.second().offset != that_iter.second().offset) {
			return false;
		}
		//ȷ����Ա����������ͬ
		if (struct_template_container_->find(iter.second().type_code).is_equal(
			that.struct_template_container_->find(that_iter.second().type_code)
		) == false) {
			return false;
		}
	}
	return true;
}

std::ostream& operator<<(std::ostream& os, const VariableManager::StructTemplateContainer& stc) {
	std::size_t size = stc.struct_template_container_.size();
	for (std::size_t i = 0; i < size; i++) {
		if (i < stc.basic_type_count()) {
			os << "���ʹ���["
				<< i << "]:��������[" << stc.struct_template_container_.find(i).second().name()
				<< "],��С["
				<< stc.struct_template_container_.find(i).second().size() << "]\n";
			continue;
		}
		os << "���ʹ���[" << i << "]" << stc.struct_template_container_[i];
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
				<< "��ʼ��VariableManagerʱ,�б��д��ڿ�����" << std::endl;
			ASSERT(false);
		}
		if (is_character(name[0]) == false && name[0] != '_') {
			SCRIPT_CERR
				<< "��ʼ��VariableManagerʱ,�б��д��ڲ��Ϸ�����["
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
					<< "��ʼ��VariableManagerʱ,�б��д��ڲ��Ϸ�����["
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
		<< "VariableManager: �������ṹ��[" << new_type << "]\n";
#endif // VARIABLE_MANAGER_LOG

	get_block(struct_data, pointer);
	pointer++;//����":"

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
				<< "��ʼ��VariableManagerʱ��ʹ���˲���������[" << type << "]" << std::endl;
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

		if (struct_template_container_[index].declare_variable(this, result.first, name) == false) {
			SCRIPT_CERR
				<< "��ʼ��VariableManagerʱ,�ṹ��[" << new_type
				<< "]�д����ظ���Ա����[" << name << "]" << std::endl;
			ASSERT(false);
		}
#ifdef VARIABLE_MANAGER_LOG
		SCRIPT_CLOG
			<< "VariableManager: ��������Ա����<" << type << "," << name << ">\n";
#endif // VARIABLE_MANAGER_LOG

		get_block(struct_data, pointer);

		if (struct_data[pointer] == ',') {
			pointer++;//����","
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

std::optional<VariableManager::StructProxy> VariableManager::StructProxy::get_member(
	const std::string& var_name
) const {
	auto type_code = struct_template_container_->find(type_code_).get_member_type_code(var_name);
	if (type_code == std::nullopt) {
		return std::nullopt;
	}
	return StructProxy(type_code.value(), *struct_template_container_);
}

const std::string& VariableManager::StructProxy::name()const {
	return struct_template_container_->find(type_code_).name();
}

std::optional<VariableManager::StructProxy::CopyErrorMessage>
VariableManager::StructProxy::copy_all_relative_type(StructTemplateContainer& that) {
	return struct_template_container_->copy_all_relative_type(type_code_, that);
}

bool VariableManager::StructProxy::is_equal(const StructProxy& that) const {
	if (that.struct_template_container_ == struct_template_container_) {
		return that.type_code_ == type_code_;
	}
	const auto& that_type = that.struct_template_container_->find(that.type_code_);
	return is_equal(that_type);
}

bool VariableManager::StructProxy::is_equal(const StructTemplate& that) const {
	return struct_template_container_->find(type_code_).is_equal(that);
}

BasicVariableManager::BasicVariableManager(
	const std::string& name, const std::string& struct_data
) :VariableManager(name), struct_template_container_(struct_data) {
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
) const {
	auto result = struct_template_container_.find(type_name);
	if (result == std::nullopt) {
		return std::nullopt;
	}
	return StructProxy(result.value(), struct_template_container_);
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

bool BasicVariableManager::has_name_conflict(const VariableManager& vm) const {
	auto iter = name_space_.cbegin();
	for (; iter != name_space_.cend(); ++iter) {
		auto that_ptr = vm.find(iter.first());
		if (that_ptr != std::nullopt) {
			return true;
		}
	}
	const auto& all_types = struct_template_container_.all_types();
	auto type_iter = all_types.cbegin();
	for (; type_iter != all_types.cend(); ++type_iter) {
		auto that_type = vm.find_type(type_iter.first());
		if (that_type != std::nullopt) {

			return true;
		}
	}
	return false;
}

bool BasicVariableManager::has_name_conflict(const FunctionManager& fm) const {

	return false;
}


void BasicVariableManager::print_struct_data(std::ostream& os) const {
	os << "[" << name_ << "]:\n";
	os << struct_template_container_;
}

void BasicVariableManager::print_heap_data(std::ostream& os)const {
	struct PrintFormat {
		std::size_t max_vec_element_per_line = 6;
	}format;

	auto print_int = [&](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.int_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				os << "    ";
			}
			os << "�������ͱ���<Int," << name << "> = ["
				<< data.value() << "]\n";
		};

	auto print_float = [&](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.float_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				os << "    ";
			}
			os << "�������ͱ���<Float," << name << "> = ["
				<< data.value() << "]\n";
		};

	auto print_char = [&](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.char_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				os << "    ";
			}
			os << "�������ͱ���<Char," << name << "> = ["
				<< data.value() << "]\n";
		};

	auto print_string = [&](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.string_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				os << "    ";
			}
			os << "�������ͱ���<String," << name << "> = ["
				<< *data.value() << "]\n";
		};

	auto print_vector_int = [&, format](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.vector_int_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				os << "    ";
			}
			os
				<< "�������ͱ���<VectorInt," << name << "> = {";
			const ScriptVectorInt& vec = *data.value();
			std::size_t size = vec.size();
			for (std::size_t i = 0; i < size; i++) {
				if (i % format.max_vec_element_per_line == 0) {
					os << "\n";
					for (std::size_t i = 0; i < block_count; i++) {
						os << "    ";
					}
				}
				os << "[" << i << "](" << vec[i] << ") ";
			}
			os << "}\n";
		};

	auto print_vector_float = [&, format](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.vector_float_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			for (std::size_t i = 0; i < block_count; i++) {
				os << "    ";
			}
			os << "�������ͱ���<VectorFloat," << name << "> = {";
			const ScriptVectorFloat& vec = *data.value();
			std::size_t size = vec.size();
			for (std::size_t i = 0; i < size; i++) {
				if (i % format.max_vec_element_per_line == 0) {
					os << "\n";
					for (std::size_t i = 0; i < block_count; i++) {
						os << "    ";
					}
				}
				os << "[" << i << "](" << vec[i] << ") ";
			}
			for (std::size_t i = 0; i < block_count; i++) {
				os << "    ";
			}
			os << "}\n";
		};

	auto print_package = [&, format, this](
		auto func, const std::string& name, const InternalPtr& ptr, std::size_t block_count
		) {
			for (std::size_t i = 0; i < block_count; i++) {
				os << "    ";
			}
			os << "�������ͱ���<Package," << name << "> = {\n";

			std::size_t data;
			memcpy(&data, byte_heap_.data() + ptr.pointer, sizeof(std::size_t));

			const Package& package = package_heap_[data];
			std::size_t size = package.size();
			auto piter = package.cbegin();
			//����
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
				os << "    ";
			}
			os << "}\n";
		};

	auto print_struct = [&, format, this](
		auto func, const std::string& name, const InternalPtr& ptr, std::size_t block_count
		) {
			for (std::size_t i = 0; i < block_count; i++) {
				os << "    ";
			}
			os << "�ṹ�����<Struct," << name << "> = {\n";

			const auto& member_visitor =
				struct_template_container_.find(ptr.type_code).members().get_visitor();

			std::size_t member_count = member_visitor.size(), offset = 0, size = 0;
			for (std::size_t i = 0; i < member_count; i++) {
				size = struct_template_container_.find(member_visitor[i].second->type_code).size();
				func(
					*member_visitor[i].first,
					{ ptr.pointer + offset,member_visitor[i].second->type_code },
					block_count + 1
				);
				offset += size;
			}

			for (std::size_t i = 0; i < block_count; i++) {
				os << "    ";
			}
			os << "}\n";
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

	os << "[" << name_ << "]:\n";
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
	//����
	auto iter = name_space_.find(var_name);
	if (iter != name_space_.end()) {
		return std::nullopt;
	}

	//���������ܺ�������ͬ
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
	const auto& members =
		struct_template_container_.find(type_code).members();
	std::size_t member_count = members.size(), offset = 0, size = 0;
	for (std::size_t i = 0, data_ptr; i < member_count; i++) {
		size = struct_template_container_.find(members[i].type_code).size();
		BasicDataType type = BasicDataType(members[i].type_code);
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
			init_struct_memory(pointer + offset, members[i].type_code);
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