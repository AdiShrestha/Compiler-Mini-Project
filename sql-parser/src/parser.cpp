// parser.cpp
#include "parser.h"
#include <iostream>
#include <initializer_list>

Parser::Parser(std::vector<Token> tokens, ErrorReporter& reporter)
    : tokens_(std::move(tokens)), pos_(0), reporter_(reporter) {}

ParseResult Parser::parse() {
    parseProgram();
    
    if (reporter_.hasErrors()) {
        return {false, reporter_.getErrors()[0].message};
    }
    return {true, "VALID SQL"};
}

// ── Token Navigation ──

const Token& Parser::current() const {
    return tokens_[pos_];
}

const Token& Parser::peek(int offset) const {
    int idx = pos_ + offset;
    if (idx >= static_cast<int>(tokens_.size())) return tokens_.back(); // EOF
    return tokens_[idx];
}

Token Parser::consume() {
    if (!isAtEnd()) pos_++;
    return tokens_[pos_ - 1];
}

Token Parser::expect(TokenType type, const std::string& context) {
    if (check(type)) return consume();
    
    // We could add a typeToString helper, but we'll use a generic message here
    reporter_.error(current(), "Unexpected token '" + current().lexeme + "' — " + context);
    return Token(TokenType::UNKNOWN, "", current().line, current().column);
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return type == TokenType::END_OF_FILE;
    return current().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        consume();
        return true;
    }
    return false;
}

bool Parser::matchAny(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            consume();
            return true;
        }
    }
    return false;
}

bool Parser::isAtEnd() const {
    return current().type == TokenType::END_OF_FILE;
}

void Parser::synchronize() {
    // Skip until we find a semicolon or EOF to recover from errors
    consume();
    while (!isAtEnd()) {
        if (current().type == TokenType::SEMICOLON) {
            consume();
            return;
        }
        consume();
    }
}

// ── Top Level ──

void Parser::parseProgram() {
    while (!isAtEnd()) {
        parseStatement();
        if (check(TokenType::SEMICOLON)) consume(); // Optional trailing semicolon
    }
}

void Parser::parseStatement() {
    if (match(TokenType::KW_SELECT)) {
        parseSelectStmt();
    } else if (match(TokenType::KW_INSERT)) {
        parseInsertStmt();
    } else if (match(TokenType::KW_UPDATE)) {
        parseUpdateStmt();
    } else if (match(TokenType::KW_DELETE)) {
        parseDeleteStmt();
    } else if (match(TokenType::KW_CREATE)) {
        parseCreateStmt();
    } else if (match(TokenType::KW_DROP)) {
        parseDropStmt();
    } else {
        reporter_.error(current(), "Unexpected token '" + current().lexeme + "' — expected SQL statement (SELECT, INSERT, UPDATE, DELETE, CREATE, DROP)");
        synchronize();
    }
}

// ── SELECT ──

void Parser::parseSelectStmt() {
    match(TokenType::KW_DISTINCT); // Optional
    
    parseSelectList();
    
    expect(TokenType::KW_FROM, "expected FROM keyword");
    parseTableRef();
    
    if (check(TokenType::KW_INNER) || check(TokenType::KW_LEFT) || check(TokenType::KW_RIGHT) || check(TokenType::KW_FULL) || check(TokenType::KW_JOIN)) {
        parseJoinClause(); // Stubbed for Phase 3
    }
    
    if (match(TokenType::KW_WHERE)) {
        parseWhereClause();
    }
    
    if (match(TokenType::KW_GROUP)) {
        parseGroupByClause(); // Stubbed for Phase 3
    }
    
    if (match(TokenType::KW_HAVING)) {
        parseHavingClause(); // Stubbed for Phase 3
    }
    
    if (match(TokenType::KW_ORDER)) {
        parseOrderByClause(); // Stubbed for Phase 3
    }
    
    if (match(TokenType::KW_LIMIT)) {
        parseLimitClause(); // Stubbed for Phase 3
    }
}

void Parser::parseSelectList() {
    if (match(TokenType::OP_STAR)) {
        return;
    }
    
    parseSelectItem();
    while (match(TokenType::COMMA)) {
        parseSelectItem();
    }
}

void Parser::parseSelectItem() {
    parseExpression();
    if (match(TokenType::KW_AS)) {
        expect(TokenType::IDENTIFIER, "expected alias identifier after AS");
    } else if (check(TokenType::IDENTIFIER)) {
        // Optional alias without AS
        consume();
    }
}

