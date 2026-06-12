// token.h
// Defines all token types and the Token struct used by the Lexer and Parser.
#pragma once
#include <string>

enum class TokenType {
    // ── Literals ──
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,

    // ── Identifier ──
    IDENTIFIER,

    // ── DML Keywords ──
    KW_SELECT, KW_FROM, KW_WHERE, KW_INSERT, KW_INTO, KW_VALUES,
    KW_UPDATE, KW_SET, KW_DELETE, KW_DISTINCT, KW_AS,

    // ── DDL Keywords ──
    KW_CREATE, KW_TABLE, KW_DROP, KW_IF, KW_EXISTS, KW_NOT,

    // ── Join Keywords ──
    KW_JOIN, KW_INNER, KW_LEFT, KW_RIGHT, KW_FULL, KW_OUTER, KW_ON,

    // ── Clause Keywords ──
    KW_ORDER, KW_BY, KW_GROUP, KW_HAVING, KW_LIMIT, KW_OFFSET,
    KW_ASC, KW_DESC,

    // ── Condition Keywords ──
    KW_AND, KW_OR, KW_IN, KW_LIKE, KW_BETWEEN, KW_IS,
    KW_ALL, KW_ANY,

    // ── Value Keywords ──
    KW_NULL, KW_TRUE, KW_FALSE, KW_DEFAULT,

    // ── Constraint Keywords ──
    KW_PRIMARY, KW_KEY, KW_FOREIGN, KW_REFERENCES,
    KW_UNIQUE, KW_AUTO_INCREMENT, KW_CHECK,

    // ── Data Type Keywords ──
    KW_INT, KW_INTEGER, KW_BIGINT, KW_SMALLINT, KW_VARCHAR,
    KW_CHAR, KW_TEXT, KW_FLOAT, KW_DOUBLE, KW_DECIMAL,
    KW_BOOLEAN, KW_BOOL, KW_DATE, KW_DATETIME, KW_TIMESTAMP,
    KW_CLOB,

    // ── Aggregate Functions ──
    KW_COUNT, KW_SUM, KW_AVG, KW_MAX, KW_MIN,

    // ── Operators ──
    OP_EQ,          // =
    OP_NEQ,         // != or <>
    OP_LT,          // <
    OP_GT,          // >
    OP_LTE,         // <=
    OP_GTE,         // >=
    OP_PLUS,        // +
    OP_MINUS,       // -
    OP_STAR,        // *
    OP_SLASH,       // /
    OP_PERCENT,     // %

    // ── Punctuation ──
    LPAREN,         // (
    RPAREN,         // )
    COMMA,          // ,
    SEMICOLON,      // ;
    DOT,            // .

    // ── Special ──
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType   type;
    std::string lexeme;     // exact text from source
    int         line;       // 1-based line number
    int         column;     // 1-based column number

    Token(TokenType t, std::string lex, int ln, int col)
        : type(t), lexeme(std::move(lex)), line(ln), column(col) {}

    // Returns a human-readable string for the token type
    std::string typeToString() const;
};
