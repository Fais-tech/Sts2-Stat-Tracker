-- =============================================================
-- Showcase queries
-- Run all:   psql -d sts2 -f sql/queries.sql
-- Most exclude abandoned runs: quitting at Neow after 3 seconds
-- isn't a real loss and would drag win rates down.
-- =============================================================


-- 1. Win rate by character  (GROUP BY, FILTER, ROLLUP for an "Overall" row)
SELECT COALESCE(character_name, 'OVERALL')               AS character,
       COUNT(*)                                          AS runs,
       COUNT(*) FILTER (WHERE won_run)                   AS wins,
       ROUND(100.0 * AVG(won_run::int), 1)               AS win_rate_pct
FROM runs
WHERE NOT abandoned_run
GROUP BY ROLLUP (character_name)
ORDER BY character_name NULLS LAST;


-- 2. Does win rate drop as ascension goes up?
SELECT ascension,
       COUNT(*)                             AS runs,
       ROUND(100.0 * AVG(won_run::int), 1)  AS win_rate_pct
FROM runs
WHERE NOT abandoned_run
GROUP BY ascension
ORDER BY ascension;


-- 3. Deadliest encounter in each act  (JOIN + window function RANK)
WITH deaths AS (
    SELECT e.act,
           r.killed_by_encounter  AS encounter,
           e.difficulty,
           COUNT(*)               AS times_killed_me
    FROM runs r
    JOIN encounters e ON e.encounter_name = r.killed_by_encounter
    GROUP BY e.act, r.killed_by_encounter, e.difficulty
)
SELECT act, encounter, difficulty, times_killed_me
FROM (
    SELECT d.*,
           RANK() OVER (PARTITION BY act ORDER BY times_killed_me DESC) AS rnk
    FROM deaths d
) ranked
WHERE rnk <= 3
ORDER BY act, rnk;


-- 4. Hardest fights by average damage taken  (3-table join: combats -> floors, combats -> encounters)
SELECT c.encounter_name,
       e.act,
       e.difficulty,
       COUNT(*)                        AS times_fought,
       ROUND(AVG(f.damage_taken), 1)   AS avg_damage_taken,
       ROUND(AVG(c.turns_taken), 1)    AS avg_turns
FROM combats c
JOIN floors     f ON f.run_id = c.run_id AND f.floor_number = c.floor_number
JOIN encounters e ON e.encounter_name = c.encounter_name
GROUP BY c.encounter_name, e.act, e.difficulty
HAVING COUNT(*) >= 10
ORDER BY avg_damage_taken DESC
LIMIT 15;


-- 5. Card pick rate and win rate
--    Same numbers the C++ program computes, now done in SQL.
--    Compare a few rows against output/cards.csv to validate.
SELECT cc.card_name,
       c.card_class,
       c.card_rarity,
       COUNT(*)                                              AS times_seen,
       COUNT(*) FILTER (WHERE cc.was_picked)                 AS times_picked,
       ROUND(100.0 * AVG(cc.was_picked::int), 2)             AS pick_rate_pct,
       COUNT(*) FILTER (WHERE cc.was_picked AND r.won_run)   AS wins_when_picked,
       ROUND(100.0 * COUNT(*) FILTER (WHERE cc.was_picked AND r.won_run)
             / NULLIF(COUNT(*) FILTER (WHERE cc.was_picked), 0), 2) AS win_rate_when_picked_pct
FROM card_choices cc
JOIN cards c ON c.card_name = cc.card_name
JOIN runs  r ON r.run_id    = cc.run_id
GROUP BY cc.card_name, c.card_class, c.card_rarity
ORDER BY times_seen DESC
LIMIT 20;


-- 6. Cards that most improve win rate vs. that character's baseline
--    (CTEs + comparing each card to its character's average; min 10 picks)
WITH baseline AS (
    SELECT character_name, AVG(won_run::int) AS base_win_rate
    FROM runs
    WHERE NOT abandoned_run
    GROUP BY character_name
),
card_picks AS (
    SELECT r.character_name,
           cc.card_name,
           COUNT(*)            AS picks,
           AVG(r.won_run::int) AS win_rate_when_picked
    FROM card_choices cc
    JOIN runs r ON r.run_id = cc.run_id
    WHERE cc.was_picked AND NOT r.abandoned_run
    GROUP BY r.character_name, cc.card_name
    HAVING COUNT(*) >= 10
)
SELECT cp.character_name,
       cp.card_name,
       cp.picks,
       ROUND(100.0 * cp.win_rate_when_picked, 1)                    AS win_rate_when_picked_pct,
       ROUND(100.0 * b.base_win_rate, 1)                            AS character_baseline_pct,
       ROUND(100.0 * (cp.win_rate_when_picked - b.base_win_rate), 1) AS lift_pct
FROM card_picks cp
JOIN baseline b USING (character_name)
ORDER BY lift_pct DESC
LIMIT 15;


-- 7. Average HP by floor, winning runs vs. losing runs
--    Shows where losing runs start falling behind.
SELECT f.floor_number,
       ROUND(AVG(f.current_hp) FILTER (WHERE r.won_run), 1)     AS avg_hp_wins,
       ROUND(AVG(f.current_hp) FILTER (WHERE NOT r.won_run), 1) AS avg_hp_losses
FROM floors f
JOIN runs r ON r.run_id = f.run_id
WHERE NOT r.abandoned_run
GROUP BY f.floor_number
HAVING COUNT(*) FILTER (WHERE r.won_run) >= 5
ORDER BY f.floor_number;


-- 8. Running total of wins over time  (window function with ORDER BY)
SELECT to_timestamp(run_id)::date                         AS played_on,
       character_name,
       won_run,
       SUM(won_run::int) OVER (ORDER BY run_id)           AS cumulative_wins,
       COUNT(*)          OVER (ORDER BY run_id)           AS cumulative_runs
FROM runs
WHERE NOT abandoned_run
ORDER BY run_id DESC
LIMIT 20;
