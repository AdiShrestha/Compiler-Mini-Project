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
