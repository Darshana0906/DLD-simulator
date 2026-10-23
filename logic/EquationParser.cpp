#include "EquationParser.h"
#include <QRegularExpression>

// =========================================================================
// ASTNode convenience constructors
// =========================================================================

std::unique_ptr<ASTNode> ASTNode::makeVariable(const QString &name) {
    auto node = std::make_unique<ASTNode>();
    node->type = ASTNodeType::VARIABLE;
    node->variableName = name;
    return node;
}

std::unique_ptr<ASTNode> ASTNode::makeConst(bool value) {
    auto node = std::make_unique<ASTNode>();
    node->type = value ? ASTNodeType::CONST_1 : ASTNodeType::CONST_0;
    return node;
}

std::unique_ptr<ASTNode> ASTNode::makeUnary(ASTNodeType type, std::unique_ptr<ASTNode> child) {
    auto node = std::make_unique<ASTNode>();
    node->type = type;
    node->children.push_back(std::move(child));
    return node;
}

std::unique_ptr<ASTNode> ASTNode::makeBinary(ASTNodeType type,
                                              std::unique_ptr<ASTNode> left,
                                              std::unique_ptr<ASTNode> right) {
    auto node = std::make_unique<ASTNode>();
    node->type = type;
    node->children.push_back(std::move(left));
    node->children.push_back(std::move(right));
    return node;
}

std::unique_ptr<ASTNode> ASTNode::makeNary(ASTNodeType type,
                                            std::vector<std::unique_ptr<ASTNode>> children) {
    auto node = std::make_unique<ASTNode>();
    node->type = type;
    node->children = std::move(children);
    return node;
}

// =========================================================================
// ParserState helpers
// =========================================================================

static const Token s_endToken{TokenType::END_OF_INPUT, "", -1};

const Token &EquationParser::ParserState::current() const {
    if (pos < static_cast<int>(tokens->size()))
        return (*tokens)[pos];
    return s_endToken;
}

const Token &EquationParser::ParserState::peek() const {
    return current();
}

bool EquationParser::ParserState::atEnd() const {
    return pos >= static_cast<int>(tokens->size()) ||
           (*tokens)[pos].type == TokenType::END_OF_INPUT;
}

void EquationParser::ParserState::advance() {
    if (pos < static_cast<int>(tokens->size()))
        pos++;
}

bool EquationParser::ParserState::match(TokenType type) {
    if (!atEnd() && current().type == type) {
        advance();
        return true;
    }
    return false;
}

// =========================================================================
// Tokenizer
// =========================================================================

