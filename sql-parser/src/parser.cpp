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
    if (match(TokenType::DOT)) {
        expect(TokenType::IDENTIFIER, "expected column name after '.'");
    }
    while (match(TokenType::COMMA)) {
        expect(TokenType::IDENTIFIER, "expected column name");
        if (match(TokenType::DOT)) {
            expect(TokenType::IDENTIFIER, "expected column name after '.'");
        }
    }
}

// ── Phase 3 Implementations ──

void Parser::parseCreateStmt() {
    expect(TokenType::KW_TABLE, "expected TABLE keyword after CREATE");
    if (match(TokenType::KW_IF)) {
        expect(TokenType::KW_NOT, "expected NOT after IF");
        expect(TokenType::KW_EXISTS, "expected EXISTS after IF NOT");
    }
    expect(TokenType::IDENTIFIER, "expected table name");
    expect(TokenType::LPAREN, "expected '(' before column definitions");
    parseColumnDefList();
    if (match(TokenType::COMMA)) {
        parseTableConstraints();
    }
    expect(TokenType::RPAREN, "expected ')' after table definition");
}

void Parser::parseDropStmt() {
    expect(TokenType::KW_TABLE, "expected TABLE keyword after DROP");
    if (match(TokenType::KW_IF)) {
        expect(TokenType::KW_EXISTS, "expected EXISTS after IF");
    }
    expect(TokenType::IDENTIFIER, "expected table name");
}

void Parser::parseJoinClause() {
    while (check(TokenType::KW_INNER) || check(TokenType::KW_LEFT) || check(TokenType::KW_RIGHT) || check(TokenType::KW_FULL) || check(TokenType::KW_JOIN)) {
        if (matchAny({TokenType::KW_INNER, TokenType::KW_LEFT, TokenType::KW_RIGHT, TokenType::KW_FULL})) {
            match(TokenType::KW_OUTER); // optional
        }
        expect(TokenType::KW_JOIN, "expected JOIN keyword");
        parseTableRef();
        expect(TokenType::KW_ON, "expected ON keyword");
        parseCondition();
    }
}

void Parser::parseGroupByClause() {
    expect(TokenType::KW_BY, "expected BY after GROUP");
    parseColumnList();
}

void Parser::parseHavingClause() {
    parseCondition();
}

void Parser::parseOrderByClause() {
    expect(TokenType::KW_BY, "expected BY after ORDER");
    parseExpression();
    matchAny({TokenType::KW_ASC, TokenType::KW_DESC}); // optional
    
    while (match(TokenType::COMMA)) {
        parseExpression();
        matchAny({TokenType::KW_ASC, TokenType::KW_DESC});
    }
}

void Parser::parseLimitClause() {
    expect(TokenType::INTEGER_LITERAL, "expected integer literal after LIMIT");
    if (match(TokenType::KW_OFFSET)) {
        expect(TokenType::INTEGER_LITERAL, "expected integer literal after OFFSET");
    }
}

void Parser::parseColumnDefList() {
    parseColumnDef();
    while (check(TokenType::COMMA)) {
        if (peek(1).type == TokenType::KW_PRIMARY || peek(1).type == TokenType::KW_UNIQUE || peek(1).type == TokenType::KW_FOREIGN) {
            break;
        }
        consume(); // consume COMMA
        parseColumnDef();
    }
}

void Parser::parseColumnDef() {
    expect(TokenType::IDENTIFIER, "expected column name");
    parseDataType();
    parseColumnConstraints();
}

