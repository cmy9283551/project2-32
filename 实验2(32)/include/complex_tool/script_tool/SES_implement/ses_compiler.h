#pragma once

#include "ses_builder.h"

namespace ses {
	//编译器可以被继承,以适配不同的需求,并允许各部分覆写编译过程
	//初始版本的编译器提供了单线程编译标准SES的实现
	class Compiler {
	public:
		virtual ~Compiler() = default;
	private:
	};
}