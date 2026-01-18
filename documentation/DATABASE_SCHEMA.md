# Database Schema

**HiveControl SQLite Database Documentation**
Version 2.10 - 2026-01-17

---

## Overview

HiveControl uses SQLite 3 as its database engine. The database file is located at `/home/HiveControl/data/hive-data.db` and contains all sensor readings, configuration, logs, and system data.

**Database File**: `/home/HiveControl/data/hive-data.db`
**Format**: SQLite 3
**Typical Size**: 26MB - 60MB+ (depending on data collection duration)

---

## Core Tables

### allhivedata

**Purpose**: Stores all sensor readings and measurements

**Columns**:

| Column | Type | Description |
|--------|------|-------------|
| `id` | INTEGER PRIMARY KEY | Auto-increment record ID |
| `date` | TEXT | Timestamp (YYYY-MM-DD HH:MM:SS) |
| `hivetempf` | REAL | Hive internal temperature (Fahrenheit) |
| `hivetempc` | REAL | Hive internal temperature (Celsius) |
| `hiveHum` | REAL | Hive internal humidity (%) |
| `hivedew` | REAL | Hive dew point |
| `hiveweight` | REAL | Net hive weight (pounds) |
| `hiverawweight` | REAL | Gross hive weight (pounds) |
| `outsidetemp` | REAL | External temperature (Fahrenheit) |
| `outsideHum` | REAL | External humidity (%) |
| `pressure` | REAL | Barometric pressure (inHg) |
| `rain` | REAL | Rainfall (inches) |
| `wind` | REAL | Wind speed (mph) |
| `winddir` | TEXT | Wind direction |
| `solar` | REAL | Solar radiation (W/m²) |
| `uvindex` | REAL | UV index |
| `luxlevel` | REAL | Light level (lux) |
| `IN_COUNT` | INTEGER | Bees entering hive (count) |
| `OUT_COUNT` | INTEGER | Bees leaving hive (count) |
| `pm25` | REAL | PM2.5 air quality (µg/m³) |
| `pm10` | REAL | PM10 air quality (µg/m³) |
| `gdd` | REAL | Growing degree days |
| `pollen` | INTEGER | Pollen count |

**Indexes**:
```sql
CREATE INDEX idx_date ON allhivedata(date);
```

**Example Query**:
```sql
SELECT date, hiveweight, hivetempf, hiveHum
FROM allhivedata
WHERE date >= datetime('now', '-7 days')
ORDER BY date DESC;
```

**Data Retention**: No automatic cleanup; manually managed via admin interface

---

### hiveconfig

**Purpose**: System configuration and settings

**Columns**:

| Column | Type | Description |
|--------|------|-------------|
| `id` | INTEGER PRIMARY KEY | Always 1 (single row table) |
| `hivename` | TEXT | Hive display name |
| `hiveapi` | TEXT | API key for external access |
| `yardid` | TEXT | Yard identifier |
| `city` | TEXT | Location city |
| `state` | TEXT | Location state/province |
| `country` | TEXT | Location country |
| `latitude` | REAL | GPS latitude |
| `longitude` | REAL | GPS longitude |
| `version` | INTEGER | Configuration version (auto-increment) |
| `timezone` | TEXT | Timezone (e.g., America/New_York) |
| `share_hivetool` | TEXT | Share with Hivetool.org (yes/no) |
| `HT_USERNAME` | TEXT | Hivetool.org username |
| `HT_PASSWORD` | TEXT | Hivetool.org password |
| `HT_URL` | TEXT | Hivetool.org upload URL |
| `GDD_BASE_TEMP` | INTEGER | Base temperature for GDD calculation |
| `GDD_START_DATE` | TEXT | GDD calculation start date |
| `POWER` | TEXT | Power status |
| `INTERNET` | TEXT | Internet status |
| `STATUS` | TEXT | System status |
| `COMPUTER` | TEXT | Computer type |
| `START_DATE` | TEXT | System start date |
| `ZIP` | TEXT | ZIP/postal code |
| `ENABLE_HIVE_WEIGHT_CHK` | TEXT | Enable weight sensor (yes/no) |
| `ENABLE_HIVE_TEMP_CHK` | TEXT | Enable temp sensor (yes/no) |
| `ENABLE_HIVE_CAMERA` | TEXT | Enable camera (yes/no) |
| `ENABLE_LUX` | TEXT | Enable light sensor (yes/no) |
| `ENABLE_BEECOUNTER` | TEXT | Enable bee counter (yes/no) |
| `SCALETYPE` | TEXT | Weight sensor type (hx711/phidget/cpw200) |
| `TEMPTYPE` | TEXT | Temp sensor type (dht22/sht31d/bme680/etc) |
| `LUX_SOURCE` | TEXT | Light sensor type (tsl2591/tsl2561) |
| `CAMERATYPE` | TEXT | Camera type |
| `COUNTERTYPE` | TEXT | Bee counter type |
| `HIVE_TEMP_GPIO` | INTEGER | GPIO pin for temp sensor |
| `hx711_slope` | REAL | HX711 calibration slope |
| `hx711_zero` | REAL | HX711 calibration zero point |
| `chart_smoothing` | TEXT | Enable chart smoothing (yes/no) |
| `RUN` | TEXT | Data collection enabled (yes/no) |

