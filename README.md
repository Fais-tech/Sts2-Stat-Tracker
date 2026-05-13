# Sts2-Stat-Tracker

A C++ command-line tool that parses run history files from **Slay the Spire 2** and generates detailed statistics across characters, cards, relics, badges, and enemy encounters.

## Features

- **Character Statistics** — win rate, total runs, and abandoned runs per character
- **Card Statistics** — pick rate and win rate for every card, grouped by class and rarity
- **Relic Statistics** — pick rate and win rate for every relic, grouped by rarity
- **Badge Statistics** — bronze, silver, and gold counts for all earned badges
- **Encounter Statistics** — average damage taken, average turns, and kill count per enemy encounter, grouped by act and difficulty

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

The `.run` files are JSON run exports from Slay the Spire 2. The program will process all `.run` files it finds and print a formatted statistics report to the terminal.

## Sample Output

```
Processed 97 run file(s).

========== Character Statistics ==========
  Ironclad    | Runs:  23 | Wins:   8 | Win Rate:  34.78% | Abandoned:   2
  Silent      | Runs:  18 | Wins:   5 | Win Rate:  27.78% | Abandoned:   1
  ...
  Overall     | Runs:  97 | Wins:  30 | Win Rate:  30.93% | Abandoned:   7

========== Card Statistics ==========

=== Ironclad ===

  -- Common --
  Anger        | Pick Rate:  42.10% | Picked:  8/ 19 | Win Rate:  37.50% | Wins:   3
  ...
```

## Objective

This project was built to demonstrate practical C++ concepts applied to real data processing:

- **STL Containers** — `unordered_map` for O(1) average-case stat lookups across 500+ cards, relics, and encounters; `vector` for dynamic sorted output lists
- **Structs and Inheritance** — base structs (`Card`, `Relic`, `Encounter`) are extended via inheritance into statistics structs that add tracking fields and computed methods
- **File I/O and Filesystem** — uses `std::filesystem` to iterate a directory and `std::ifstream` to batch-read JSON run files
- **JSON Parsing** — integrates the `nlohmann/json` library to deserialize structured game data from raw files
- **Sorting with Lambdas** — `std::sort` with custom lambda comparators to order output by class, rarity, act, and difficulty
- **String Manipulation** — `std::transform` and `substr` to normalize game IDs into display-friendly names
- **Formatted Output** — `std::iomanip` (`setw`, `setprecision`, `fixed`) to align tabular statistics output in the terminal
- **Header Guards** — all headers use `#ifndef`/`#define`/`#endif` to prevent duplicate inclusion

## Technologies

- **C++17** — structured bindings, `std::filesystem`, `if constexpr`
- **STL** — `unordered_map`, `vector`, `algorithm`, `iomanip`
- **[nlohmann/json](https://github.com/nlohmann/json)** — JSON parsing
