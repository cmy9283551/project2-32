#pragma once

#include <memory>

//������ses�ļ��������ڱ�������ͷ�ļ�,�Լ��ٱ�������
//������ʹ�ýű����������ͷ�ļ�
//�ұ�֤�����ļ��������κα�������ص�����
#include "SES_implementation/ses_script.h"
#include "SES_implementation/ses_virtual_machine.h"

namespace ses{
	//pimplǰ������
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