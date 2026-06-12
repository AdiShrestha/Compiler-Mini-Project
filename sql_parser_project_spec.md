# SQL Query Parser — Complete Project Specification
## Compiler Design Project (Project F) | C++ Implementation

---

> **HOW TO USE THIS DOCUMENT**
> This is a self-contained master specification. Every design decision, grammar rule,
> file, module, class, method, test case, and report section is defined here.
> A developer with C++ knowledge can recreate this project entirely from this document alone.

---

## 1. PROJECT OVERVIEW

### 1.1 Objective
Build a **SQL Query Validator** — a C++ command-line application that:
- Accepts a SQL query string as input (interactively or from file)
- Runs it through a formal Lexer → Parser pipeline
- Reports `VALID` or `INVALID` with detailed error diagnostics (line, column, reason)

### 1.2 Scope — Supported SQL Statements
The parser covers the following SQL statement types:

| Statement     | Example                                                        |
|---------------|----------------------------------------------------------------|
| SELECT        | `SELECT * FROM users WHERE id = 1;`                            |
| INSERT        | `INSERT INTO users (id, name) VALUES (1, 'Alice');`            |
| UPDATE        | `UPDATE users SET name = 'Bob' WHERE id = 2;`                  |
| DELETE        | `DELETE FROM users WHERE id = 3;`                              |
| CREATE TABLE  | `CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(100));`  |
| DROP TABLE    | `DROP TABLE users;`                                            |

Supported clauses and features:
- `WHERE` with compound conditions (`AND`, `OR`, `NOT`)
- `ORDER BY`, `GROUP BY`, `HAVING`, `LIMIT`
- `JOIN` (INNER, LEFT, RIGHT, FULL)
- Aggregate functions: `COUNT`, `SUM`, `AVG`, `MAX`, `MIN`
- Subqueries in `WHERE` clause
- `DISTINCT`, `AS` aliasing
- `LIKE`, `IN`, `BETWEEN`, `IS NULL`, `IS NOT NULL`
- All standard comparison and arithmetic operators

---

## 2. TECHNOLOGY STACK & JUSTIFICATION

### Language: C++ (C++17 standard)
**Justification for report:**
> C++ was selected because it offers the precise level of abstraction required for implementing
> a compiler front-end pedagogically. Manual control over tokenization and recursive parsing
> demonstrates deep theoretical understanding. STL containers (`std::vector`, `std::stack`,
> `std::map`, `std::string`) handle auxiliary data structures without obscuring core parsing
> logic. The object-oriented model maps naturally to the theoretical pipeline: Lexer, Parser,
> and ErrorReporter as distinct, testable components. C++ also produces a fast native binary,
> making interactive demonstration seamless.

### Build System: GNU Make
### Standard: C++17
### Compiler: g++ (GCC) or clang++
### No external parser libraries (Flex/Bison are NOT used — everything is hand-written)
**Justification:** Hand-writing the lexer and parser from scratch demonstrates direct
application of compiler theory (CFG, recursive descent, grammar transformations).

---

## 3. THEORETICAL BACKGROUND (Required for Report)

### 3.1 Formal Language Theory
- SQL syntax is a **Context-Free Language (CFL)**
- It is described by a **Context-Free Grammar (CFG)**: G = (V, Σ, R, S)
  - V = set of non-terminals (e.g., SelectStmt, Condition, Expression)
  - Σ = set of terminals (tokens: SELECT, FROM, identifier, ';', etc.)
  - R = set of production rules
  - S = start symbol (Program)

### 3.2 Parser Type: Recursive Descent (LL(1) with extensions)
- **Top-down, predictive parser**
- Each non-terminal has a corresponding C++ method
- Uses 1-token lookahead to decide which production to apply
- Extended to handle left-recursive expressions using **iterative loops** (operator precedence climbing)
- Chosen over LR because recursive descent is more readable, debuggable, and maps directly to the grammar structure

### 3.3 Grammar Transformations Applied (Critical for Report Section)

#### 3.3.1 Left Recursion Removal
The natural grammar for expressions is left-recursive:

