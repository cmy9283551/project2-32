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
		IndexedMap<std::string, ParameterType> parameter_list;
	};
}