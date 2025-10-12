#pragma once

namespace ses {

	//真实存储数据,操作数据的容器
	//只支持对基本类型的操作(因为结构体都会被拆解为基本类型)
	//在执行时使用
	class ScriptStack {
	public:
	private:
	};

	//嵌入到可执行脚本中的虚拟机,用于执行指令集
	class VirtualMachine {

	};
}