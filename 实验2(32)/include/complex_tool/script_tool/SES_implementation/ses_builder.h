#pragma once

#include "ses_parser.h"

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



}