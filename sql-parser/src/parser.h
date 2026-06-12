// parser.h
// Implements the recursive descent parser for SQL statements.
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
