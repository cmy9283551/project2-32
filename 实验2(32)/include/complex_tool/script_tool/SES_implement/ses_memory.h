#pragma once

#include "complex_tool/script_tool/variable_manager.h"

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

	//真实存储数据,操作数据的容器
	//只支持对基本类型的操作(因为结构体都会被拆解为基本类型)
	class ScriptStack {
	public:
	private:
	};
}