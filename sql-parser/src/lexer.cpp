// lexer.cpp
#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <algorithm>

// ─────────────────────────────────────────────────────────────
//  Static keyword table  (all keys stored in UPPER CASE)
// ─────────────────────────────────────────────────────────────
const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    // DML
    {"SELECT",        TokenType::KW_SELECT},
    {"FROM",          TokenType::KW_FROM},
    {"WHERE",         TokenType::KW_WHERE},
    {"INSERT",        TokenType::KW_INSERT},
    {"INTO",          TokenType::KW_INTO},
    {"VALUES",        TokenType::KW_VALUES},
    {"UPDATE",        TokenType::KW_UPDATE},
    {"SET",           TokenType::KW_SET},
    {"DELETE",        TokenType::KW_DELETE},
    {"DISTINCT",      TokenType::KW_DISTINCT},
    {"AS",            TokenType::KW_AS},

    // DDL
    {"CREATE",        TokenType::KW_CREATE},
    {"TABLE",         TokenType::KW_TABLE},
    {"DROP",          TokenType::KW_DROP},
    {"IF",            TokenType::KW_IF},
    {"EXISTS",        TokenType::KW_EXISTS},
    {"NOT",           TokenType::KW_NOT},

    // Join
    {"JOIN",          TokenType::KW_JOIN},
    {"INNER",         TokenType::KW_INNER},
    {"LEFT",          TokenType::KW_LEFT},
    {"RIGHT",         TokenType::KW_RIGHT},
    {"FULL",          TokenType::KW_FULL},
    {"OUTER",         TokenType::KW_OUTER},
    {"ON",            TokenType::KW_ON},

    // Clauses
    {"ORDER",         TokenType::KW_ORDER},
    {"BY",            TokenType::KW_BY},
    {"GROUP",         TokenType::KW_GROUP},
    {"HAVING",        TokenType::KW_HAVING},
    {"LIMIT",         TokenType::KW_LIMIT},
    {"OFFSET",        TokenType::KW_OFFSET},
    {"ASC",           TokenType::KW_ASC},
    {"DESC",          TokenType::KW_DESC},

    // Conditions
    {"AND",           TokenType::KW_AND},
    {"OR",            TokenType::KW_OR},
    {"IN",            TokenType::KW_IN},
    {"LIKE",          TokenType::KW_LIKE},
    {"BETWEEN",       TokenType::KW_BETWEEN},
    {"IS",            TokenType::KW_IS},
    {"ALL",           TokenType::KW_ALL},
    {"ANY",           TokenType::KW_ANY},

    // Values
    {"NULL",          TokenType::KW_NULL},
    {"TRUE",          TokenType::KW_TRUE},
    {"FALSE",         TokenType::KW_FALSE},
    {"DEFAULT",       TokenType::KW_DEFAULT},

    // Constraints
    {"PRIMARY",       TokenType::KW_PRIMARY},
    {"KEY",           TokenType::KW_KEY},
    {"FOREIGN",       TokenType::KW_FOREIGN},
    {"REFERENCES",    TokenType::KW_REFERENCES},
    {"UNIQUE",        TokenType::KW_UNIQUE},
    {"AUTO_INCREMENT",TokenType::KW_AUTO_INCREMENT},
    {"CHECK",         TokenType::KW_CHECK},

    // Data types
    {"INT",           TokenType::KW_INT},
    {"INTEGER",       TokenType::KW_INTEGER},
    {"BIGINT",        TokenType::KW_BIGINT},
    {"SMALLINT",      TokenType::KW_SMALLINT},
    {"VARCHAR",       TokenType::KW_VARCHAR},
    {"CHAR",          TokenType::KW_CHAR},
    {"TEXT",          TokenType::KW_TEXT},
    {"CLOB",          TokenType::KW_CLOB},
    {"FLOAT",         TokenType::KW_FLOAT},
    {"DOUBLE",        TokenType::KW_DOUBLE},
    {"DECIMAL",       TokenType::KW_DECIMAL},
    {"BOOLEAN",       TokenType::KW_BOOLEAN},
    {"BOOL",          TokenType::KW_BOOL},
    {"DATE",          TokenType::KW_DATE},
    {"DATETIME",      TokenType::KW_DATETIME},
    {"TIMESTAMP",     TokenType::KW_TIMESTAMP},

    // Aggregates
    {"COUNT",         TokenType::KW_COUNT},
    {"SUM",           TokenType::KW_SUM},
    {"AVG",           TokenType::KW_AVG},
    {"MAX",           TokenType::KW_MAX},
    {"MIN",           TokenType::KW_MIN},
};