std::vector<Token> EquationParser::tokenize(const QString &line, int lineOffset,
                                             QString &error, int &errorPos) {
    std::vector<Token> tokens;
    int i = 0;
    int len = line.length();

    while (i < len) {
        QChar ch = line[i];

        // Skip whitespace
        if (ch.isSpace()) {
            i++;
            continue;
        }

        int tokenStart = lineOffset + i;

        // Single-character operators and delimiters
        if (ch == '(') {
            tokens.push_back({TokenType::LPAREN, "(", tokenStart});
            i++;
        } else if (ch == ')') {
            tokens.push_back({TokenType::RPAREN, ")", tokenStart});
            i++;
        } else if (ch == '+') {
            tokens.push_back({TokenType::OP_OR, "+", tokenStart});
            i++;
        } else if (ch == '*' || ch == '.') {
            tokens.push_back({TokenType::OP_AND, "*", tokenStart});
            i++;
        } else if (ch == '^') {
            tokens.push_back({TokenType::OP_XOR, "^", tokenStart});
            i++;
        } else if (ch == '\'') {
            tokens.push_back({TokenType::OP_NOT, "'", tokenStart});
            i++;
        } else if (ch == '=') {
            tokens.push_back({TokenType::EQUALS, "=", tokenStart});
            i++;
        }
        // Unicode operators: · (U+00B7), ⊕ (U+2295), ⊙ (U+2299)
        else if (ch.unicode() == 0x00B7) {
            tokens.push_back({TokenType::OP_AND, QString::fromUtf8("·"), tokenStart});
            i++;
        } else if (ch.unicode() == 0x2295) {
            tokens.push_back({TokenType::OP_XOR, QString::fromUtf8("⊕"), tokenStart});
            i++;
        } else if (ch.unicode() == 0x2299) {
            tokens.push_back({TokenType::OP_XNOR, QString::fromUtf8("⊙"), tokenStart});
            i++;
        }
        // Variable names: only 'S' (input switches) and 'Y' (LED output / feedback) variables allowed
        else if (ch.isLetter()) {
            int start = i;
            while (i < len && (line[i].isLetterOrNumber() || line[i] == '_')) {
                i++;
            }
            QString name = line.mid(start, i - start);

            if (!name.startsWith('S', Qt::CaseInsensitive) && !name.startsWith('Y', Qt::CaseInsensitive)) {
                error = QString("Invalid variable '%1'. Only 'S' (switches, e.g. S1, S2) and 'Y' (LED outputs, e.g. Y1, Y2) are allowed.").arg(name);
                errorPos = tokenStart;
                return {};
            }

            tokens.push_back({TokenType::VARIABLE, name.toUpper(), tokenStart});
        }
        // Constants: '0' or '1'
        else if (ch == '0') {
            tokens.push_back({TokenType::CONST_0, "0", tokenStart});
            i++;
        } else if (ch == '1') {
            tokens.push_back({TokenType::CONST_1, "1", tokenStart});
            i++;
        }
        // Unknown character
        else {
            error = QString("Unexpected character '%1' at position %2").arg(ch).arg(tokenStart);
            errorPos = tokenStart;
            return {};
        }
    }

    tokens.push_back({TokenType::END_OF_INPUT, "", lineOffset + len});
    return tokens;
}

// =========================================================================
// NAND/NOR Optimization
// =========================================================================

std::unique_ptr<ASTNode> EquationParser::optimizeNot(std::unique_ptr<ASTNode> child) {
    // NOT(AND(...)) → NAND(...)
    if (child->type == ASTNodeType::AND) {
        child->type = ASTNodeType::NAND;
        return child;
    }
    // NOT(OR(...)) → NOR(...)
    if (child->type == ASTNodeType::OR) {
        child->type = ASTNodeType::NOR;
        return child;
    }
    // Otherwise keep as NOT
    return ASTNode::makeUnary(ASTNodeType::NOT, std::move(child));
}

// =========================================================================
// Recursive Descent Parser
// =========================================================================

// atom ::= VARIABLE | CONST_0 | CONST_1 | '(' expr ')'
std::unique_ptr<ASTNode> EquationParser::parseAtom(ParserState &state, QString &error) {
    const Token &tok = state.current();

    if (tok.type == TokenType::VARIABLE) {
        auto node = ASTNode::makeVariable(tok.text);
        state.advance();
        return node;
    }
    if (tok.type == TokenType::CONST_0) {
        auto node = ASTNode::makeConst(false);
        state.advance();
        return node;
    }
    if (tok.type == TokenType::CONST_1) {
        auto node = ASTNode::makeConst(true);
        state.advance();
        return node;
    }
    if (tok.type == TokenType::LPAREN) {
        state.advance(); // consume '('
        auto expr = parseExpression(state, error);
        if (!expr) return nullptr;

        if (!state.match(TokenType::RPAREN)) {
            error = QString("Expected ')' at position %1").arg(state.current().position);
            return nullptr;
        }
        return expr;
    }

    error = QString("Unexpected token '%1' at position %2").arg(tok.text).arg(tok.position);
    return nullptr;
}

