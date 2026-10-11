-- =============================================================
-- Load the CSVs written by sts_tracker into the tables.
-- Run from the project root (paths are relative to output/raw/).
-- Order matters: parents before children, so foreign keys resolve.
-- Empty CSV fields load as NULL.
-- =============================================================

\set ON_ERROR_STOP on

BEGIN;

-- Lookups first
\copy cards      FROM 'output/raw/cards.csv'      WITH (FORMAT csv, HEADER true)
\copy relics     FROM 'output/raw/relics.csv'     WITH (FORMAT csv, HEADER true)
\copy encounters FROM 'output/raw/encounters.csv' WITH (FORMAT csv, HEADER true)

-- Then runs, then everything that points at runs / floors
\copy runs          FROM 'output/raw/runs.csv'          WITH (FORMAT csv, HEADER true)
\copy floors        FROM 'output/raw/floors.csv'        WITH (FORMAT csv, HEADER true)
\copy combats       FROM 'output/raw/combats.csv'       WITH (FORMAT csv, HEADER true)
\copy badges_earned FROM 'output/raw/badges_earned.csv' WITH (FORMAT csv, HEADER true)

-- choice_id is SERIAL, so list the columns that are actually in the CSV
\copy card_choices  (run_id, floor_number, card_name,  was_picked) FROM 'output/raw/card_choices.csv'  WITH (FORMAT csv, HEADER true)
\copy relic_choices (run_id, floor_number, relic_name, was_picked) FROM 'output/raw/relic_choices.csv' WITH (FORMAT csv, HEADER true)

COMMIT;

-- Row counts, to compare against `wc -l output/raw/*.csv` (minus 1 header line each)
SELECT 'runs'          AS table_name, COUNT(*) AS row_count FROM runs
UNION ALL SELECT 'floors',        COUNT(*) FROM floors
UNION ALL SELECT 'combats',       COUNT(*) FROM combats
UNION ALL SELECT 'card_choices',  COUNT(*) FROM card_choices
UNION ALL SELECT 'relic_choices', COUNT(*) FROM relic_choices
UNION ALL SELECT 'badges_earned', COUNT(*) FROM badges_earned
UNION ALL SELECT 'cards',         COUNT(*) FROM cards
UNION ALL SELECT 'relics',        COUNT(*) FROM relics
UNION ALL SELECT 'encounters',    COUNT(*) FROM encounters;
