#include "complex_tool/script_tool/variable_manager.h"

#include "complex_tool/script_tool/function_manager.h"

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
	os << "这个类不支持输出结构体数据\n";
}

void VariableManager::print_heap_data(std::ostream& os) const {
	os << "[" << name_ << "]:\n";
	os << "这个类不支持输出堆数据\n";
}

const std::string& VariableManager::name()const {
	return name_;
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

std::optional<StructProxy> BasicVariableManager::find_type(
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
			if (that_type->is_equal(type_iter.second()) == false) {
				return true;
			}
		}
	}
	return false;
}

bool BasicVariableManager::has_name_conflict(const FunctionManager& fm) const {
	return fm.has_name_conflict(*this);
}

void BasicVariableManager::print_struct_data(std::ostream& os) const {
	print_header(os, "[" + name_ + "]:结构体");
	os << struct_template_container_;
	print_footer(os, "[" + name_ + "]:结构体");
}

void BasicVariableManager::print_heap_data(std::ostream& os)const {
	struct PrintFormat {
		std::size_t max_vec_element_per_line = 6;
	}format;

	auto print_block = [&](std::size_t block_count) {
		for (std::size_t i = 0; i < block_count; i++) {
			os << "    ";
		}
		};

	auto print_int = [&](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.int_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			print_block(block_count);
			os << "基础类型变量<Int," << name << "> = ["
				<< data.value() << "]\n";
		};

	auto print_float = [&](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.float_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			print_block(block_count);
			os << "基础类型变量<Float," << name << "> = ["
				<< data.value() << "]\n";
		};

	auto print_char = [&](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.char_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			print_block(block_count);
			os << "基础类型变量<Char," << name << "> = ["
				<< data.value() << "]\n";
		};

	auto print_string = [&](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.string_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			print_block(block_count);
			os << "基础类型变量<String," << name << "> = ["
				<< *data.value() << "]\n";
		};

	auto print_vector_int = [&, format](
		const std::string& name, const ConstDataPtr& ptr, std::size_t block_count
		) {
			auto data = ptr.vector_int_data();
			if (data == std::nullopt) {
				ASSERT(false);
			}
			print_block(block_count);
			os
				<< "基础类型变量<VectorInt," << name << "> = {";
			const ScriptVectorInt& vec = *data.value();
			std::size_t size = vec.size();
			for (std::size_t i = 0; i < size; i++) {
				if (i % format.max_vec_element_per_line == 0) {
					os << "\n";
					print_block(block_count);
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
			print_block(block_count);
			os << "基础类型变量<VectorFloat," << name << "> = {";
			const ScriptVectorFloat& vec = *data.value();
			std::size_t size = vec.size();
			for (std::size_t i = 0; i < size; i++) {
				if (i % format.max_vec_element_per_line == 0) {
					os << "\n";
					print_block(block_count);
				}
				os << "[" << i << "](" << vec[i] << ") ";
			}
			print_block(block_count);
			os << "}\n";
		};

	auto print_package = [&, format, this](
		auto func, const std::string& name, const InternalPtr& ptr, std::size_t block_count
		) {
			print_block(block_count);
			os << "基础类型变量<Package," << name << "> = {\n";

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
			print_block(block_count);
			os << "}\n";
		};

	auto print_struct = [&, format, this](
		auto func, const std::string& name, const InternalPtr& ptr, std::size_t block_count
		) {
			print_block(block_count);
			os << "结构体变量<Struct," << name << "> = {\n";

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
			print_block(block_count);
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

	print_header(os, "[" + name_ + "]:堆数据");
	auto visitors = name_space_.get_visitor();
	std::size_t size = visitors.size();
	for (std::size_t i = 0; i < size; i++) {
		print_data(*visitors[i].first, *visitors[i].second, 0);
	}
	print_footer(os, "[" + name_ + "]:堆数据");
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