```
expression → expression '+' term          (LEFT RECURSIVE — cannot use in LL parser)
expression → term
```

This is transformed to:
```
expression  → term expression'
expression' → '+' term expression'
expression' → '-' term expression'
expression' → ε
```

In implementation this becomes an **iterative loop** (cleaner than the epsilon production):
```
term = parseTerm();
while (current token is + or -)
    op = consume()
    right = parseTerm()
    term = BinaryExpr(term, op, right)
return term
```

#### 3.3.2 Left Factoring
Before:
```
statement → SELECT ...
statement → SELECT DISTINCT ...
```
After (factor out common prefix):
```
statement  → SELECT select_modifier ...
select_modifier → DISTINCT | ε
```

#### 3.3.3 Ambiguity Resolution
AND/OR precedence: `a OR b AND c` must parse as `a OR (b AND c)`.
Resolved by splitting into separate grammar levels:
```
condition       → or_condition
or_condition    → and_condition ('OR' and_condition)*
and_condition   → not_condition ('AND' not_condition)*
not_condition   → 'NOT' not_condition | comparison
```

---

## 4. COMPLETE FORMAL GRAMMAR (CFG)

```
(* Start Symbol *)
Program         → StatementList EOF

StatementList   → Statement ';' StatementList
                | Statement ';'
                | Statement

Statement       → SelectStmt
                | InsertStmt
                | UpdateStmt
                | DeleteStmt
                | CreateStmt
                | DropStmt

(* ───────────── SELECT ───────────── *)
SelectStmt      → SELECT [DISTINCT] SelectList FROM TableRef
                  [JoinClause]
                  [WhereClause]
                  [GroupByClause]
                  [HavingClause]
                  [OrderByClause]
                  [LimitClause]

SelectList      → '*'
                | SelectItem (',' SelectItem)*

SelectItem      → Expression [AS Identifier]
                | Identifier '.' '*'

TableRef        → Identifier [AS Identifier]
                | '(' SelectStmt ')' AS Identifier

JoinClause      → JoinType JOIN TableRef ON Condition

JoinType        → INNER | LEFT | RIGHT | FULL | LEFT OUTER | RIGHT OUTER | FULL OUTER | ε

WhereClause     → WHERE Condition

GroupByClause   → GROUP BY ColumnList

HavingClause    → HAVING Condition

OrderByClause   → ORDER BY OrderItem (',' OrderItem)*
OrderItem       → Expression [ASC | DESC]

LimitClause     → LIMIT IntegerLiteral [OFFSET IntegerLiteral]

(* ───────────── INSERT ───────────── *)
InsertStmt      → INSERT INTO Identifier InsertBody

InsertBody      → VALUES '(' ValueList ')'
                | '(' ColumnList ')' VALUES '(' ValueList ')'

ValueList       → Value (',' Value)*
Value           → Literal | NULL | DEFAULT

(* ───────────── UPDATE ───────────── *)
UpdateStmt      → UPDATE Identifier SET AssignmentList [WhereClause]

AssignmentList  → Assignment (',' Assignment)*
Assignment      → Identifier '=' Expression

(* ───────────── DELETE ───────────── *)
DeleteStmt      → DELETE FROM Identifier [WhereClause]

(* ───────────── CREATE TABLE ───────────── *)
CreateStmt      → CREATE TABLE [IF NOT EXISTS] Identifier
                  '(' ColumnDefList [TableConstraintList] ')'

ColumnDefList   → ColumnDef (',' ColumnDef)*

ColumnDef       → Identifier DataType ColumnConstraint*

DataType        → INT | INTEGER | BIGINT | SMALLINT
                | VARCHAR '(' IntegerLiteral ')'
                | CHAR '(' IntegerLiteral ')'
                | TEXT | CLOB
                | FLOAT | DOUBLE | DECIMAL '(' IntegerLiteral ',' IntegerLiteral ')'
                | BOOLEAN | BOOL
                | DATE | DATETIME | TIMESTAMP

ColumnConstraint→ NOT NULL
                | NULL
                | PRIMARY KEY
                | UNIQUE
                | DEFAULT Literal
                | AUTO_INCREMENT
                | CHECK '(' Condition ')'
                | REFERENCES Identifier ['(' Identifier ')']

TableConstraintList → ',' TableConstraint (TableConstraint)*
TableConstraint → PRIMARY KEY '(' ColumnList ')'
                | UNIQUE '(' ColumnList ')'
                | FOREIGN KEY '(' ColumnList ')' REFERENCES Identifier '(' ColumnList ')'

(* ───────────── DROP ───────────── *)
DropStmt        → DROP TABLE [IF EXISTS] Identifier

(* ───────────── CONDITIONS ───────────── *)
Condition       → OrCondition

OrCondition     → AndCondition (OR AndCondition)*

AndCondition    → NotCondition (AND NotCondition)*

NotCondition    → NOT NotCondition
                | Predicate

Predicate       → Expression CompOp Expression
                | Expression IS [NOT] NULL
                | Expression [NOT] BETWEEN Expression AND Expression
                | Expression [NOT] IN '(' ValueList ')'
                | Expression [NOT] IN '(' SelectStmt ')'
                | Expression [NOT] LIKE StringLiteral
                | EXISTS '(' SelectStmt ')'
                | '(' Condition ')'

CompOp          → '=' | '!=' | '<>' | '<' | '>' | '<=' | '>='

(* ───────────── EXPRESSIONS ───────────── *)
Expression      → Term (( '+' | '-' ) Term)*

Term            → Factor (( '*' | '/' | '%' ) Factor)*

Factor          → ['-' | '+'] Primary

Primary         → Literal
                | NULL
                | Identifier
                | Identifier '.' Identifier
                | FunctionCall
                | '(' Expression ')'
                | '(' SelectStmt ')'

FunctionCall    → Identifier '(' ArgumentList ')'
                | COUNT '(' '*' ')'
                | COUNT '(' [DISTINCT] Expression ')'
                | AVG '(' Expression ')'
                | SUM '(' Expression ')'
                | MAX '(' Expression ')'
                | MIN '(' Expression ')'

ArgumentList    → Expression (',' Expression)* | ε

(* ───────────── PRIMITIVES ───────────── *)
ColumnList      → Identifier (',' Identifier)*

Literal         → IntegerLiteral
                | FloatLiteral
                | StringLiteral
                | BooleanLiteral

Identifier      → [a-zA-Z_][a-zA-Z0-9_]*   (* not a reserved keyword *)
IntegerLiteral  → [0-9]+
FloatLiteral    → [0-9]+ '.' [0-9]+
StringLiteral   → '\'' [^']* '\''
                | '"' [^"]* '"'
BooleanLiteral  → TRUE | FALSE
```