void Parser::parseTableRef() {
    if (match(TokenType::LPAREN)) {
        expect(TokenType::KW_SELECT, "expected SELECT for subquery");
        parseSelectStmt();
        expect(TokenType::RPAREN, "expected ')' closing subquery");
        if (match(TokenType::KW_AS)) {
            expect(TokenType::IDENTIFIER, "expected alias after subquery AS");
        } else {
            expect(TokenType::IDENTIFIER, "expected alias after subquery");
        }
    } else {
        expect(TokenType::IDENTIFIER, "expected table name");
        if (match(TokenType::KW_AS)) {
            expect(TokenType::IDENTIFIER, "expected alias after AS");
        } else if (check(TokenType::IDENTIFIER)) {
            consume();
        }
    }
}

void Parser::parseWhereClause() {
    parseCondition();
}

// ── INSERT ──

void Parser::parseInsertStmt() {
    expect(TokenType::KW_INTO, "expected INTO keyword");
    expect(TokenType::IDENTIFIER, "expected table name");
    parseInsertBody();
}

void Parser::parseInsertBody() {
    if (match(TokenType::LPAREN)) {
        parseColumnList();
        expect(TokenType::RPAREN, "expected ')' after column list");
    }
    
    expect(TokenType::KW_VALUES, "expected VALUES keyword");
    expect(TokenType::LPAREN, "expected '(' before value list");
    parseValueList();
    expect(TokenType::RPAREN, "expected ')' after value list");
}

void Parser::parseValueList() {
    parseExpression(); // Simplification: Expression handles Literals/NULL
    while (match(TokenType::COMMA)) {
        parseExpression();
    }
}

// ── UPDATE ──

void Parser::parseUpdateStmt() {
    expect(TokenType::IDENTIFIER, "expected table name");
    expect(TokenType::KW_SET, "expected SET keyword");
    parseAssignmentList();
    
    if (match(TokenType::KW_WHERE)) {
        parseWhereClause();
    }
}

void Parser::parseAssignmentList() {
    parseAssignment();
    while (match(TokenType::COMMA)) {
        parseAssignment();
    }
}

void Parser::parseAssignment() {
    expect(TokenType::IDENTIFIER, "expected column name");
    expect(TokenType::OP_EQ, "expected '=' in assignment");
    parseExpression();
}

// ── DELETE ──

void Parser::parseDeleteStmt() {
    expect(TokenType::KW_FROM, "expected FROM keyword");
    expect(TokenType::IDENTIFIER, "expected table name");
    
    if (match(TokenType::KW_WHERE)) {
        parseWhereClause();
    }
}

// ── Conditions (OR < AND < NOT < Predicate) ──

void Parser::parseCondition() {
    parseOrCondition();
}

void Parser::parseOrCondition() {
    parseAndCondition();
    while (match(TokenType::KW_OR)) {
        parseAndCondition();
    }
}

void Parser::parseAndCondition() {
    parseNotCondition();
    while (match(TokenType::KW_AND)) {
        parseNotCondition();
    }
}

void Parser::parseNotCondition() {
    if (match(TokenType::KW_NOT)) {
        parseNotCondition();
    } else {
        parsePredicate();
    }
}

void Parser::parsePredicate() {
    // Check if EXISTS
    if (match(TokenType::KW_EXISTS)) {
        expect(TokenType::LPAREN, "expected '(' after EXISTS");
        expect(TokenType::KW_SELECT, "expected SELECT for EXISTS subquery");
        parseSelectStmt();
        expect(TokenType::RPAREN, "expected ')' after EXISTS subquery");
        return;
    }

    parseExpression();

    if (matchAny({TokenType::OP_EQ, TokenType::OP_NEQ, TokenType::OP_LT, TokenType::OP_GT, TokenType::OP_LTE, TokenType::OP_GTE})) {
        parseExpression();
    } else if (match(TokenType::KW_IS)) {
        match(TokenType::KW_NOT); // Optional NOT
        expect(TokenType::KW_NULL, "expected NULL after IS");
    } else if (match(TokenType::KW_BETWEEN)) {
        parseExpression();
        expect(TokenType::KW_AND, "expected AND in BETWEEN clause");
        parseExpression();
    } else if (match(TokenType::KW_IN)) {
        expect(TokenType::LPAREN, "expected '(' after IN");
        if (match(TokenType::KW_SELECT)) {
            parseSelectStmt();
        } else {
            parseValueList();
        }
        expect(TokenType::RPAREN, "expected ')' after IN list");
    } else if (match(TokenType::KW_LIKE)) {
        parseExpression();
    }
}

// ── Expressions (+/- < */ < Unary < Primary) ──

void Parser::parseExpression() {
    parseTerm();
    while (matchAny({TokenType::OP_PLUS, TokenType::OP_MINUS})) {
        parseTerm();
    }
}