// ─────────────────────────────────────────────────────────────
//  Token::typeToString()  — defined here to avoid a separate .cpp
// ─────────────────────────────────────────────────────────────
std::string Token::typeToString() const {
    switch (type) {
        case TokenType::INTEGER_LITERAL: return "INTEGER_LITERAL";
        case TokenType::FLOAT_LITERAL:   return "FLOAT_LITERAL";
        case TokenType::STRING_LITERAL:  return "STRING_LITERAL";
        case TokenType::IDENTIFIER:      return "IDENTIFIER";
        case TokenType::KW_SELECT:       return "SELECT";
        case TokenType::KW_FROM:         return "FROM";
        case TokenType::KW_WHERE:        return "WHERE";
        case TokenType::KW_INSERT:       return "INSERT";
        case TokenType::KW_INTO:         return "INTO";
        case TokenType::KW_VALUES:       return "VALUES";
        case TokenType::KW_UPDATE:       return "UPDATE";
        case TokenType::KW_SET:          return "SET";
        case TokenType::KW_DELETE:       return "DELETE";
        case TokenType::KW_DISTINCT:     return "DISTINCT";
        case TokenType::KW_AS:           return "AS";
        case TokenType::KW_CREATE:       return "CREATE";
        case TokenType::KW_TABLE:        return "TABLE";
        case TokenType::KW_DROP:         return "DROP";
        case TokenType::KW_IF:           return "IF";
        case TokenType::KW_EXISTS:       return "EXISTS";
        case TokenType::KW_NOT:          return "NOT";
        case TokenType::KW_JOIN:         return "JOIN";
        case TokenType::KW_INNER:        return "INNER";
        case TokenType::KW_LEFT:         return "LEFT";
        case TokenType::KW_RIGHT:        return "RIGHT";
        case TokenType::KW_FULL:         return "FULL";
        case TokenType::KW_OUTER:        return "OUTER";
        case TokenType::KW_ON:           return "ON";
        case TokenType::KW_ORDER:        return "ORDER";
        case TokenType::KW_BY:           return "BY";
        case TokenType::KW_GROUP:        return "GROUP";
        case TokenType::KW_HAVING:       return "HAVING";
        case TokenType::KW_LIMIT:        return "LIMIT";
        case TokenType::KW_OFFSET:       return "OFFSET";
        case TokenType::KW_ASC:          return "ASC";
        case TokenType::KW_DESC:         return "DESC";
        case TokenType::KW_AND:          return "AND";
        case TokenType::KW_OR:           return "OR";
        case TokenType::KW_IN:           return "IN";
        case TokenType::KW_LIKE:         return "LIKE";
        case TokenType::KW_BETWEEN:      return "BETWEEN";
        case TokenType::KW_IS:           return "IS";
        case TokenType::KW_ALL:          return "ALL";
        case TokenType::KW_ANY:          return "ANY";
        case TokenType::KW_NULL:         return "NULL";
        case TokenType::KW_TRUE:         return "TRUE";
        case TokenType::KW_FALSE:        return "FALSE";
        case TokenType::KW_DEFAULT:      return "DEFAULT";
        case TokenType::KW_PRIMARY:      return "PRIMARY";
        case TokenType::KW_KEY:          return "KEY";
        case TokenType::KW_FOREIGN:      return "FOREIGN";
        case TokenType::KW_REFERENCES:   return "REFERENCES";
        case TokenType::KW_UNIQUE:       return "UNIQUE";
        case TokenType::KW_AUTO_INCREMENT:return "AUTO_INCREMENT";
        case TokenType::KW_CHECK:        return "CHECK";
        case TokenType::KW_INT:          return "INT";
        case TokenType::KW_INTEGER:      return "INTEGER";
        case TokenType::KW_BIGINT:       return "BIGINT";
        case TokenType::KW_SMALLINT:     return "SMALLINT";
        case TokenType::KW_VARCHAR:      return "VARCHAR";
        case TokenType::KW_CHAR:         return "CHAR";
        case TokenType::KW_TEXT:         return "TEXT";
        case TokenType::KW_CLOB:         return "CLOB";
        case TokenType::KW_FLOAT:        return "FLOAT";
        case TokenType::KW_DOUBLE:       return "DOUBLE";
        case TokenType::KW_DECIMAL:      return "DECIMAL";
        case TokenType::KW_BOOLEAN:      return "BOOLEAN";
        case TokenType::KW_BOOL:         return "BOOL";
        case TokenType::KW_DATE:         return "DATE";
        case TokenType::KW_DATETIME:     return "DATETIME";
        case TokenType::KW_TIMESTAMP:    return "TIMESTAMP";
        case TokenType::KW_COUNT:        return "COUNT";
        case TokenType::KW_SUM:          return "SUM";
        case TokenType::KW_AVG:          return "AVG";
        case TokenType::KW_MAX:          return "MAX";
        case TokenType::KW_MIN:          return "MIN";
        case TokenType::OP_EQ:           return "=";
        case TokenType::OP_NEQ:          return "!=";
        case TokenType::OP_LT:           return "<";
        case TokenType::OP_GT:           return ">";
        case TokenType::OP_LTE:          return "<=";
        case TokenType::OP_GTE:          return ">=";
        case TokenType::OP_PLUS:         return "+";
        case TokenType::OP_MINUS:        return "-";
        case TokenType::OP_STAR:         return "*";
        case TokenType::OP_SLASH:        return "/";
        case TokenType::OP_PERCENT:      return "%";
        case TokenType::LPAREN:          return "(";
        case TokenType::RPAREN:          return ")";
        case TokenType::COMMA:           return ",";
        case TokenType::SEMICOLON:       return ";";
        case TokenType::DOT:             return ".";
        case TokenType::END_OF_FILE:     return "EOF";
        case TokenType::UNKNOWN:         return "UNKNOWN";
        default:                         return "?";
    }
}

