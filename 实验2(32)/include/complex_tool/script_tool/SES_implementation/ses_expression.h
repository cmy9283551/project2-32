#pragma once

#include "ses_statement.h"

namespace ses {
	//表达式节点
	class ExpressionNode : public AbstractSyntaxTree {
	public:
		virtual ~ExpressionNode() = default;
	};
}