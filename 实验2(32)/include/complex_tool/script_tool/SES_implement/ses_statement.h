#pragma once

//语法解析,构建语法树

class SESAbstractSyntaxTree {
public:
	virtual ~SESAbstractSyntaxTree() = default;
};

//脚本库中的一个独立脚本
class SESScriptNode : public SESAbstractSyntaxTree {

};

class SESStatementNode : public SESAbstractSyntaxTree {
public:
	virtual ~SESStatementNode() = default;
};

//脚本块,对应{...},存放语句
class SESBlockStmtNode : public SESStatementNode {
public:
private:
};

//表达式,对应分号结尾的语句
class SESExpressionStmtNode : public SESStatementNode {

};

//声明,对应变量声明语句
class SESDeclarationStmtNode : public SESStatementNode {

};

//if语句
class SESIfStmtNode : public SESStatementNode {

};

//while语句
class SESWhileStmtNode : public SESStatementNode {

};

//for语句
class SESForStmtNode : public SESStatementNode {

};

//break语句
class SESBreakStmtNode : public SESStatementNode {
};

//continue语句
class SESContinueStmtNode : public SESStatementNode {

};

//return语句
class SESReturnStmtNode : public SESStatementNode {

};