---

## 5. COMPLETE FILE STRUCTURE

```
sql-parser/
│
├── src/
│   ├── main.cpp              # Entry point, CLI loop, file input mode
│   ├── token.h               # TokenType enum, Token struct
│   ├── lexer.h               # Lexer class declaration
│   ├── lexer.cpp             # Lexer class implementation
│   ├── parser.h              # Parser class declaration
│   ├── parser.cpp            # Parser class implementation
│   ├── error_reporter.h      # ErrorReporter class declaration
│   ├── error_reporter.cpp    # ErrorReporter class implementation
│   └── ast.h                 # (Optional) AST node types for parse tree display
│
├── tests/
│   ├── valid_queries.sql     # 30+ valid SQL test cases (one per line)
│   ├── invalid_queries.sql   # 30+ invalid SQL test cases (one per line)
│   └── run_tests.sh          # Bash script that runs all test cases automatically
│
├── report/
│   └── report.docx           # Project report (see Section 12 for structure)
│
├── Makefile                  # Build configuration
├── README.md                 # Setup, usage, examples
└── .gitignore
```

---

## 6. TOKEN SPECIFICATION (token.h)

```cpp
// token.h
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
    KW_EXISTS, KW_ALL, KW_ANY,

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

    std::string typeToString() const;  // returns human-readable token type name
};
```