**Example Query**:
```sql
SELECT hivename, latitude, longitude, timezone
FROM hiveconfig
WHERE id = 1;
```

**Important Notes**:
- Only one row exists (id=1)
- `version` increments on each update for change tracking
- `RUN='no'` pauses all data collection

---

### logs

**Purpose**: System audit trail and error logging

**Columns**:

| Column | Type | Description |
|--------|------|-------------|
| `date` | TEXT | Timestamp (YYYY-MM-DD HH:MM:SS) |
| `program` | TEXT | Source program/script name |
| `type` | TEXT | Log level (INFO/ERROR/WARNING/SUCCESS) |
| `message` | TEXT | Log message content |

**Indexes**:
```sql
CREATE INDEX idx_logs_date ON logs(date);
CREATE INDEX idx_logs_type ON logs(type);
```

**Example Queries**:

Get recent errors:
```sql
SELECT date, program, message
FROM logs
WHERE type = 'ERROR'
ORDER BY date DESC
LIMIT 50;
```

Get logs for specific program:
```sql
SELECT date, type, message
FROM logs
WHERE program = 'WEIGHT'
ORDER BY date DESC;
```

**Common Log Types**:
- `INFO` - Informational messages
- `ERROR` - Error conditions
- `WARNING` - Warning conditions
- `SUCCESS` - Successful operations

**Common Programs**:
- `MAIN` - currconditions.sh main script
- `WEIGHT` - Weight sensor operations
- `TEMP` - Temperature sensor operations
- `WEATHER` - Weather data collection
- `BACKUP` - Backup operations
- `PASSWORD` - Password changes
- `SYSTEM` - System commands

---

### msgqueue

**Purpose**: System command queue for background operations

**Columns**:

| Column | Type | Description |
|--------|------|-------------|
| `id` | INTEGER PRIMARY KEY | Auto-increment message ID |
| `date` | TEXT | Message creation timestamp |
| `message` | TEXT | Command/message content |
| `status` | TEXT | Status (new/processing/completed/error) |

**Example Query**:
```sql
SELECT id, message, status, date
FROM msgqueue
WHERE status = 'new'
ORDER BY date ASC;
```

**Common Messages**:
- `upgrade` - System upgrade request
- `backup` - Backup creation request
- `export` - Data export request

**Status Flow**:
1. `new` - Message created, waiting for processing
2. `processing` - Currently being processed
3. `completed` - Successfully completed
4. `error` - Failed with error

---

### notifications

**Purpose**: Alert/notification configuration

**Columns**:

| Column | Type | Description |
|--------|------|-------------|
| `id` | INTEGER PRIMARY KEY | Notification rule ID |
| `name` | TEXT | Rule name |
| `measure` | TEXT | Measurement to monitor |
| `threshold_type` | TEXT | Comparison type (above/below/change) |
| `threshold_value` | REAL | Threshold value |
| `time_period` | TEXT | Time period for evaluation |
| `status` | TEXT | Rule status (active/inactive) |

**Example Query**:
```sql
SELECT name, measure, threshold_value
FROM notifications
WHERE status = 'active';
```

**Note**: Notification system is a placeholder feature in v2.10

---

### siteconfig

**Purpose**: Site-specific sensor configuration

**Columns**:

| Column | Type | Description |
|--------|------|-------------|
| `id` | INTEGER PRIMARY KEY | Always 1 (single row table) |
| `weather_level` | TEXT | Weather station detail level |
| `weather_detail` | TEXT | Weather station type |
| `key` | TEXT | Weather API key |
| `wxstation` | TEXT | Weather station identifier |
| `WXTEMPTYPE` | TEXT | Weather temp sensor type |
| `WX_TEMPER_DEVICE` | TEXT | Weather sensor device path |
| `WX_TEMP_GPIO` | INTEGER | Weather sensor GPIO pin |
| `local_wx_type` | TEXT | Local weather station type |
| `local_wx_url` | TEXT | Local weather station URL |
| `ambient_api_key` | TEXT | Ambient Weather API key |
| `ambient_app_key` | TEXT | Ambient Weather app key |
| `ambient_mac` | TEXT | Ambient Weather MAC address |
| `purpleair_id` | TEXT | PurpleAir station ID |
| `pollen_location` | TEXT | Pollen.com location code |

