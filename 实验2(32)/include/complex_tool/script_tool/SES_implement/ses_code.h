#pragma once

#include "tool/container.hpp"

namespace ses {
	//描述传入传出脚本的参数
	//参数只能为基本类型,这里仅表示参数的类型列表
	struct ScriptParameter {
		enum class ParameterType {
			Int = 0,
			Float = 1,
			Char = 2,
			String = 3,
			VectorInt = 4,
			VectorFloat = 5,
			Package = 6
		};

		static const std::string& parameter_type_to_string(ParameterType type);

		friend std::ostream& operator<<(std::ostream& os, const ScriptParameter& parameter);

		IndexedMap<std::string, ParameterType> parameters;
	};
}