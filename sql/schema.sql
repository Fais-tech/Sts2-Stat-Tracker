-- =============================================================
-- Slay the Spire 2 Stat Tracker: PostgreSQL schema
--
-- 3 lookup tables  : cards, relics, encounters   (from the C++ headers)
-- 6 event tables   : runs, floors, combats, card_choices,
--                    relic_choices, badges_earned (from .run files)
--
-- Re-runnable: drops everything first, children before parents.
-- =============================================================

SET client_min_messages = warning;

DROP TABLE IF EXISTS badges_earned, relic_choices, card_choices,
                     combats, floors, runs,
                     encounters, relics, cards CASCADE;
DROP TYPE IF EXISTS act_name, encounter_difficulty;

-- -------------------------------------------------------------
-- Enum types: fixed sets of values.
-- Declaration order is the sort order, so ORDER BY act gives game order
-- instead of alphabetical. Overgrowth and Underdocks are the two
-- alternate first acts.
-- -------------------------------------------------------------
CREATE TYPE act_name             AS ENUM ('overgrowth', 'underdocks', 'hive', 'glory');
CREATE TYPE encounter_difficulty AS ENUM ('normal', 'elite', 'boss');

-- -------------------------------------------------------------
-- Lookup tables: one row per thing that exists in the game
-- -------------------------------------------------------------

CREATE TABLE cards (
    card_name    TEXT PRIMARY KEY,
    card_class   TEXT NOT NULL,   -- ironclad, silent, ..., colorless
    card_rarity  TEXT NOT NULL,   -- basic, common, uncommon, rare, ...
    card_type    TEXT NOT NULL    -- attack, skill, power, curse
);

CREATE TABLE relics (
    relic_name    TEXT PRIMARY KEY,
    relic_rarity  TEXT NOT NULL
);

CREATE TABLE encounters (
    encounter_name  TEXT PRIMARY KEY,
    act             act_name             NOT NULL,
    difficulty      encounter_difficulty NOT NULL
);

-- -------------------------------------------------------------
-- Event tables: one row per thing that happened in a run
-- -------------------------------------------------------------

CREATE TABLE runs (
    run_id               BIGINT PRIMARY KEY,   -- unix timestamp from the filename
    character_name       TEXT    NOT NULL,
    ascension            INT     NOT NULL,
    won_run              BOOLEAN NOT NULL,
    abandoned_run        BOOLEAN NOT NULL,
    run_time             INT,                  -- seconds
    killed_by_encounter  TEXT REFERENCES encounters (encounter_name),  -- NULL if not killed by a fight
    killed_by_event      TEXT                                          -- NULL if not killed by an event
);

-- Every map point visited. floor_number counts across the whole run.
CREATE TABLE floors (
    run_id           BIGINT NOT NULL REFERENCES runs (run_id) ON DELETE CASCADE,
    floor_number     INT    NOT NULL,
    act_number       INT    NOT NULL,
    room_type        TEXT,
    damage_taken     INT,
    hp_healed        INT,
    current_hp       INT,
    max_hp_gained    INT,
    gold_difference  INT,
    PRIMARY KEY (run_id, floor_number)
);

-- Subtype of floors: only monster / elite / boss floors.
-- The PK is also the FK, which makes this a one-to-one specialization.
CREATE TABLE combats (
    run_id          BIGINT NOT NULL,
    floor_number    INT    NOT NULL,
    encounter_name  TEXT   NOT NULL REFERENCES encounters (encounter_name),
    turns_taken     INT    NOT NULL CHECK (turns_taken >= 0),
    PRIMARY KEY (run_id, floor_number),
    FOREIGN KEY (run_id, floor_number) REFERENCES floors (run_id, floor_number) ON DELETE CASCADE
);

-- One row per card offered. Every row is a "seen"; was_picked says if it was taken.
CREATE TABLE card_choices (
    choice_id     SERIAL PRIMARY KEY,
    run_id        BIGINT  NOT NULL,
    floor_number  INT     NOT NULL,
    card_name     TEXT    NOT NULL REFERENCES cards (card_name),
    was_picked    BOOLEAN NOT NULL,
    FOREIGN KEY (run_id, floor_number) REFERENCES floors (run_id, floor_number) ON DELETE CASCADE
);

CREATE TABLE relic_choices (
    choice_id     SERIAL PRIMARY KEY,
    run_id        BIGINT  NOT NULL,
    floor_number  INT     NOT NULL,
    relic_name    TEXT    NOT NULL REFERENCES relics (relic_name),
    was_picked    BOOLEAN NOT NULL,
    FOREIGN KEY (run_id, floor_number) REFERENCES floors (run_id, floor_number) ON DELETE CASCADE
);

CREATE TABLE badges_earned (
    run_id      BIGINT NOT NULL REFERENCES runs (run_id) ON DELETE CASCADE,
    badge_name  TEXT   NOT NULL,
    rarity      TEXT   NOT NULL CHECK (rarity IN ('bronze', 'silver', 'gold')),
    PRIMARY KEY (run_id, badge_name)
);

-- -------------------------------------------------------------
-- Indexes on foreign keys used in joins.
-- (Postgres indexes primary keys automatically, but not foreign keys.)
-- -------------------------------------------------------------
CREATE INDEX idx_card_choices_card       ON card_choices  (card_name);
CREATE INDEX idx_card_choices_run_floor  ON card_choices  (run_id, floor_number);
CREATE INDEX idx_relic_choices_relic     ON relic_choices (relic_name);
CREATE INDEX idx_relic_choices_run_floor ON relic_choices (run_id, floor_number);
CREATE INDEX idx_combats_encounter       ON combats       (encounter_name);
CREATE INDEX idx_runs_killed_by          ON runs          (killed_by_encounter);