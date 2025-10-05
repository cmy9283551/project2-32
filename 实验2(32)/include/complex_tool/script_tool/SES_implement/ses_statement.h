#pragma once

struct SESScriptConfig;
struct SESModuleConfig;

//�﷨����,�����﷨��

class SESAbstractSyntaxTree {
public:
	virtual ~SESAbstractSyntaxTree() = default;
};

class SESStatementNode : public SESAbstractSyntaxTree {
public:
	virtual ~SESStatementNode() = default;
};

//�ű����е�һ�������ű�
class SESScriptNode : public SESAbstractSyntaxTree {
public:
private:
	std::vector<std::unique_ptr<SESStatementNode>> statements_;
	std::unique_ptr<SESScriptConfig> config_;
	std::string script_name_;
};

//�ű���,��Ӧ{...},������
class SESBlockStmtNode : public SESStatementNode {
public:
private:
};

//���ʽ,��Ӧ�ֺŽ�β�����
class SESExpressionStmtNode : public SESStatementNode {

};

//����,��Ӧ�����������
class SESDeclarationStmtNode : public SESStatementNode {

};

//if���
class SESIfStmtNode : public SESStatementNode {

};

//while���
class SESWhileStmtNode : public SESStatementNode {

};

//for���
class SESForStmtNode : public SESStatementNode {

};

//break���
class SESBreakStmtNode : public SESStatementNode {
};

//continue���
class SESContinueStmtNode : public SESStatementNode {

};

//return���
class SESReturnStmtNode : public SESStatementNode {

};