void Parser::parseTerm() {
    parseFactor();
    while (matchAny({TokenType::OP_STAR, TokenType::OP_SLASH, TokenType::OP_PERCENT})) {
        parseFactor();
    }
}

void Parser::parseFactor() {
    matchAny({TokenType::OP_PLUS, TokenType::OP_MINUS}); // Optional unary prefix
    parsePrimary();
}

void Parser::parsePrimary() {
    if (matchAny({TokenType::INTEGER_LITERAL, TokenType::FLOAT_LITERAL, TokenType::STRING_LITERAL, TokenType::KW_TRUE, TokenType::KW_FALSE, TokenType::KW_NULL})) {
        return;
    }
    
    if (match(TokenType::IDENTIFIER)) {
        if (match(TokenType::DOT)) {
            if (match(TokenType::OP_STAR)) {
                // Table.*
                return;
            }
            expect(TokenType::IDENTIFIER, "expected column name after '.'");
        } else if (match(TokenType::LPAREN)) {
            // Function call
            if (match(TokenType::OP_STAR)) {
                // e.g. COUNT(*)
                expect(TokenType::RPAREN, "expected ')' after '*'");
            } else if (match(TokenType::KW_DISTINCT)) {
                parseExpression();
                expect(TokenType::RPAREN, "expected ')' after function arguments");
            } else if (!check(TokenType::RPAREN)) {
                parseValueList(); // Argument list
                expect(TokenType::RPAREN, "expected ')' after function arguments");
            } else {
                consume(); // RPAREN for empty args
            }
        }
        return;
    }
    
    if (match(TokenType::LPAREN)) {
        if (match(TokenType::KW_SELECT)) {
            parseSelectStmt();
        } else {
            parseExpression();
        }
        expect(TokenType::RPAREN, "expected ')'");
        return;
    }
    
    // Aggregates like COUNT, AVG
    if (matchAny({TokenType::KW_COUNT, TokenType::KW_SUM, TokenType::KW_AVG, TokenType::KW_MAX, TokenType::KW_MIN})) {
        expect(TokenType::LPAREN, "expected '(' after aggregate function");
        if (match(TokenType::OP_STAR)) {
            // COUNT(*)
        } else {
            match(TokenType::KW_DISTINCT); // Optional
            parseExpression();
        }
        expect(TokenType::RPAREN, "expected ')' after aggregate arguments");
        return;
    }

    reporter_.error(current(), "Unexpected token '" + current().lexeme + "' in expression");
    consume(); // prevent infinite loop
}

// ── Helpers ──

void Parser::parseColumnList() {
    expect(TokenType::IDENTIFIER, "expected column name");
    while (match(TokenType::COMMA)) {
        expect(TokenType::IDENTIFIER, "expected column name");
    }
}

// ── Phase 3 Stubs ──

void Parser::parseCreateStmt() {
    // Phase 3
    reporter_.error(current(), "CREATE TABLE is not implemented in Phase 2");
    synchronize();
}

void Parser::parseDropStmt() {
    // Phase 3
    reporter_.error(current(), "DROP TABLE is not implemented in Phase 2");
    synchronize();
}

void Parser::parseJoinClause() {
    // Consume until WHERE/GROUP/ORDER or end
    while (!isAtEnd() && !check(TokenType::KW_WHERE) && !check(TokenType::KW_GROUP) && !check(TokenType::KW_ORDER) && !check(TokenType::KW_LIMIT) && !check(TokenType::SEMICOLON)) {
        consume();
    }
}

void Parser::parseGroupByClause() {
    expect(TokenType::KW_BY, "expected BY after GROUP");
    while (!isAtEnd() && !check(TokenType::KW_HAVING) && !check(TokenType::KW_ORDER) && !check(TokenType::KW_LIMIT) && !check(TokenType::SEMICOLON)) {
        consume();
    }
}

void Parser::parseHavingClause() {
    while (!isAtEnd() && !check(TokenType::KW_ORDER) && !check(TokenType::KW_LIMIT) && !check(TokenType::SEMICOLON)) {
        consume();
    }
}

void Parser::parseOrderByClause() {
    expect(TokenType::KW_BY, "expected BY after ORDER");
    while (!isAtEnd() && !check(TokenType::KW_LIMIT) && !check(TokenType::SEMICOLON)) {
        consume();
    }
}

void Parser::parseLimitClause() {
    consume(); // integer literal
    if (match(TokenType::KW_OFFSET)) consume();
}

void Parser::parseColumnDefList() {}
void Parser::parseColumnDef() {}
void Parser::parseDataType() {}
void Parser::parseColumnConstraints() {}
void Parser::parseTableConstraints() {}