---

## 7. LEXER SPECIFICATION (lexer.h / lexer.cpp)

### 7.1 Class Interface

```cpp
// lexer.h
#pragma once
#include "token.h"
#include "error_reporter.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
public:
    explicit Lexer(const std::string& source, ErrorReporter& reporter);

    // Tokenize the entire input and return token list
    std::vector<Token> tokenize();

private:
    std::string     source_;
    int             pos_;       // current character index
    int             line_;      // current line (1-based)
    int             column_;    // current column (1-based)
    ErrorReporter&  reporter_;

    static const std::unordered_map<std::string, TokenType> keywords_;

    // Internal helpers
    char        current() const;
    char        peek(int offset = 1) const;
    char        advance();
    bool        isAtEnd() const;
    void        skipWhitespaceAndComments();

    Token       readIdentifierOrKeyword();
    Token       readNumber();
    Token       readString(char quote);
    Token       readOperatorOrPunctuation();

    bool        isDigit(char c) const;
    bool        isAlpha(char c) const;
    bool        isAlphaNumeric(char c) const;
};
```

### 7.2 Keyword Map (lexer.cpp)
The static `keywords_` map must contain ALL keywords from the TokenType enum, keyed by their uppercase string form. During identifier scanning, the lexeme is uppercased and checked against this map — if found, the keyword token is returned; otherwise an `IDENTIFIER` token is returned.

### 7.3 Comment Handling
- Single-line: `--` until end of line → skip
- Multi-line: `/* ... */` → skip (track line increments inside)

### 7.4 String Literal Handling
- Single-quoted: `'Alice'` → STRING_LITERAL with lexeme = `Alice`
- Handle escaped single quotes: `''` inside a string = literal single quote
- Unterminated string → report error, return UNKNOWN

### 7.5 Number Handling
- Integer: sequence of digits → INTEGER_LITERAL
- Float: digits `.` digits → FLOAT_LITERAL
- Invalid: `123abc` → lex `123` as INTEGER, then `abc` as IDENTIFIER (let parser catch the sequence error)

---

## 8. PARSER SPECIFICATION (parser.h / parser.cpp)

### 8.1 Class Interface

```cpp
// parser.h
#pragma once
#include "token.h"
#include "error_reporter.h"
#include <vector>

struct ParseResult {
    bool        valid;
    std::string summary;    // "VALID SQL" or first error description
};

class Parser {
public:
    Parser(std::vector<Token> tokens, ErrorReporter& reporter);
    ParseResult parse();

private:
    std::vector<Token>  tokens_;
    int                 pos_;
    ErrorReporter&      reporter_;

    // ── Token navigation ──
    const Token&    current() const;
    const Token&    peek(int offset = 1) const;
    Token           consume();
    Token           expect(TokenType type, const std::string& context);
    bool            check(TokenType type) const;
    bool            match(TokenType type);
    bool            matchAny(std::initializer_list<TokenType> types);
    bool            isAtEnd() const;

    // ── Top-level ──
    void    parseProgram();
    void    parseStatement();

    // ── Statements ──
    void    parseSelectStmt();
    void    parseInsertStmt();
    void    parseUpdateStmt();
    void    parseDeleteStmt();
    void    parseCreateStmt();
    void    parseDropStmt();

    // ── SELECT sub-clauses ──
    void    parseSelectList();
    void    parseSelectItem();
    void    parseTableRef();
    void    parseJoinClause();
    void    parseWhereClause();
    void    parseGroupByClause();
    void    parseHavingClause();
    void    parseOrderByClause();
    void    parseLimitClause();

    // ── INSERT sub-clauses ──
    void    parseInsertBody();
    void    parseValueList();

    // ── UPDATE sub-clauses ──
    void    parseAssignmentList();
    void    parseAssignment();

    // ── CREATE TABLE sub-clauses ──
    void    parseColumnDefList();
    void    parseColumnDef();
    void    parseDataType();
    void    parseColumnConstraints();
    void    parseTableConstraints();

    // ── Conditions (precedence: OR < AND < NOT < comparison) ──
    void    parseCondition();
    void    parseOrCondition();
    void    parseAndCondition();
    void    parseNotCondition();
    void    parsePredicate();

    // ── Expressions (precedence: +/- < */ < unary < primary) ──
    void    parseExpression();
    void    parseTerm();
    void    parseFactor();
    void    parsePrimary();
    void    parseFunctionCall(const std::string& name);

    // ── Helpers ──
    void    parseColumnList();
    void    parseIdentifier();
    void    parseLiteral();

    // ── Error recovery ──
    void    synchronize();  // skip tokens until ';' or EOF to recover after error
};
```

