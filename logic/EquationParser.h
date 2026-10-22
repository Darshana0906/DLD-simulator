#ifndef EQUATIONPARSER_H
#define EQUATIONPARSER_H

#include <QString>
#include <QStringList>
#include <vector>
#include <memory>

// =========================================================================
// AST Node Types — represents the parsed structure of a Boolean equation
// =========================================================================
enum class ASTNodeType {
    VARIABLE,   // leaf: "A", "B", "S1" etc.
    CONST_0,    // leaf: logic 0
    CONST_1,    // leaf: logic 1
    AND,        // binary+: children are AND operands
    OR,         // binary+: children are OR operands
    NOT,        // unary: single child (complement)
    NAND,       // optimized: NOT(AND(...))
    NOR,        // optimized: NOT(OR(...))
    XOR,        // binary: two children
    XNOR        // binary: two children
};

struct ASTNode {
    ASTNodeType type;
    QString variableName;  // only meaningful for VARIABLE nodes
    std::vector<std::unique_ptr<ASTNode>> children;

    // Convenience constructors
    static std::unique_ptr<ASTNode> makeVariable(const QString &name);
    static std::unique_ptr<ASTNode> makeConst(bool value);
    static std::unique_ptr<ASTNode> makeUnary(ASTNodeType type, std::unique_ptr<ASTNode> child);
    static std::unique_ptr<ASTNode> makeBinary(ASTNodeType type,
                                                std::unique_ptr<ASTNode> left,
                                                std::unique_ptr<ASTNode> right);
    static std::unique_ptr<ASTNode> makeNary(ASTNodeType type,
                                              std::vector<std::unique_ptr<ASTNode>> children);
};

// =========================================================================
// Parsed Equation: one "Y = expression" line
// =========================================================================
struct ParsedEquation {
    QString outputName;                // e.g. "Y1"
    std::unique_ptr<ASTNode> root;     // AST root of the expression
};

// =========================================================================
// Parse Result: returned by the parser
// =========================================================================
struct ParseResult {
    bool success = false;
    QString errorMessage;
    int errorPosition = -1;            // character position of error
    std::vector<ParsedEquation> equations;
};

// =========================================================================
// Token Types for the lexer
// =========================================================================
enum class TokenType {
    VARIABLE,       // A, B, S1, X2, etc.
    CONST_0,        // literal 0
    CONST_1,        // literal 1
    OP_AND,         // · or *
    OP_OR,          // +
    OP_XOR,         // ⊕ or ^
    OP_XNOR,        // ⊙
    OP_NOT,         // ' (postfix complement)
    LPAREN,         // (
    RPAREN,         // )
    EQUALS,         // =
    END_OF_INPUT
};

struct Token {
    TokenType type;
    QString text;
    int position;    // position in original string
};

// =========================================================================
// EquationParser: Tokenizer + Recursive Descent Parser
// =========================================================================
class EquationParser {
public:
    // Parse one or more equations (separated by newlines)
    // Each line: "Y = expression" or just "expression"
    static ParseResult parse(const QString &input);

private:
    // Tokenizer
    static std::vector<Token> tokenize(const QString &line, int lineOffset, QString &error, int &errorPos);

    // Recursive descent parser
    // Grammar (highest to lowest precedence):
    //   atom       ::= VARIABLE | CONST | '(' expr ')'
    //   postfix    ::= atom ( "'" )*
    //   and_expr   ::= postfix ( ('·'|'*') postfix )*
    //   xor_expr   ::= and_expr ( ('⊕'|'^') and_expr )*  |  and_expr ( '⊙' and_expr )*
    //   or_expr    ::= xor_expr ( '+' xor_expr )*
    //   expr       ::= or_expr

    struct ParserState {
        const std::vector<Token> *tokens = nullptr;
        int pos = 0;

        const Token &current() const;
        const Token &peek() const;
        bool atEnd() const;
        void advance();
        bool match(TokenType type);
    };

    static std::unique_ptr<ASTNode> parseExpression(ParserState &state, QString &error);
    static std::unique_ptr<ASTNode> parseOrExpr(ParserState &state, QString &error);
    static std::unique_ptr<ASTNode> parseXorExpr(ParserState &state, QString &error);
    static std::unique_ptr<ASTNode> parseAndExpr(ParserState &state, QString &error);
    static std::unique_ptr<ASTNode> parsePostfix(ParserState &state, QString &error);
    static std::unique_ptr<ASTNode> parseAtom(ParserState &state, QString &error);

    // NAND/NOR optimization: if NOT wraps an AND/OR, merge into NAND/NOR
    static std::unique_ptr<ASTNode> optimizeNot(std::unique_ptr<ASTNode> child);
};

#endif // EQUATIONPARSER_H
