# SQL Query Parser

A hand-written SQL validator built in C++ for the Compiler Design course.
Implements a full Lexer + Recursive Descent Parser pipeline.

## Requirements
- g++ with C++17 support (`g++ --version`)
- GNU Make

## Build
```bash
make
```

## Run
```bash
# Interactive mode
./sql_parser

# Single query
./sql_parser "SELECT * FROM users WHERE id = 1;"

# Batch file
./sql_parser --file tests/valid_queries.sql
```

## Run Tests
```bash
make test
```

## Supported Statements
SELECT, INSERT, UPDATE, DELETE, CREATE TABLE, DROP TABLE

## Grammar
See report/report.docx — Section 3 for the full CFG.