### 8.2 Error Recovery Strategy
When a parse error occurs:
1. `reporter_.error(token, message)` is called
2. `synchronize()` is called — advances past tokens until `;` or EOF
3. Parsing continues (allows reporting multiple errors in one pass)
4. `ParseResult.valid = false` if any errors were reported

### 8.3 Key Parsing Decisions

**Lookahead for statements:**
```
current token = SELECT → parseSelectStmt()
current token = INSERT → parseInsertStmt()
current token = UPDATE → parseUpdateStmt()
current token = DELETE → parseDeleteStmt()
current token = CREATE → parseCreateStmt()
current token = DROP   → parseDropStmt()
otherwise              → error: "Unexpected token, expected SQL statement"
```

**Lookahead for INSERT body:**
```
current token = '('   → parse column list, then VALUES, then value list
current token = VALUES → parse value list directly
```

**Distinguishing subquery vs expression in parentheses:**
```
'(' followed by SELECT → parse as subquery (SelectStmt)
'(' otherwise          → parse as grouped expression
```

---

## 9. ERROR REPORTER SPECIFICATION (error_reporter.h / error_reporter.cpp)

```cpp
// error_reporter.h
#pragma once
#include "token.h"
#include <string>
#include <vector>

struct ParseError {
    int         line;
    int         column;
    std::string lexeme;     // the offending token
    std::string message;    // human-readable error description
};

class ErrorReporter {
public:
    void    error(const Token& token, const std::string& message);
    void    error(int line, int col, const std::string& lexeme, const std::string& message);
    bool    hasErrors() const;
    void    printAll() const;
    void    clear();
    int     errorCount() const;

    const std::vector<ParseError>& getErrors() const;

private:
    std::vector<ParseError> errors_;
};
```

**Output format for errors:**
```
[ERROR] Line 1, Column 8: Unexpected token 'FRM' — expected FROM keyword
[ERROR] Line 1, Column 14: Missing table name after FROM
```

---

## 10. MAIN ENTRY POINT SPECIFICATION (main.cpp)

### 10.1 Modes of Operation

```
Mode 1 — Interactive REPL:
    Run: ./sql_parser
    Prompt: sql> _
    User types a query, presses Enter
    Output: [VALID] or [INVALID] with errors
    Type 'exit' or 'quit' to end

Mode 2 — Single query from command line:
    Run: ./sql_parser "SELECT * FROM users;"
    Output: [VALID] or [INVALID] with errors

Mode 3 — Batch file mode:
    Run: ./sql_parser --file test.sql
    Output: Results for each query in the file
```

### 10.2 Output Format

**Valid query:**
```
sql> SELECT name, age FROM employees WHERE age > 25;

  ✔  VALID SQL
─────────────────────────────────────────────
  Tokens  : 13
  Statement: SELECT
```

**Invalid query:**
```
sql> SELECT name FORM employees;

  ✗  INVALID SQL  (1 error)
─────────────────────────────────────────────
  [ERROR] Line 1, Col 13: Unexpected token 'FORM' — expected FROM keyword
```

### 10.3 Main Loop Pseudocode

