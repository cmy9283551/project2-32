#pragma once

//�﷨����,�����﷨��

class SESAbstractSyntaxTree {
public:
	virtual ~SESAbstractSyntaxTree() = default;
};

//�ű����е�һ�������ű�
class SESScriptNode : public SESAbstractSyntaxTree {

};

class SESStatementNode : public SESAbstractSyntaxTree {
public:
	virtual ~SESStatementNode() = default;
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