#pragma once

#include "complex_tool/script_tool/variable_manager.h"

namespace ses {
	//�ڱ��������м���ʱ���ڲ�������,λ�õĹ���
	//�����治��ʵ�洢����,ֻ�洢����ָ��
	class VirtualMemory {
	public:
		struct DataPtr {
			std::size_t pointer;
			std::size_t type_index;//������stc�еı��
		};
	private:
		IndexedMap<std::string, DataPtr> virtual_memory_;
	};

	//��ʵ�洢����,�������ݵ�����
	//ֻ֧�ֶԻ������͵Ĳ���(��Ϊ�ṹ�嶼�ᱻ���Ϊ��������)
	class ScriptStack {
	public:
	private:
	};
}