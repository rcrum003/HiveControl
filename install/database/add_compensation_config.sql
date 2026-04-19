--
-- Adds weight compensation columns to hiveconfig table.
-- Run this on existing installations to add environmental compensation support.
--
-- NOTE: SQLite ALTER TABLE ADD COLUMN will error if the column already exists.
-- Run each statement individually and ignore "duplicate column" errors, or
-- use the compute_compensation.py script which handles this automatically.
--

-- Weight environmental compensation coefficients
ALTER TABLE hiveconfig ADD COLUMN WEIGHT_TEMP_COEFF REAL DEFAULT 0;
ALTER TABLE hiveconfig ADD COLUMN WEIGHT_HUMIDITY_COEFF REAL DEFAULT 0;
ALTER TABLE hiveconfig ADD COLUMN WEIGHT_REF_TEMP REAL DEFAULT NULL;
ALTER TABLE hiveconfig ADD COLUMN WEIGHT_REF_HUMIDITY REAL DEFAULT NULL;
ALTER TABLE hiveconfig ADD COLUMN WEIGHT_COMPENSATION_ENABLED TEXT DEFAULT 'no';
ALTER TABLE hiveconfig ADD COLUMN WEIGHT_MONITOR_INTERVAL INTEGER DEFAULT 15;