// ─────────────────────────────────────────────────────────────
//  Lexer implementation
// ─────────────────────────────────────────────────────────────

Lexer::Lexer(const std::string& source, ErrorReporter& reporter)
    : source_(source), pos_(0), line_(1), column_(1), reporter_(reporter) {}

// ── Character navigation ──────────────────────────────────────

char Lexer::current() const {
    return isAtEnd() ? '\0' : source_[pos_];
}

char Lexer::peek(int offset) const {
    int idx = pos_ + offset;
    return (idx < 0 || idx >= static_cast<int>(source_.size())) ? '\0' : source_[idx];
}

char Lexer::advance() {
    char c = source_[pos_++];
    if (c == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return c;
}

bool Lexer::isAtEnd() const {
    return pos_ >= static_cast<int>(source_.size());
}

// ── Character classifiers ─────────────────────────────────────

bool Lexer::isDigit(char c)        const { return c >= '0' && c <= '9'; }
bool Lexer::isAlpha(char c)        const { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }
bool Lexer::isAlphaNumeric(char c) const { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }

std::string Lexer::toUpper(const std::string& s) const {
    std::string result = s;
    for (char& c : result) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return result;
}

// ── Skip whitespace and SQL comments ─────────────────────────

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = current();

        // Plain whitespace
        if (std::isspace(static_cast<unsigned char>(c))) {
            advance();
            continue;
        }

        // Single-line comment:  -- ...
        if (c == '-' && peek() == '-') {
            while (!isAtEnd() && current() != '\n') advance();
            continue;
        }

        // Multi-line comment:  /* ... */
        if (c == '/' && peek() == '*') {
            advance(); advance(); // consume '/' and '*'
            while (!isAtEnd()) {
                if (current() == '*' && peek() == '/') {
                    advance(); advance(); // consume '*' and '/'
                    break;
                }
                advance();
            }
            continue;
        }

        break; // non-whitespace, non-comment
    }
}

// ── Read an identifier or keyword ────────────────────────────

Token Lexer::readIdentifierOrKeyword() {
    int startLine = line_, startCol = column_;
    std::string lexeme;

    while (!isAtEnd() && isAlphaNumeric(current())) {
        lexeme += advance();
    }

    // Keyword lookup is case-insensitive
    std::string upper = toUpper(lexeme);
    auto it = keywords_.find(upper);
    if (it != keywords_.end()) {
        return Token(it->second, lexeme, startLine, startCol);
    }
    return Token(TokenType::IDENTIFIER, lexeme, startLine, startCol);
}

// ── Read an integer or float literal ─────────────────────────

