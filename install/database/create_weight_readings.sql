--
-- Creates the weight_readings table for high-frequency weight sampling.
-- Run this on existing installations to add the table.
-- Safe to run multiple times (uses IF NOT EXISTS).
--

CREATE TABLE IF NOT EXISTS weight_readings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    hiveid TEXT,
    date TEXT,
    date_utc TEXT,
    raw_weight REAL,
    net_weight REAL,
    compensated_weight REAL,
    ambient_temp_f REAL,
    ambient_humidity REAL,
    temp_source TEXT DEFAULT 'weather_cache',
    sample_count INTEGER DEFAULT 50,
    sync INTEGER DEFAULT 0
);

CREATE INDEX IF NOT EXISTS idx_weight_readings_date ON weight_readings(date);
CREATE INDEX IF NOT EXISTS idx_weight_readings_hiveid ON weight_readings(hiveid);
CREATE INDEX IF NOT EXISTS idx_weight_readings_date_utc ON weight_readings(date_utc);