// postfix ::= atom ( "'" )*
std::unique_ptr<ASTNode> EquationParser::parsePostfix(ParserState &state, QString &error) {
    auto node = parseAtom(state, error);
    if (!node) return nullptr;

    // Handle postfix NOT (complement) — can be chained: A'' = A
    while (!state.atEnd() && state.current().type == TokenType::OP_NOT) {
        state.advance(); // consume "'"
        node = optimizeNot(std::move(node));
    }
    return node;
}

// and_expr ::= postfix ( ('·'|'*') postfix )*
// Also handles implicit AND via juxtaposition when next token is VARIABLE, CONST, or LPAREN
std::unique_ptr<ASTNode> EquationParser::parseAndExpr(ParserState &state, QString &error) {
    auto left = parsePostfix(state, error);
    if (!left) return nullptr;

    std::vector<std::unique_ptr<ASTNode>> operands;
    operands.push_back(std::move(left));

    while (!state.atEnd()) {
        TokenType tt = state.current().type;

        // Explicit AND operator
        if (tt == TokenType::OP_AND) {
            state.advance(); // consume operator
            auto right = parsePostfix(state, error);
            if (!right) return nullptr;
            operands.push_back(std::move(right));
            continue;
        }

        // Implicit AND (juxtaposition): next token starts an atom
        // e.g. "AB" or "A(B+C)" — only when no explicit operator separates them
        if (tt == TokenType::VARIABLE || tt == TokenType::CONST_0 ||
            tt == TokenType::CONST_1 || tt == TokenType::LPAREN) {
            auto right = parsePostfix(state, error);
            if (!right) return nullptr;
            operands.push_back(std::move(right));
            continue;
        }

        break;
    }

    if (operands.size() == 1)
        return std::move(operands[0]);

    return ASTNode::makeNary(ASTNodeType::AND, std::move(operands));
}

// xor_expr ::= and_expr ( ('⊕'|'^') and_expr )* | and_expr ( '⊙' and_expr )*
std::unique_ptr<ASTNode> EquationParser::parseXorExpr(ParserState &state, QString &error) {
    auto left = parseAndExpr(state, error);
    if (!left) return nullptr;

    while (!state.atEnd()) {
        TokenType tt = state.current().type;

        if (tt == TokenType::OP_XOR) {
            state.advance();
            auto right = parseAndExpr(state, error);
            if (!right) return nullptr;
            left = ASTNode::makeBinary(ASTNodeType::XOR, std::move(left), std::move(right));
        } else if (tt == TokenType::OP_XNOR) {
            state.advance();
            auto right = parseAndExpr(state, error);
            if (!right) return nullptr;
            left = ASTNode::makeBinary(ASTNodeType::XNOR, std::move(left), std::move(right));
        } else {
            break;
        }
    }

    return left;
}

// or_expr ::= xor_expr ( '+' xor_expr )*
std::unique_ptr<ASTNode> EquationParser::parseOrExpr(ParserState &state, QString &error) {
    auto left = parseXorExpr(state, error);
    if (!left) return nullptr;

    std::vector<std::unique_ptr<ASTNode>> operands;
    operands.push_back(std::move(left));

    while (!state.atEnd() && state.current().type == TokenType::OP_OR) {
        state.advance(); // consume '+'
        auto right = parseXorExpr(state, error);
        if (!right) return nullptr;
        operands.push_back(std::move(right));
    }

    if (operands.size() == 1)
        return std::move(operands[0]);

    return ASTNode::makeNary(ASTNodeType::OR, std::move(operands));
}

// expr ::= or_expr
std::unique_ptr<ASTNode> EquationParser::parseExpression(ParserState &state, QString &error) {
    return parseOrExpr(state, error);
}

// =========================================================================
// Public API: Parse multi-line input
// =========================================================================

