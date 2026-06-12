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