```cpp
int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) != "--file") {
        // Mode 2: single query
        runQuery(argv[1]);
    } else if (argc == 3 && std::string(argv[1]) == "--file") {
        // Mode 3: file batch
        runFile(argv[2]);
    } else {
        // Mode 1: interactive REPL
        printBanner();
        std::string line;
        while (true) {
            std::cout << "sql> ";
            if (!std::getline(std::cin, line)) break;
            if (line == "exit" || line == "quit") break;
            if (line.empty()) continue;
            runQuery(line);
        }
    }
}

void runQuery(const std::string& query) {
    ErrorReporter reporter;
    Lexer lexer(query, reporter);
    auto tokens = lexer.tokenize();

    if (!reporter.hasErrors()) {
        Parser parser(tokens, reporter);
        auto result = parser.parse();
        printResult(result, reporter, tokens.size());
    } else {
        printResult({false, "Lexer errors"}, reporter, 0);
    }
}
```

---

## 11. MAKEFILE

```makefile
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = sql_parser
SRCDIR   = src
SRCS     = $(SRCDIR)/main.cpp \
           $(SRCDIR)/lexer.cpp \
           $(SRCDIR)/parser.cpp \
           $(SRCDIR)/error_reporter.cpp
OBJS     = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

test: all
	@bash tests/run_tests.sh

.PHONY: all clean test
```

---

## 12. TEST CASES

### 12.1 Valid Queries (tests/valid_queries.sql)

```sql
SELECT * FROM users;
SELECT id, name, email FROM customers;
SELECT DISTINCT department FROM employees;
SELECT * FROM orders WHERE total > 100;
SELECT name, salary FROM employees WHERE salary >= 50000 AND department = 'Engineering';
SELECT name FROM employees WHERE department = 'HR' OR department = 'Finance';
SELECT * FROM products WHERE price BETWEEN 10 AND 100;
SELECT * FROM users WHERE name LIKE 'A%';
SELECT * FROM orders WHERE status IN ('pending', 'shipped', 'delivered');
SELECT * FROM employees WHERE manager_id IS NULL;
SELECT * FROM orders WHERE created_at IS NOT NULL;
SELECT COUNT(*) FROM users;
SELECT department, COUNT(*) FROM employees GROUP BY department;
SELECT department, AVG(salary) FROM employees GROUP BY department HAVING AVG(salary) > 60000;
SELECT * FROM employees ORDER BY salary DESC;
SELECT * FROM orders ORDER BY created_at ASC LIMIT 10;
SELECT * FROM orders LIMIT 5 OFFSET 10;
SELECT e.name, d.name FROM employees e JOIN departments d ON e.dept_id = d.id;
SELECT e.name, d.name FROM employees e LEFT JOIN departments d ON e.dept_id = d.id;
SELECT name AS full_name, salary AS monthly_pay FROM employees;
INSERT INTO users VALUES (1, 'Alice', 'alice@mail.com');
INSERT INTO users (id, name, email) VALUES (2, 'Bob', 'bob@mail.com');
UPDATE users SET name = 'Charlie' WHERE id = 3;
UPDATE employees SET salary = salary + 5000 WHERE department = 'Engineering';
DELETE FROM users WHERE id = 10;
DELETE FROM orders WHERE created_at < '2023-01-01';
CREATE TABLE students (id INT PRIMARY KEY, name VARCHAR(100) NOT NULL, age INT);
CREATE TABLE orders (id INT PRIMARY KEY AUTO_INCREMENT, customer_id INT, total FLOAT, created_at DATE);
CREATE TABLE products (id INT, name VARCHAR(255) NOT NULL, price DECIMAL(10,2), UNIQUE(name));
DROP TABLE temp_table;
SELECT * FROM users WHERE id IN (SELECT user_id FROM orders WHERE total > 500);
SELECT name, (salary * 12) AS annual_salary FROM employees;
SELECT MAX(salary) FROM employees;
SELECT MIN(price), MAX(price), AVG(price) FROM products;
```

### 12.2 Invalid Queries (tests/invalid_queries.sql)

