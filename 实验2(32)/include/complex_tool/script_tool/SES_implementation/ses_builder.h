#pragma once

#include "ses_parser.h"

namespace ses {
	//在编译生成中间码时用于查找类型,位置的工具
	//这里面不真实存储数据,只存储数据指针
	class VirtualMemory {
	public:
		struct DataPtr {
			std::size_t pointer;
			std::size_t type_index;//类型在stc中的编号
		};
	private:
		IndexedMap<std::string, DataPtr> virtual_memory_;
	};



}