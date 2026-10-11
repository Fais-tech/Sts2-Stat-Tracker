#!/usr/bin/env bash
# Rebuild the sts2 Postgres database from output/raw/*.csv
# Usage (from the project root):  ./load_db.sh        or  ./load_db.sh mydbname
set -e
DB="${1:-sts2}"

createdb "$DB" 2>/dev/null || true          # no-op if it already exists
psql -d "$DB" -q -f sql/schema.sql          # drop + recreate tables
psql -d "$DB" -f sql/load.sql               # load CSVs, print row counts
echo "Done. Try:  psql -d $DB -f sql/queries.sql"