**Example Query**:
```sql
SELECT local_wx_type, ambient_api_key, purpleair_id
FROM siteconfig
WHERE id = 1;
```

---

## Database Operations

### Connection (PHP)

```php
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
// $conn is now available
```

### Connection (Bash)

```bash
DATABASE="/home/HiveControl/data/hive-data.db"
sqlite3 "$DATABASE" "SELECT * FROM hiveconfig WHERE id=1;"
```

### Insert Data

```php
$stmt = $conn->prepare("
    INSERT INTO allhivedata (date, hiveweight, hivetempf, hiveHum)
    VALUES (?, ?, ?, ?)
");
$stmt->execute([$date, $weight, $temp, $humidity]);
```

### Update Configuration

```php
$stmt = $conn->prepare("
    UPDATE hiveconfig
    SET hivename = ?, latitude = ?, longitude = ?
    WHERE id = 1
");
$stmt->execute([$name, $lat, $lon]);
```

### Query Time Range

```php
$stmt = $conn->prepare("
    SELECT date, hiveweight, hivetempf
    FROM allhivedata
    WHERE date >= datetime('now', '-7 days')
    ORDER BY date ASC
");
$stmt->execute();
$data = $stmt->fetchAll(PDO::FETCH_ASSOC);
```

---

## Database Maintenance

### Backup

**Via Web Interface**:
- Navigate to Admin → Backup & Restore
- Click "Create Full Backup"

**Via Command Line**:
```bash
# Copy database file
sudo cp /home/HiveControl/data/hive-data.db \
        /home/HiveControl/data/backups/backup_$(date +%Y%m%d_%H%M%S).db
```

### Restore

**Via Web Interface**:
- Navigate to Admin → Backup & Restore
- Select backup file
- Click "Restore"

**Via Command Line**:
```bash
# Stop data collection first
sudo systemctl stop cron

# Restore backup
sudo cp /home/HiveControl/data/backups/backup.db \
        /home/HiveControl/data/hive-data.db

# Fix permissions
sudo chown www-data:www-data /home/HiveControl/data/hive-data.db

# Restart data collection
sudo systemctl start cron
```

### Vacuum (Optimize)

```bash
sqlite3 /home/HiveControl/data/hive-data.db "VACUUM;"
```

### Integrity Check

```bash
sqlite3 /home/HiveControl/data/hive-data.db "PRAGMA integrity_check;"
```

### Clear Old Data

**Via Web Interface**:
- Admin → System Commands → Clear All Data

**Via SQL**:
```sql
-- Delete data older than 1 year
DELETE FROM allhivedata
WHERE date < datetime('now', '-365 days');

-- Remove zero values for a specific column
DELETE FROM allhivedata
WHERE hiveweight = 0;
```

---

## Database Schema Evolution

### Version 2.10

**New Tables**: None

**Modified Tables**:
- `hiveconfig` - Added `chart_smoothing` column
- `logs` - No changes (security fixes in application code)

**Security Improvements**:
- All queries use parameterized statements
- No SQL injection vulnerabilities
- Proper error handling

### Version 2.09 and Earlier

See [RELEASE](../RELEASE) file for historical changes.

---

## Performance Considerations

### Indexes

The database uses indexes on frequently queried columns:
- `allhivedata.date` - For time-range queries
- `logs.date` - For log retrieval
- `logs.type` - For error filtering

### WAL Mode

SQLite WAL (Write-Ahead Logging) mode is enabled for better concurrency:

```bash
sqlite3 /home/HiveControl/data/hive-data.db "PRAGMA journal_mode=WAL;"
```

### Query Optimization

**Good** - Use indexed columns:
```sql
SELECT * FROM allhivedata
WHERE date >= '2026-01-01'
ORDER BY date;
```

**Bad** - Full table scan:
```sql
SELECT * FROM allhivedata
WHERE hivetempf > 70;  -- No index on hivetempf
```

---

## Troubleshooting

### Database Locked

**Cause**: Multiple processes accessing database simultaneously

**Solution**:
```bash
# Check for processes using database
lsof /home/HiveControl/data/hive-data.db

# Wait a moment and retry
```

### Corruption

**Symptoms**: "database disk image is malformed"

**Solution**:
```bash
# Try to recover
sqlite3 /home/HiveControl/data/hive-data.db ".recover" | \
    sqlite3 recovered.db

# Restore from backup if recovery fails
```

### Permissions

**Issue**: "unable to open database file"

**Solution**:
```bash
sudo chown www-data:www-data /home/HiveControl/data/hive-data.db
sudo chmod 664 /home/HiveControl/data/hive-data.db
```

---

## See Also

- [Web Application Structure](WEB_APPLICATION.md)
- [Bash Scripts Reference](BASH_SCRIPTS.md)
- [API Reference](API.md)
- [Backup & Restore](BACKUP_FEATURE.md)
