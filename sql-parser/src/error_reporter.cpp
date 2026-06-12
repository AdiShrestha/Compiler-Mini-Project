// error_reporter.cpp
#include "error_reporter.h"
#include <iostream>
#include <iomanip>

void ErrorReporter::error(const Token& token, const std::string& message) {
    errors_.push_back({token.line, token.column, token.lexeme, message});
}

void ErrorReporter::error(int line, int col, const std::string& lexeme,
                          const std::string& message) {
    errors_.push_back({line, col, lexeme, message});
}

bool ErrorReporter::hasErrors() const {
    return !errors_.empty();
}

int ErrorReporter::errorCount() const {
    return static_cast<int>(errors_.size());
}

void ErrorReporter::printAll() const {
    for (const auto& e : errors_) {
        std::cerr << "  [ERROR] Line " << e.line
                  << ", Col "  << e.column << ": ";
        if (!e.lexeme.empty()) {
            std::cerr << "'" << e.lexeme << "' — ";
        }
        std::cerr << e.message << "\n";
    }
}

void ErrorReporter::clear() {
    errors_.clear();
}

const std::vector<ParseError>& ErrorReporter::getErrors() const {
    return errors_;
}
