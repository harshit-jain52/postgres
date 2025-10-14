-- Clean slate
DROP TABLE IF EXISTS my_table CASCADE;
DROP USER IF EXISTS alice, bob, charlie, david, eve;

-- Create users
CREATE USER alice;
CREATE USER bob;
CREATE USER charlie;
CREATE USER david;
CREATE USER eve;

-- Create table
CREATE TABLE my_table (
    id SERIAL PRIMARY KEY,
    name TEXT,
    privacy TEXT,
    category TEXT
);

-- Insert sample data
INSERT INTO my_table (name, privacy, category) VALUES
('record1', 'level1', 'finance'),
('record2', 'level2', 'hr'),
('record3', 'level3', 'engg'),
('record4', 'level1', 'finance'),
('record5', 'level2', 'hr');

-- Test access for each user

\echo '\n--- Alice ---'
SET SESSION AUTHORIZATION alice;
\timing on
SELECT * FROM my_table;
\timing off


\echo '\n--- Bob ---'
SET SESSION AUTHORIZATION bob;
\timing on
SELECT * FROM my_table;
\timing off


\echo '\n--- Charlie ---'
SET SESSION AUTHORIZATION charlie;
\timing on
SELECT * FROM my_table;
\timing off


\echo '\n--- David ---'
SET SESSION AUTHORIZATION david;
\timing on
SELECT * FROM my_table;
\timing off


\echo '\n--- Eve ---'
SET SESSION AUTHORIZATION eve;
\timing on
SELECT * FROM my_table;
\timing off

RESET SESSION AUTHORIZATION;