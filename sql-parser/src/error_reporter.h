// error_reporter.h
// Collects and displays parse/lex errors with line and column information.
#pragma once
#include "token.h"
#include <string>
#include <vector>

struct ParseError {
    int         line;
    int         column;
    std::string lexeme;     // the offending token text
    std::string message;    // human-readable description
};

class ErrorReporter {
public:
    // Record an error at the given token's position
    void error(const Token& token, const std::string& message);

    // Record an error at an explicit position (used by lexer)
    void error(int line, int col, const std::string& lexeme, const std::string& message);

    bool    hasErrors()  const;
    int     errorCount() const;

    // Print all collected errors to stderr
    void    printAll()   const;

    // Reset — used between queries in the REPL
    void    clear();

    const std::vector<ParseError>& getErrors() const;

private:
    std::vector<ParseError> errors_;
};
