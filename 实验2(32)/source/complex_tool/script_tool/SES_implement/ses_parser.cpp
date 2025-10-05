#include "complex_tool/script_tool/SES_implement/ses_parser.h"

std::vector<std::unique_ptr<SESAbstractSyntaxTree>> SESParser::parse_ses(
	const std::string& script_path,
	const SESCompileDependence& dependence
)const {
	std::vector<std::unique_ptr<SESAbstractSyntaxTree>> asts;
	SESTokenStream token_stream(script_path);
	SESLexer lexer;
	lexer.tokenize(token_stream);
	while()
	return asts;
}

std::unique_ptr<SESAbstractSyntaxTree> SESParser::parse_ses_script(
	SESTokenStream& token_stream,
	const SESCompileDependence& dependence
)const {

	return std::unique_ptr<SESAbstractSyntaxTree>();
}

std::unique_ptr<SESStatementNode> SESRecursiveDescentParser::parse_ses_statement(
	SESTokenStream& token_stream,
	const SESScriptConfig& config
)const {

	return std::unique_ptr<SESStatementNode>();
}

std::unique_ptr<SESStatementNode> SESPrattParser::parse_ses_statement(
	SESTokenStream& token_stream,
	const SESScriptConfig& config
)const {

	return std::unique_ptr<SESStatementNode>();
}