Token Lexer::readNumber() {
    int startLine = line_, startCol = column_;
    std::string lexeme;
    bool isFloat = false;

    while (!isAtEnd() && isDigit(current())) {
        lexeme += advance();
    }

    // Check for decimal point followed by more digits
    if (!isAtEnd() && current() == '.' && isDigit(peek())) {
        isFloat = true;
        lexeme += advance(); // consume '.'
        while (!isAtEnd() && isDigit(current())) {
            lexeme += advance();
        }
    }

    TokenType type = isFloat ? TokenType::FLOAT_LITERAL : TokenType::INTEGER_LITERAL;
    return Token(type, lexeme, startLine, startCol);
}

// ── Read a string literal (single or double quoted) ──────────

Token Lexer::readString(char quote) {
    int startLine = line_, startCol = column_;
    advance(); // consume opening quote
    std::string lexeme;

    while (!isAtEnd()) {
        char c = current();

        // Escaped single quote: '' inside a single-quoted string
        if (c == quote && peek() == quote) {
            lexeme += quote;
            advance(); advance();
            continue;
        }

        if (c == quote) {
            advance(); // consume closing quote
            return Token(TokenType::STRING_LITERAL, lexeme, startLine, startCol);
        }

        if (c == '\n') {
            // Unterminated string at end of line
            break;
        }

        lexeme += advance();
    }

    // Unterminated string literal
    reporter_.error(startLine, startCol, lexeme,
                    "Unterminated string literal");
    return Token(TokenType::UNKNOWN, lexeme, startLine, startCol);
}

// ── Read operators and punctuation ───────────────────────────

Token Lexer::readOperatorOrPunctuation() {
    int startLine = line_, startCol = column_;
    char c = advance();

    switch (c) {
        case '=': return Token(TokenType::OP_EQ,      "=",  startLine, startCol);
        case '+': return Token(TokenType::OP_PLUS,    "+",  startLine, startCol);
        case '-': return Token(TokenType::OP_MINUS,   "-",  startLine, startCol);
        case '*': return Token(TokenType::OP_STAR,    "*",  startLine, startCol);
        case '/': return Token(TokenType::OP_SLASH,   "/",  startLine, startCol);
        case '%': return Token(TokenType::OP_PERCENT, "%",  startLine, startCol);
        case '(': return Token(TokenType::LPAREN,     "(",  startLine, startCol);
        case ')': return Token(TokenType::RPAREN,     ")",  startLine, startCol);
        case ',': return Token(TokenType::COMMA,      ",",  startLine, startCol);
        case ';': return Token(TokenType::SEMICOLON,  ";",  startLine, startCol);
        case '.': return Token(TokenType::DOT,        ".",  startLine, startCol);

        case '!':
            if (!isAtEnd() && current() == '=') {
                advance();
                return Token(TokenType::OP_NEQ, "!=", startLine, startCol);
            }
            reporter_.error(startLine, startCol, "!", "Unexpected character '!'");
            return Token(TokenType::UNKNOWN, "!", startLine, startCol);

        case '<':
            if (!isAtEnd() && current() == '=') {
                advance();
                return Token(TokenType::OP_LTE, "<=", startLine, startCol);
            }
            if (!isAtEnd() && current() == '>') {
                advance();
                return Token(TokenType::OP_NEQ, "<>", startLine, startCol);
            }
            return Token(TokenType::OP_LT, "<", startLine, startCol);

        case '>':
            if (!isAtEnd() && current() == '=') {
                advance();
                return Token(TokenType::OP_GTE, ">=", startLine, startCol);
            }
            return Token(TokenType::OP_GT, ">", startLine, startCol);

        default: {
            std::string ch(1, c);
            reporter_.error(startLine, startCol, ch,
                            "Unexpected character '" + ch + "'");
            return Token(TokenType::UNKNOWN, ch, startLine, startCol);
        }
    }
}

// ── Main tokenize loop ────────────────────────────────────────

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        skipWhitespaceAndComments();

        if (isAtEnd()) break;

        char c = current();

        if (isAlpha(c)) {
            tokens.push_back(readIdentifierOrKeyword());
        } else if (isDigit(c)) {
            tokens.push_back(readNumber());
        } else if (c == '\'' || c == '"') {
            tokens.push_back(readString(c));
        } else {
            tokens.push_back(readOperatorOrPunctuation());
        }
    }

    // Always end with EOF
    tokens.emplace_back(TokenType::END_OF_FILE, "", line_, column_);
    return tokens;
}
