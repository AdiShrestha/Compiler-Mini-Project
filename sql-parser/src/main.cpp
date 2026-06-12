// main.cpp
// Integrated with Lexer and Parser (Phase 2)
#include "lexer.h"
#include "parser.h"
#include "error_reporter.h"
#include <iostream>
#include <string>
#include <iomanip>

static void runQuery(const std::string& query) {
    ErrorReporter reporter;
    Lexer lexer(query, reporter);
    auto tokens = lexer.tokenize();

    if (reporter.hasErrors()) {
        std::cout << "\n  ✗  INVALID SQL  (Lexer errors)\n";
        std::cout << "─────────────────────────────────────────────\n";
        reporter.printAll();
        return;
    }

    Parser parser(tokens, reporter);
    auto result = parser.parse();

    if (result.valid) {
        std::cout << "\n  ✔  VALID SQL\n";
        std::cout << "─────────────────────────────────────────────\n";
        std::cout << "  Tokens   : " << tokens.size() - 1 << "\n";
        if (!tokens.empty() && tokens[0].type != TokenType::END_OF_FILE) {
            std::cout << "  Statement: " << tokens[0].lexeme << "\n";
        }
    } else {
        std::cout << "\n  ✗  INVALID SQL  (" << reporter.errorCount() << " errors)\n";
        std::cout << "─────────────────────────────────────────────\n";
        reporter.printAll();
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        // Single query from command line
        runQuery(argv[1]);
        return 0;
    }

    // Interactive mode
    std::string line;
    while (true) {
        std::cout << "\nsql> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;
        runQuery(line);
    }
    return 0;
}
