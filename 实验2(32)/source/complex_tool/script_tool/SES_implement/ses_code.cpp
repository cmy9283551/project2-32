#include "complex_tool/script_tool/SES_implement/ses_code.h"

#include <unordered_map>

#include "tool/script_tool/script_debug_tool.h"

namespace ses {
	const std::string& ScriptParameter::parameter_type_to_string(ParameterType type) {
		static const std::unordered_map<ParameterType, std::string> parameter_to_string_map = {
			{ParameterType::Int,"int" },
			{ParameterType::Float,"float"},
			{ParameterType::Char,"char"},
			{ParameterType::String,"string"},
			{ParameterType::VectorInt,"vector_int"},
			{ParameterType::VectorFloat,"vector_float"},
			{ParameterType::Package,"package"}
		};
		auto iter = parameter_to_string_map.find(type);
		if (iter == parameter_to_string_map.cend()) {
			SCRIPT_CERR << "Error: Unknown token type." << std::endl;
			ASSERT(false);
		}
		return iter->second;
	}

	std::ostream& ses::operator<<(std::ostream& os, const ScriptParameter& parameter) {
		auto visitors = parameter.parameters.get_visitor();
		std::size_t size = visitors.size();
		for (std::size_t i = 0; i < size; i++) {
			os << "<" << *visitors[i].first << ","
				<< ScriptParameter::parameter_type_to_string(*visitors[i].second) << ">\n";
		}
		return os;
	}
}