ParseResult EquationParser::parse(const QString &input) {
    ParseResult result;
    result.success = true;

    QStringList lines = input.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        result.success = false;
        result.errorMessage = "No equation provided. Please enter a Boolean equation.";
        return result;
    }

    int lineOffset = 0;
    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) {
            lineOffset += rawLine.length() + 1;
            continue;
        }

        // Try to extract output name: "Y = expr", "Y1 = expr", "y1 - expr", "Y1 -> expr"
        QString outputName;
        QString exprPart = line;

        int eqIdx = line.indexOf('=');
        int sepLen = 1;
        if (eqIdx < 0) {
            eqIdx = line.indexOf("->");
            if (eqIdx >= 0) {
                sepLen = 2;
            } else {
                // Check for '-' separator (e.g. "y1 - s1.y1")
                eqIdx = line.indexOf('-');
                sepLen = 1;
            }
        }

        if (eqIdx > 0) {
            QString lhs = line.left(eqIdx).trimmed();
            QRegularExpression identRe("^[A-Za-z][A-Za-z0-9_]*$");
            if (identRe.match(lhs).hasMatch()) {
                outputName = lhs.toUpper();
                exprPart = line.mid(eqIdx + sepLen).trimmed();
            }
        }

        // Validate output name: must start with 'Y' (corresponds to output element / LED)
        if (!outputName.isEmpty()) {
            if (!outputName.startsWith('Y')) {
                result.success = false;
                result.errorMessage = QString("Equation output '%1' must start with 'Y' (e.g. Y1, Y2). 'S' is reserved for input switches.").arg(outputName);
                result.errorPosition = lineOffset;
                return result;
            }
        } else {
            outputName = QString("Y%1").arg(result.equations.size() + 1);
        }

        // Prevent duplicate output definitions
        for (const auto &existingEq : result.equations) {
            if (existingEq.outputName == outputName) {
                result.success = false;
                result.errorMessage = QString("Duplicate output '%1'. Each equation must define a unique 'Y' output.").arg(outputName);
                result.errorPosition = lineOffset;
                return result;
            }
        }

        if (exprPart.isEmpty()) {
            result.success = false;
            result.errorMessage = QString("Empty expression for '%1' on line: %2").arg(outputName).arg(line);
            result.errorPosition = lineOffset + (eqIdx >= 0 ? eqIdx + sepLen : 0);
            return result;
        }

        // Tokenize the expression part
        QString tokenError;
        int tokenErrorPos = -1;
        int exprOffset = lineOffset + (eqIdx >= 0 ? eqIdx + sepLen : 0);

        // Adjust for whitespace between separator and expression
        if (eqIdx >= 0) {
            int skip = 0;
            QString afterSep = line.mid(eqIdx + sepLen);
            while (skip < afterSep.length() && afterSep[skip].isSpace()) skip++;
            exprOffset = lineOffset + eqIdx + sepLen + skip;
        }

        auto tokens = tokenize(exprPart, exprOffset, tokenError, tokenErrorPos);
        if (!tokenError.isEmpty()) {
            result.success = false;
            result.errorMessage = tokenError;
            result.errorPosition = tokenErrorPos;
            return result;
        }

        // Parse the expression
        ParserState state;
        state.tokens = &tokens;
        state.pos = 0;

        QString parseError;
        auto ast = parseExpression(state, parseError);

        if (!ast) {
            result.success = false;
            result.errorMessage = parseError.isEmpty()
                ? QString("Failed to parse expression: %1").arg(exprPart)
                : parseError;
            result.errorPosition = state.current().position;
            return result;
        }

        // Ensure all tokens were consumed
        if (!state.atEnd()) {
            result.success = false;
            result.errorMessage = QString("Unexpected token '%1' at position %2")
                .arg(state.current().text)
                .arg(state.current().position);
            result.errorPosition = state.current().position;
            return result;
        }

        ParsedEquation eq;
        eq.outputName = outputName;
        eq.root = std::move(ast);
        result.equations.push_back(std::move(eq));

        lineOffset += rawLine.length() + 1;
    }

    if (result.equations.empty()) {
        result.success = false;
        result.errorMessage = "No valid equations found in input.";
    }

    return result;
}