```sql
SELECT FROM users;                          -- missing column list
SELECT * FORM users;                        -- typo: FORM instead of FROM
SELECT * FROM;                              -- missing table name
SELECT * FROM users WHERE;                  -- WHERE without condition
SELECT * FROM users WHERE age >> 25;        -- invalid operator >>
INSERT users VALUES (1, 'Alice');           -- missing INTO
INSERT INTO VALUES (1, 2);                  -- missing table name
INSERT INTO users (id, name VALUES (1, 'Alice');  -- missing closing paren
UPDATE SET name = 'Bob';                    -- missing table name
UPDATE users name = 'Bob';                  -- missing SET
DELETE users WHERE id = 1;                  -- missing FROM
DELETE FROM WHERE id = 5;                   -- missing table name
CREATE users (id INT);                      -- missing TABLE keyword
CREATE TABLE (id INT);                      -- missing table name
CREATE TABLE users id INT;                  -- missing parentheses
DROP users;                                 -- missing TABLE keyword
SELECT * FROM users WHERE AND age > 25;     -- AND without left operand
SELECT * FROM users ORDER age;              -- missing BY
SELECT * FROM users GROUP age;              -- missing BY
SELECT name salary FROM employees;          -- missing comma between columns
SELECT * FROM employees JOIN ON e.id=d.id;  -- missing table name in JOIN
SELECT * FROM users WHERE name = ;          -- missing value after =
SELECT 1 + FROM users;                      -- incomplete expression
UPDATE users SET salary = WHERE id = 1;     -- missing expression after =
```

### 12.3 Test Runner (tests/run_tests.sh)

```bash
#!/bin/bash
PARSER=./sql_parser
PASS=0
FAIL=0

echo "=== Running Valid Query Tests ==="
while IFS= read -r query || [[ -n "$query" ]]; do
    [[ "$query" =~ ^--.*$ || -z "$query" ]] && continue
    result=$("$PARSER" "$query" 2>&1)
    if echo "$result" | grep -q "VALID"; then
        echo "  PASS: $query"
        ((PASS++))
    else
        echo "  FAIL (expected VALID): $query"
        echo "        Got: $result"
        ((FAIL++))
    fi
done < tests/valid_queries.sql

echo ""
echo "=== Running Invalid Query Tests ==="
while IFS= read -r query || [[ -n "$query" ]]; do
    [[ "$query" =~ ^--.*$ || -z "$query" ]] && continue
    # Strip inline comment
    query="${query%%--*}"
    query=$(echo "$query" | xargs)
    [[ -z "$query" ]] && continue
    result=$("$PARSER" "$query" 2>&1)
    if echo "$result" | grep -q "INVALID"; then
        echo "  PASS: $query"
        ((PASS++))
    else
        echo "  FAIL (expected INVALID): $query"
        echo "        Got: $result"
        ((FAIL++))
    fi
done < tests/invalid_queries.sql

echo ""
echo "=== Results: $PASS passed, $FAIL failed ==="
```

---

## 13. README.md

```markdown
# SQL Query Parser

A hand-written SQL validator built in C++ for the Compiler Design course.
Implements a full Lexer + Recursive Descent Parser pipeline.

## Requirements
- g++ with C++17 support (`g++ --version`)
- GNU Make

## Build
make

## Run
# Interactive mode
./sql_parser

# Single query
./sql_parser "SELECT * FROM users WHERE id = 1;"

# Batch file
./sql_parser --file tests/valid_queries.sql

## Run Tests
make test

## Supported Statements
SELECT, INSERT, UPDATE, DELETE, CREATE TABLE, DROP TABLE

## Grammar
See report/report.docx — Section 3 for the full CFG.
```

---

## 14. PROJECT REPORT STRUCTURE

The report must cover the following sections clearly:

### Section 1 — Introduction (1 page)
- Project objective
- Brief description of what was built
- Tools and language used

### Section 2 — Theoretical Background (2–3 pages)
- Context-Free Grammar (CFG) definition and relevance to SQL
- Lexical analysis theory (regular expressions, DFAs)
- Parsing theory: LL vs LR, why LL/recursive descent was chosen
- First and Follow sets (even if not formally computed, show examples)

