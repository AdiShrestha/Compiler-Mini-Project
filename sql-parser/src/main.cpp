// main.cpp  —  Phase 1 stub: tokenizes input and prints the token list.
// This will be replaced in Phase 4 with the full REPL + parser integration.
#include "lexer.h"
#include "error_reporter.h"
#include <iostream>
#include <string>
#include <iomanip>

static void printTokens(const std::vector<Token>& tokens) {
    std::cout << "\n";
    std::cout << std::left
              << std::setw(5)  << "Line"
              << std::setw(5)  << "Col"
              << std::setw(22) << "Type"
              << "Lexeme\n";
    std::cout << std::string(50, '-') << "\n";

    for (const auto& tok : tokens) {
        if (tok.type == TokenType::END_OF_FILE) break;
        std::cout << std::left
                  << std::setw(5)  << tok.line
                  << std::setw(5)  << tok.column
                  << std::setw(22) << tok.typeToString()
                  << tok.lexeme    << "\n";
    }
    std::cout << "\n";
}

static void runQuery(const std::string& query) {
    ErrorReporter reporter;
    Lexer lexer(query, reporter);
    auto tokens = lexer.tokenize();

    if (reporter.hasErrors()) {
        std::cout << "  [LEXER ERRORS]\n";
        reporter.printAll();
    } else {
        std::cout << "  Tokens (" << (tokens.size() - 1) << "):\n";
        printTokens(tokens);
    }
}

int main(int argc, char* argv[]) {
    std::cout << "==============================================\n";
    std::cout << "  SQL Parser — Phase 1 (Lexer Demo)\n";
    std::cout << "==============================================\n";

    if (argc >= 2) {
        // Single query from command line
        runQuery(argv[1]);
        return 0;
    }

    // Interactive mode
    std::string line;
    while (true) {
        std::cout << "sql> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;
        runQuery(line);
    }
    return 0;
}
