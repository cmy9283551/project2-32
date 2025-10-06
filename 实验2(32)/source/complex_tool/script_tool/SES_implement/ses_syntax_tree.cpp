#include "complex_tool/script_tool/SES_implement/ses_syntax_tree.h"

namespace ses {
    const SourceLocation& AbstractSyntaxTree::location() const {
        return location_;
    }

    void ScriptNode::visit(ASTVisitor& visitor) {
        //TO DO
    }

    ASTType ScriptNode::type() const {
        return type_;
    }
}