### Section 3 — Grammar Specification (2–3 pages)
- Complete CFG in BNF/EBNF as defined in Section 4 of this document
- Annotated with what each rule covers

### Section 4 — Grammar Transformations (1–2 pages)
- Left recursion removal with before/after examples
- Left factoring examples
- Ambiguity resolution (AND/OR precedence)
- Justification: why these were necessary for LL parsing

### Section 5 — Language Justification (0.5 page)
- Why C++ was chosen (from Section 2 of this spec)

### Section 6 — Module Description (3–4 pages)
Each module described with:
- Purpose
- Input/Output
- Key design decisions
- Code snippets of the most important methods

Modules: Token, Lexer, Parser, ErrorReporter, Main

### Section 7 — Test Cases and Results (1–2 pages)
- Table of valid queries → expected VALID → actual result
- Table of invalid queries → expected INVALID → actual result + error message shown

### Section 8 — Conclusion (0.5 page)
- What was achieved
- Limitations (what SQL is not supported)
- Possible extensions

---

## 15. INTERACTIVE DEMO GUIDE (for evaluation day)

Prepare these live demo scenarios in this exact order:

1. **Basic SELECT** — `SELECT * FROM employees;` → VALID
2. **Complex SELECT** — `SELECT name, AVG(salary) FROM employees WHERE dept='Eng' GROUP BY dept HAVING AVG(salary) > 60000 ORDER BY name ASC LIMIT 10;` → VALID
3. **JOIN query** — `SELECT e.name, d.name FROM employees e LEFT JOIN departments d ON e.dept_id = d.id;` → VALID
4. **Subquery** — `SELECT * FROM users WHERE id IN (SELECT user_id FROM orders WHERE total > 500);` → VALID
5. **CREATE TABLE** — `CREATE TABLE products (id INT PRIMARY KEY, name VARCHAR(100) NOT NULL, price DECIMAL(10,2));` → VALID
6. **INSERT / UPDATE / DELETE** — one of each → VALID
7. **Typo error** — `SELECT * FORM users;` → INVALID with column/line error
8. **Missing keyword** — `INSERT users VALUES (1, 2);` → INVALID
9. **Incomplete expression** — `SELECT * FROM users WHERE age > ;` → INVALID
10. **Multi-error query** — something with 2+ errors → show multiple errors reported
11. **Batch file mode** — run `make test`, show pass/fail counts

**Key talking point:** Walk through the pipeline on the whiteboard:
`Input String → Lexer → Token Stream → Parser → Valid/Invalid + Errors`

---

## 16. IMPLEMENTATION ORDER (Suggested)

Follow this order to avoid circular dependencies:

```
Step 1: token.h                  (no dependencies)
Step 2: error_reporter.h/.cpp    (depends on token.h)
Step 3: lexer.h/.cpp             (depends on token.h, error_reporter.h)
Step 4: parser.h/.cpp            (depends on token.h, error_reporter.h)
Step 5: main.cpp                 (depends on all above)
Step 6: Write and run test cases
Step 7: ast.h (optional, for parse tree visualization)
Step 8: Report
```

---

## 17. COMMON IMPLEMENTATION PITFALLS TO AVOID

1. **Case sensitivity in keywords**: Always uppercase the lexeme before map lookup. SQL is case-insensitive for keywords.
2. **Semicolons**: Some queries may or may not end with `;`. Handle both.
3. **Multi-token operators**: `!=`, `<>`, `<=`, `>=` must be read as single tokens by the lexer — not two separate tokens.
4. **`*` ambiguity**: `SELECT *` uses `*` as wildcard; `a * b` uses it as multiplication. Both are `OP_STAR` token — the parser resolves context.
5. **String literal contents**: Do not keyword-check content inside string literals (`'FROM'` is a string, not a keyword).
6. **Trailing whitespace / empty input**: Handle gracefully.
7. **End of file**: Always ensure the last token is `END_OF_FILE` and parser checks for it correctly.
8. **`<>`** operator: Must be recognized as equivalent to `!=` — both are `OP_NEQ`.
