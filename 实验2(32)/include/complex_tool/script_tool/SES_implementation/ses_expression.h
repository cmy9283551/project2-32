#pragma once

#include "ses_statement.h"

namespace ses {
	//���ʽ�ڵ�
	class ExpressionNode : public AbstractSyntaxTree {
	public:
		virtual ~ExpressionNode() = default;
	};
}