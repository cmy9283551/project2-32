#pragma once

#include "tool/container.hpp"

namespace ses {
	//�������봫���ű��Ĳ���
	//����ֻ��Ϊ��������,�������ʾ�����������б�
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