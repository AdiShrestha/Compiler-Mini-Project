# SQL Query Parser — Progress

## Phase 1 — Foundation: Token + Lexer + Error Reporter ✅ DONE

### Files Created
- `sql-parser/src/token.h` — TokenType enum (all keywords/operators/punctuation) + Token struct + `typeToString()`
- `sql-parser/src/error_reporter.h` — ParseError struct + ErrorReporter class interface
- `sql-parser/src/error_reporter.cpp` — ErrorReporter implementation
- `sql-parser/src/lexer.h` — Lexer class interface
- `sql-parser/src/lexer.cpp` — Full Lexer + static keyword map + `Token::typeToString()` definition
- `sql-parser/src/main.cpp` — Phase 1 stub (prints token table for any SQL input)
- `sql-parser/Makefile` — Build configuration

### Verified
- Compiles cleanly with `g++ -std=c++17 -Wall -Wextra -O2` (zero warnings)
- Correctly tokenizes: SELECT, INSERT, CREATE TABLE, subqueries, JOINs
- Multi-char operators: `!=`, `<>`, `<=`, `>=` → single tokens
- String literals: `'Alice'` → lexeme = `Alice` (quotes stripped)
- `--` single-line comments → skipped
- `/* ... */` multi-line comments → skipped
- Line/column tracking: correct even after multi-line input
- Typos (`FORM`) correctly lex as IDENTIFIER (parser will catch semantic error)

---

## Phase 2 — Parser: SELECT, INSERT, UPDATE, DELETE 🔜

## Phase 3 — Parser: CREATE TABLE, DROP, all clauses 🔜

## Phase 4 — main.cpp: REPL + single-query + file-batch 🔜

## Phase 5 — Test suite + README 🔜
