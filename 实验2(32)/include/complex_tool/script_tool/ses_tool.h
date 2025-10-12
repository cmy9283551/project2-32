#pragma once

#include <memory>

//包含的ses文件不含关于编译器的头文件,以减少编译依赖
//仅包含使用脚本工具所需的头文件
//且保证以下文件不包括任何编译器相关的内容
#include "SES_implementation/ses_script.h"
#include "SES_implementation/ses_virtual_machine.h"

namespace ses{
	//pimpl前向声明
	class Compiler;
	struct CompilerDependence;
}

using SESModule = ses::Module;
using SESScript = ses::ScriptInstance;
using SESVirtualMachine = ses::VirtualMachine;
using SESCompilerDependence = ses::CompilerDependence;

class SESCompiler {
public:
	SESCompiler(const SESCompilerDependence& dependence);
	~SESCompiler() = default;


private:
	std::unique_ptr<ses::Compiler> impl_ = nullptr;
};