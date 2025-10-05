#pragma once

#include "ses_statement.h"

//表达式节点
class SESExpressionNode : public SESAbstractSyntaxTree {
public:
	virtual ~SESExpressionNode() = default;
};