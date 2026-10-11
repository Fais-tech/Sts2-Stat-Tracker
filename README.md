# Sts2-Stat-Tracker

A C++ command-line tool that parses run history files from **Slay the Spire 2**, generates statistics across characters, cards, relics, badges, and enemy encounters, and exports a **normalized PostgreSQL database** for SQL analysis.

## Features

- **Character Statistics** — win rate, total runs, and abandoned runs per character
- **Card Statistics** — pick rate and win rate for every card, grouped by class and rarity
- **Relic Statistics** — pick rate and win rate for every relic, grouped by rarity
- **Badge Statistics** — bronze, silver, and gold counts for all earned badges
- **Encounter Statistics** — average damage taken, average turns, and kill count per enemy encounter, grouped by act and difficulty
- **PostgreSQL export** — 9-table normalized schema (~56,000 rows from 530 runs) with showcase analytical queries

## How to Build

Requires a C++17 compiler and [nlohmann/json](https://github.com/nlohmann/json).

```bash
g++ -std=c++17 -o sts_tracker stsProject.cpp
```

## How to Use

Place your `.run` history files in a folder and run:

```bash
# Uses a "history/" folder in the current directory by default
./sts_tracker

# Or specify a custom path
./sts_tracker /path/to/your/history
```

The program prints a statistics report, writes summary CSVs to `output/`, and writes normalized, one-row-per-event CSVs to `output/raw/`.

## Database

### Why normalize?

The original program aggregated everything in C++ (effectively a hand-written `SUM ... GROUP BY`), so the summary CSVs couldn't be joined or re-sliced. The raw export instead records **one row per event** — every card offered, every floor, every fight — and lets SQL do the aggregation. The original summary CSVs are kept as an answer key to validate the SQL results.

### Schema

```
            cards           relics          encounters        (lookup tables, from the C++ headers)
              ▲               ▲              ▲      ▲
              │               │              │      │
        card_choices    relic_choices     combats   │
              │               │              │      │
              └───────┬───────┘              │      │
                      ▼                      ▼      │
                    floors ◄─────────────────┘      │
                      │                             │
                      ▼                             │
   badges_earned ──► runs ──── killed_by_encounter ─┘
```

| Table | Grain (one row per…) | Primary key |
|---|---|---|
| `runs` | run | `run_id` (unix timestamp from the filename) |
| `floors` | map point visited | `(run_id, floor_number)` |
| `combats` | fight (subtype of `floors`) | `(run_id, floor_number)` — also the FK to `floors` |
| `card_choices` | card offered | `choice_id` (serial) |
| `relic_choices` | relic offered | `choice_id` (serial) |
| `badges_earned` | badge earned in a run | `(run_id, badge_name)` |
| `cards`, `relics`, `encounters` | game entity | name |

Design notes:
- **Lookup vs. event tables** — facts that depend only on a card/relic/encounter (rarity, act, difficulty) live once in a lookup table; event tables reference them by foreign key.
- **Subtype table** — combat-only columns (`encounter_name`, `turns_taken`) live in `combats` rather than as mostly-NULL columns on `floors`.
- **Run-wide floor numbers** — floors count 1…N across the whole run, so `(run_id, floor_number)` is unique without a third key column.
- **NULLs** — `killed_by_encounter` / `killed_by_event` are NULL when not applicable (empty CSV fields load as NULL).
- **Abandoned runs** are excluded from most win-rate queries; runs quit at the opening event would otherwise count as losses.

### Load it

Requires PostgreSQL with `psql` on your PATH.

```bash
./sts_tracker          # writes output/raw/*.csv
./load_db.sh           # creates database "sts2", builds tables, loads CSVs
psql -d sts2 -f sql/queries.sql
```

### Example queries (`sql/queries.sql`)

1. Win rate by character, with an overall row (`GROUP BY ROLLUP`, `FILTER`)
2. Win rate by ascension level
3. Top 3 deadliest encounters per act (`RANK() OVER (PARTITION BY …)`)
4. Hardest fights by average damage taken (3-table join)
5. Card pick rate and win rate — reproduces the C++ program's numbers in SQL
6. Cards that most improve win rate vs. the character's baseline (CTEs)
7. Average HP by floor, winning vs. losing runs
8. Running total of wins over time (window function)

## Technologies

- **C++17** — structured bindings, `std::filesystem`, STL containers, lambdas
- **[nlohmann/json](https://github.com/nlohmann/json)** — JSON parsing
- **PostgreSQL** — normalized schema, foreign keys, composite keys, check constraints, indexes, CTEs, window functions
