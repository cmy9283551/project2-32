#pragma once

#include "ses_lexer.h"
#include "ses_module.h"
#include "ses_expression.h"
#include "complex_tool/script_tool/scope_visitor.h"

//��Žű�������Ϣ
struct SESScriptConfig {
	//��������Ĳ���
	SESScriptParameter input,output;

	SESModuleVisitor module_visitor;
	ScopeVisitor scope_visitor;
};

//���ģ��������Ϣ
class SESModuleConfig {
	ScopeVisitor scope_visitor;
};

//��ʾ��������еĻ�����Ϣ
//����һ��ģ��Ľű�����ʱ,��ģ����������ģ��,������Χ
//ͨ��Ҳ�Ǹ�ģ���ܷ��ʵ����Χ
struct SESCompileDependence {
	ScopeVisitor scope_visitor;
	SESModuleVisitor module_visitor;
	SESScriptConfig default_script_config;
	SESModuleConfig default_module_config;
};

class SESExpressionParser {
public:
private:

};

class SESStatementParser {
public:
private:
};

//����AST
//���߳���,��ÿ���������໥����,���ö��������ͬʱ����
//���Զ��ʹ�ý�������ļ�
class SESParser {
public:
	//���ڽ���*.ses�ļ�,���ؽű��е����ж����ű�
	virtual std::vector<std::unique_ptr<SESAbstractSyntaxTree>> parse_ses(
		const std::string& script_path,
		const SESCompileDependence& dependence
	)const;
protected:
	std::unique_ptr<SESAbstractSyntaxTree> parse_ses_script(
		SESTokenStream& token_stream,
		const SESCompileDependence& dependence
	)const;

	virtual std::unique_ptr<SESStatementNode> parse_ses_statement(
		SESTokenStream& token_stream,
		const SESScriptConfig& config
	)const = 0;
};

//ʹ�õݹ��½������﷨������
class SESRecursiveDescentParser : public SESParser {
protected:
	std::unique_ptr<SESStatementNode> parse_ses_statement(
		SESTokenStream& token_stream,
		const SESScriptConfig& config
	)const override;
};

//ʹ��Pratt���������﷨������
class SESPrattParser : public SESParser {
protected:
	std::unique_ptr<SESStatementNode> parse_ses_statement(
		SESTokenStream& token_stream,
		const SESScriptConfig& config
	)const override;
};
