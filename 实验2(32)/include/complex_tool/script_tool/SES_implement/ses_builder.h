#pragma once

#include "ses_parser.h"

namespace ses {
	//中间代码指令集,每条指令包含操作码和操作数
	struct Instruction {
		enum class OpCode {

		};
	};

	class MemoryBlock {
	};

	//在编译时使用,用于计算内存块大小,内存类型,提供名称到内存地址的映射
	class MemoryBlockCreator {

	};

	//嵌入到可执行脚本中的虚拟机,用于执行指令集
	class VirtualMachine {

	};
	//最终生成的可执行脚本,包含指令集,内存块
	//和变量管理器指针(以调用统一变量),函数管理器指针(以调用统一函数)
	class Instance {
	public:

	};
}