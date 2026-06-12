// main.cpp
// Entry point, CLI loop, file input mode
#include "error_reporter.h"
#include "lexer.h"
#include "parser.h"
#include <fstream>
#include <iostream>
#include <string>

static void printResult(const ParseResult &result,
                        const ErrorReporter &reporter, size_t numTokens) {
  if (result.valid) {
    std::cout << "\n  ✔  VALID SQL\n";
    std::cout << "─────────────────────────────────────────────\n";
    std::cout << "  Tokens   : " << numTokens << "\n";
  } else {
    std::cout << "\n  ✗  INVALID SQL  (" << reporter.errorCount()
              << " errors)\n";
    std::cout << "─────────────────────────────────────────────\n";
    reporter.printAll();
  }
}

static void runQuery(const std::string &query) {
  ErrorReporter reporter;
  Lexer lexer(query, reporter);
  auto tokens = lexer.tokenize();

  if (reporter.hasErrors()) {
    printResult({false, "Lexer errors"}, reporter, 0);
    return;
  }

  Parser parser(tokens, reporter);
  auto result = parser.parse();

  size_t tokenCount = tokens.empty() ? 0 : tokens.size() - 1; // Exclude EOF
  printResult(result, reporter, tokenCount);

  if (result.valid && !tokens.empty() &&
      tokens[0].type != TokenType::END_OF_FILE) {
    std::cout << "  Statement: " << tokens[0].lexeme << "\n";
  }
}

static void runFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file '" << path << "'\n";
    return;
  }

  std::string line;
  // Assuming one query per line as per spec's valid_queries.sql description
  while (std::getline(file, line)) {
    // Skip empty lines or full-line comments
    if (line.empty() || line.find("--") == 0)
      continue;

    std::cout << "\n> " << line;
    runQuery(line);
  }
}

static void printBanner() {
  std::cout << "==============================================\n";
  std::cout << "  SQL Parser — Interactive REPL\n";
  std::cout << "==============================================\n";
}

int main(int argc, char *argv[]) {
  if (argc == 2 && std::string(argv[1]) != "--file") {
    // Mode 2: Single query
    runQuery(argv[1]);
    return 0;
  } else if (argc == 3 && std::string(argv[1]) == "--file") {
    // Mode 3: Batch file mode
    runFile(argv[2]);
    return 0;
  }

  // Mode 1: Interactive REPL
  printBanner();
  std::string line;
  while (true) {
    std::cout << "\nsql> ";
    if (!std::getline(std::cin, line))
      break;
    if (line == "exit" || line == "quit")
      break;
    if (line.empty())
      continue;
    runQuery(line);
  }
  return 0;
}
