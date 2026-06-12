// lexer.h
// Converts a raw SQL string into a flat list of Tokens.
#pragma once
#include "token.h"
#include "error_reporter.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
public:
    explicit Lexer(const std::string& source, ErrorReporter& reporter);

    // Scan the full source and return all tokens (last token is END_OF_FILE)
    std::vector<Token> tokenize();

private:
    const std::string& source_;
    int                pos_;    // current character index
    int                line_;   // 1-based line number
    int                column_; // 1-based column number
    ErrorReporter&     reporter_;

    // Static keyword lookup table
    static const std::unordered_map<std::string, TokenType> keywords_;

    // ── Character navigation ──
    char current()             const;
    char peek(int offset = 1)  const;
    char advance();
    bool isAtEnd()             const;

    // ── Whitespace / comment skipping ──
    void skipWhitespaceAndComments();

    // ── Token readers ──
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readString(char quote);
    Token readOperatorOrPunctuation();

    // ── Character classifiers ──
    bool isDigit(char c)        const;
    bool isAlpha(char c)        const;
    bool isAlphaNumeric(char c) const;

    // ── Utility ──
    std::string toUpper(const std::string& s) const;
};