void Parser::parseDataType() {
    if (matchAny({TokenType::KW_INT, TokenType::KW_INTEGER, TokenType::KW_BIGINT, TokenType::KW_SMALLINT,
                  TokenType::KW_TEXT, TokenType::KW_CLOB, TokenType::KW_FLOAT, TokenType::KW_DOUBLE,
                  TokenType::KW_BOOLEAN, TokenType::KW_BOOL, TokenType::KW_DATE, TokenType::KW_DATETIME, TokenType::KW_TIMESTAMP})) {
        return;
    }
    
    if (matchAny({TokenType::KW_VARCHAR, TokenType::KW_CHAR})) {
        expect(TokenType::LPAREN, "expected '(' for length");
        expect(TokenType::INTEGER_LITERAL, "expected length integer");
        expect(TokenType::RPAREN, "expected ')' after length");
    } else if (match(TokenType::KW_DECIMAL)) {
        expect(TokenType::LPAREN, "expected '(' for decimal precision");
        expect(TokenType::INTEGER_LITERAL, "expected precision integer");
        expect(TokenType::COMMA, "expected ','");
        expect(TokenType::INTEGER_LITERAL, "expected scale integer");
        expect(TokenType::RPAREN, "expected ')' after decimal arguments");
    } else {
        reporter_.error(current(), "Unexpected token '" + current().lexeme + "' — expected data type");
        consume();
    }
}

void Parser::parseColumnConstraints() {
    while (true) {
        if (match(TokenType::KW_NOT)) {
            expect(TokenType::KW_NULL, "expected NULL after NOT");
        } else if (match(TokenType::KW_NULL)) {
            // just NULL
        } else if (match(TokenType::KW_PRIMARY)) {
            expect(TokenType::KW_KEY, "expected KEY after PRIMARY");
        } else if (match(TokenType::KW_UNIQUE)) {
            // UNIQUE
        } else if (match(TokenType::KW_DEFAULT)) {
            if (!matchAny({TokenType::INTEGER_LITERAL, TokenType::FLOAT_LITERAL, TokenType::STRING_LITERAL, TokenType::KW_TRUE, TokenType::KW_FALSE, TokenType::KW_NULL})) {
                reporter_.error(current(), "expected literal value after DEFAULT");
                consume();
            }
        } else if (match(TokenType::KW_AUTO_INCREMENT)) {
            // AUTO_INCREMENT
        } else if (match(TokenType::KW_CHECK)) {
            expect(TokenType::LPAREN, "expected '(' after CHECK");
            parseCondition();
            expect(TokenType::RPAREN, "expected ')' after CHECK condition");
        } else if (match(TokenType::KW_REFERENCES)) {
            expect(TokenType::IDENTIFIER, "expected referenced table name");
            if (match(TokenType::LPAREN)) {
                expect(TokenType::IDENTIFIER, "expected referenced column name");
                expect(TokenType::RPAREN, "expected ')' after referenced column");
            }
        } else {
            break;
        }
    }
}

void Parser::parseTableConstraints() {
    while (true) {
        if (match(TokenType::KW_PRIMARY)) {
            expect(TokenType::KW_KEY, "expected KEY after PRIMARY");
            expect(TokenType::LPAREN, "expected '('");
            parseColumnList();
            expect(TokenType::RPAREN, "expected ')'");
        } else if (match(TokenType::KW_UNIQUE)) {
            expect(TokenType::LPAREN, "expected '('");
            parseColumnList();
            expect(TokenType::RPAREN, "expected ')'");
        } else if (match(TokenType::KW_FOREIGN)) {
            expect(TokenType::KW_KEY, "expected KEY after FOREIGN");
            expect(TokenType::LPAREN, "expected '('");
            parseColumnList();
            expect(TokenType::RPAREN, "expected ')'");
            expect(TokenType::KW_REFERENCES, "expected REFERENCES after FOREIGN KEY definition");
            expect(TokenType::IDENTIFIER, "expected referenced table name");
            expect(TokenType::LPAREN, "expected '('");
            parseColumnList();
            expect(TokenType::RPAREN, "expected ')'");
        } else {
            break;
        }
        
        if (!check(TokenType::COMMA)) break;
        if (peek(1).type == TokenType::KW_PRIMARY || peek(1).type == TokenType::KW_UNIQUE || peek(1).type == TokenType::KW_FOREIGN) {
            consume();
        } else {
            break;
        }
    }
}
