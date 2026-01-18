# Bash Scripts Reference

**HiveControl Data Collection Scripts Documentation**
Version 2.10 - 2026-01-17

---

## Table of Contents

- [Overview](#overview)
- [Script Architecture](#script-architecture)
- [Main Orchestrator](#main-orchestrator)
- [Sensor Scripts](#sensor-scripts)
- [Utility Scripts](#utility-scripts)
- [Configuration System](#configuration-system)
- [Error Handling](#error-handling)
- [Security Features](#security-features)

---

## Overview

HiveControl uses a collection of bash scripts to gather sensor data from physical hardware and store it in the SQLite database. The scripts run periodically via cron and are designed for robustness, error handling, and modularity.

**Key Design Principles:**
- **Fail-Fast**: Scripts exit immediately on errors (`set -eo pipefail`)
- **Modularity**: Separate scripts for each sensor type
- **Error Logging**: All errors logged to database or syslog
- **Security**: SQL/XML injection prevention, credential protection
- **Portability**: Configurable HOMEDIR for flexible installation paths

---

## Script Architecture

```
┌─────────────────────────────────────────────┐
│  cron (every 5 minutes)                     │
└──────────────────┬──────────────────────────┘
                   │
                   v
┌─────────────────────────────────────────────┐
│  currconditions.sh                          │
│  (Main Orchestrator)                        │
│  - Load configuration                       │
│  - Call sensor scripts                      │
│  - Aggregate data                           │
│  - Store in database                        │
│  - Send to Hivetool.org (optional)          │
└──────┬──────────┬──────────┬────────────────┘
       │          │          │
       v          v          v
   ┌───────┐  ┌───────┐  ┌───────┐
   │Weight │  │ Temp  │  │Weather│
   │Scripts│  │Scripts│  │Scripts│
   └───────┘  └───────┘  └───────┘
```

### Directory Structure

```
/home/HiveControl/scripts/
├── system/
│   ├── currconditions.sh       # Main data collection orchestrator
│   └── ...
├── weight/
│   ├── getweight.sh            # Weight collection dispatcher
│   ├── hx711.sh                # HX711 load cell reader
│   ├── hx711calibrate.sh       # HX711 calibration wizard
│   ├── phidget1046.sh          # Phidget bridge interface
│   └── cpw200plus.sh           # Industrial scale reader
├── temp/
│   ├── gettemp.sh              # Temperature dispatcher
│   ├── broodminder.sh          # BroodMinder BLE sensor
│   ├── broodtemp.sh            # Internal hive temp
│   ├── dht22.sh                # DHT22/AM2302 sensor
│   ├── sht31-d.sh              # SHT31-D I2C sensor
│   ├── bme680.sh               # BME680 multi-sensor
│   ├── bme280.sh               # BME280 sensor
│   └── temperhum.sh            # USB Temper sensor
├── weather/
│   ├── getWeatherFlowLocal.sh  # WeatherFlow Tempest (UDP)
│   ├── getAmbientWeather.sh    # Ambient Weather (API)
│   ├── getOurWeather.sh        # OurWeather stations
│   └── getWXunderground.sh     # Weather Underground (XML)
├── lux/
│   ├── tsl2591.sh              # TSL2591 light sensor
│   └── tsl2561.sh              # TSL2561 light sensor
├── air/
│   └── getair.sh               # PurpleAir quality data
├── beecounter/
│   └── beecounter.sh           # Optical bee counter
├── data/
│   ├── hiveconfig.sh           # Configuration loader
│   ├── check.inc               # Data validation functions
│   ├── logger.inc              # Logging functions
│   └── cloud.inc               # Hivetool.org integration
└── hiveconfig.inc              # Generated config variables
```

---

## Main Orchestrator

### currconditions.sh

**Purpose**: Main data collection script that orchestrates all sensor readings

**Version**: 2026011705

**Location**: `/home/HiveControl/scripts/system/currconditions.sh`

**Execution**: Every 5 minutes via cron

**Flow:**

```bash
1. Load configuration from database
2. Check if collection is paused (RUN variable)
3. Call sensor collection functions:
   - get_hive_weight()
   - get_hive_temp()
   - get_bee_count()
   - get_weather()
   - get_lux()
   - get_air_quality()
4. Aggregate all data
5. Insert into SQLite database
6. Send to Hivetool.org (if enabled)
7. Exit with status code
```

**Key Features:**

**Strict Mode (with compatibility):**
```bash
set -eo pipefail  # Exit on error, catch pipe failures
# Note: -u (unbound variables) not used for check.inc compatibility
```

**SQL Injection Prevention:**
```bash
function sql_escape() {
    echo "${1//\'/\'\'}"  # Double single quotes
}

# Usage:
HIVEWEIGHT_ESC=$(sql_escape "$HIVEWEIGHT")
sqlite3 "$DB" "INSERT INTO allhivedata (hiveweight) VALUES ('$HIVEWEIGHT_ESC')"
```

**XML Injection Prevention:**
```bash
function xml_escape() {
    local str="$1"
    str="${str//&/&amp;}"
    str="${str//</&lt;}"
    str="${str//>/&gt;}"
    str="${str//\"/&quot;}"
    str="${str//\'/&apos;}"
    echo "$str"
}
```

**Error Handling:**
```bash
function get_hive_weight() {
    if [ "$ENABLE_HIVE_WEIGHT_CHK" = "yes" ]; then
        local weight_output
        if ! weight_output=$("$HOMEDIR/scripts/weight/getweight.sh" 2>&1); then
            echo "ERROR: Failed to get weight data: $weight_output"
            HIVEWEIGHT="null"
            return 1
        fi
        # Parse and validate data
    fi
}

# Call with || true to continue on failure
get_hive_weight || true
get_hive_temp || true
get_weather || true
```

**Performance Optimization:**
```bash
# OLD: Multiple awk calls
TEMP=$(echo "$output" | awk '{print $1}')
HUM=$(echo "$output" | awk '{print $2}')
DEW=$(echo "$output" | awk '{print $3}')

# NEW: Single read operation
read -r TEMP HUM DEW <<< "$output"
```

**Database Operations:**
```bash
# Check database exists
if [ ! -f "$DATABASE" ]; then
    echo "ERROR: Database not found: $DATABASE"
    exit 1
fi

# Insert with error checking
if ! sqlite3 "$DATABASE" "INSERT INTO allhivedata (...) VALUES (...)"; then
    echo "ERROR: Database insert failed"
    loglocal "$DATE" MAIN ERROR "Failed to insert data into database"
    exit 1
fi

# Use last_insert_rowid() for efficiency
ROWID=$(sqlite3 "$DATABASE" "SELECT last_insert_rowid();")
```

### Version 2.10 Security Improvements

**Fixed in currconditions.sh:**
1. SQL injection in database inserts
2. XML injection in Hivetool.org data sharing
3. Hardcoded paths replaced with $HOMEDIR variable
4. Comprehensive error handling added
5. All variable expansions quoted
6. Password exposure in command line (use .netrc instead)

**Before (Vulnerable):**
```bash
# SQL injection risk
sqlite3 "$DB" "INSERT INTO logs VALUES (\"$DATE\",\"$PROGRAM\",\"$TYPE\",\"$MESSAGE\")"

# XML injection risk
echo "<temp>$TEMP</temp>" > file.xml

# Unquoted variables
cd $HOMEDIR
```

**After (Secure):**
```bash
# SQL injection prevented
MESSAGE_ESC=$(sql_escape "$MESSAGE")
sqlite3 "$DB" "INSERT INTO logs VALUES (\"$DATE\",\"$PROGRAM\",\"$TYPE\",\"$MESSAGE_ESC\")"

# XML injection prevented
TEMP_XML=$(xml_escape "$TEMP")
echo "<temp>$TEMP_XML</temp>" > file.xml

# Quoted variables
cd "$HOMEDIR"
```

---

## Sensor Scripts

### Weight Scripts

#### getweight.sh
**Purpose**: Dispatcher for weight sensor types

**Flow:**
1. Read `WEIGHT_TYPE` from configuration
2. Call appropriate sensor script
3. Return: `RAW_WEIGHT NET_WEIGHT`

**Supported Sensors:**
- `hx711` → hx711.sh
- `phidget` → phidget1046.sh
- `cpw200` → cpw200plus.sh

#### hx711.sh
**Purpose**: Read HX711 load cell amplifier

**Requirements:**
- Python 3 with pigpio library
- HX711 connected to GPIO pins
- Calibration values (slope, zero) from database

**Operation:**
```bash
# Get calibration from database
SLOPE=$(sqlite3 "$DB" "SELECT hx711_slope FROM hiveconfig WHERE id=1;")
ZERO=$(sqlite3 "$DB" "SELECT hx711_zero FROM hiveconfig WHERE id=1;")

# Read sensor (Python script)
OUTPUT=$(python3 "$HOMEDIR/scripts/weight/hx711.py" --dout "$DOUT_PIN" --pd_sck "$SCK_PIN")

# Calculate weight
RAW_WEIGHT=$(echo "$OUTPUT" | awk '{print $1}')
NET_WEIGHT=$(echo "scale=2; ($RAW_WEIGHT - $ZERO) * $SLOPE" | bc)

echo "$RAW_WEIGHT $NET_WEIGHT"
```

**Retry Logic:**
```bash
# Try up to 3 times if reading is zero
for i in {1..3}; do
    read_sensor
    if [ "$RAW_WEIGHT" != "0" ]; then
        break
    fi
    sleep 1
done
```

#### hx711calibrate.sh
**Purpose**: Interactive calibration wizard

**Steps:**
1. Read sensor with no weight (get zero point)
2. Place known weight on scale
3. Read sensor with known weight
4. Calculate slope: `slope = known_weight / (reading - zero)`
5. Store in database

### Temperature Scripts

#### gettemp.sh
**Purpose**: Dispatcher for temperature sensor types

**Flow:**
1. Read `TEMP_TYPE` from configuration
2. Call appropriate sensor script
3. Return: `TEMP_F TEMP_C HUMIDITY DEW_POINT`

**Supported Sensors:**
- `dht22` → dht22.sh
- `sht31d` → sht31-d.sh
- `bme680` → bme680.sh
- `bme280` → bme280.sh
- `broodminder` → broodminder.sh
- `temper` → temperhum.sh

#### dht22.sh
**Purpose**: Read DHT22/AM2302 humidity/temperature sensor

**Requirements:**
- Python with pigpio library
- DHT22 connected to GPIO pin

**Operation:**
```bash
# Read sensor using pigpio
OUTPUT=$(python3 "$HOMEDIR/scripts/temp/dht22.py" --gpio "$GPIO_PIN")

# Parse: "temp,humidity"
IFS=',' read -r TEMP_C HUMIDITY <<< "$OUTPUT"

# Convert to Fahrenheit
TEMP_F=$(echo "scale=1; ($TEMP_C * 9/5) + 32" | bc)

# Calculate dew point
DEW=$(calculate_dewpoint "$TEMP_C" "$HUMIDITY")

echo "$TEMP_F $TEMP_C $HUMIDITY $DEW"
```

**Error Handling:**
```bash
# Check for lock file (prevents concurrent access)
LOCKFILE="/tmp/dht22.lock"
if [ -f "$LOCKFILE" ]; then
    echo "ERROR: Sensor locked by another process"
    exit 1
fi

# Create lock
touch "$LOCKFILE"
trap "rm -f $LOCKFILE" EXIT

# Read sensor
# ... (lock automatically removed on exit)
```

#### sht31-d.sh
**Purpose**: Read SHT31-D I2C sensor (high precision)

**Requirements:**
- Python with Adafruit SHT31D library
- I2C enabled (`sudo raspi-config`)
- Sensor on I2C bus (default address 0x44)

#### bme680.sh
**Purpose**: Read BME680 sensor (temp/humidity/pressure/gas)

**Returns**: `TEMP_F TEMP_C HUMIDITY PRESSURE GAS_RESISTANCE`

### Weather Scripts

#### getWeatherFlowLocal.sh
**Purpose**: Read WeatherFlow Tempest weather station via UDP broadcast

**Version**: 7

**Operation:**
```bash
# Listen for UDP broadcasts on port 50222
# Parse JSON with jq
# Extract: temp, humidity, pressure, rain, wind, solar, UV
# Calculate dewpoint and heat index
# Return CSV format
```

**Error Handling (Version 7 fixes):**
```bash
# Suppress error messages
TEMP=$(echo "$JSON" | jq -r '.temp' 2>/dev/null)

# Set defaults before math operations
TEMP="${TEMP:-0}"

# Validate before calculations
if [ -z "$TEMP" ] || [ "$TEMP" = "null" ]; then
    # Return null CSV instead of failing
    echo "null,null,null,null,null,null,null"
    exit 0
fi
```

#### getAmbientWeather.sh
**Purpose**: Fetch data from Ambient Weather API

**Requirements:**
- API key and Application key from Ambient Weather
- Internet connection
- curl installed

**Operation:**
```bash
API_KEY=$(sqlite3 "$DB" "SELECT ambient_api_key FROM siteconfig;")
APP_KEY=$(sqlite3 "$DB" "SELECT ambient_app_key FROM siteconfig;")
MAC=$(sqlite3 "$DB" "SELECT ambient_mac FROM siteconfig;")

URL="https://api.ambientweather.net/v1/devices/$MAC"
URL+="?apiKey=$API_KEY&applicationKey=$APP_KEY&limit=1"

JSON=$(curl -s "$URL")

# Parse JSON and return CSV
```

#### getOurWeather.sh
**Purpose**: Read OurWeather stations

**Note**: Specific to jhalt/Duncan's weather station setup

#### getWXunderground.sh
**Purpose**: Weather Underground XML feed (legacy)

**Note**: IBM deprecated API access; XML still available

### Light Sensors

#### tsl2591.sh
**Purpose**: Read TSL2591 light sensor (high dynamic range)

**Returns**: LUX value

**Requirements:**
- Python with Adafruit TSL2591 library
- I2C enabled
- Sensor on I2C bus

#### tsl2561.sh
**Purpose**: Read TSL2561 light sensor

**Returns**: LUX value

### Air Quality Scripts

#### getair.sh
**Purpose**: Fetch PurpleAir air quality data

**Operation:**
```bash
SENSOR_ID=$(sqlite3 "$DB" "SELECT purpleair_id FROM siteconfig;")

URL="https://www.purpleair.com/json?show=$SENSOR_ID"
JSON=$(curl -s "$URL")

# Extract PM2.5 and PM10 values
PM25=$(echo "$JSON" | jq -r '.results[0].PM2_5Value')
PM10=$(echo "$JSON" | jq -r '.results[0].PM10Value')

echo "$PM25 $PM10"
```

### Bee Counter Scripts

#### beecounter.sh
**Purpose**: Read optical bee counter for flight activity

**Returns**: `BEES_IN BEES_OUT`

**Requirements:**
- Beecounter hardware installed
- Computer vision libraries
- Camera module

---

## Utility Scripts

### Configuration System

#### hiveconfig.sh
**Purpose**: Load configuration from database and export as shell variables

**Location**: `/home/HiveControl/scripts/data/hiveconfig.sh`

**Operation:**
```bash
# Query database
RESULT=$(sqlite3 "$DATABASE" "SELECT * FROM hiveconfig WHERE id=1;")

# Parse fields (pipe-delimited)
IFS='|' read -r ID HIVENAME HIVEAPI YARDID CITY STATE \
    COUNTRY LAT LON VERSION TIMEZONE SHARE_HIVETOOL \
    HT_USER HT_PASS HT_URL ... <<< "$RESULT"

# Export variables
export HIVENAME="$HIVENAME"
export LATITUDE="$LAT"
export LONGITUDE="$LON"
# ... (all config variables)

# Write to hiveconfig.inc for sourcing
cat > "$HOMEDIR/scripts/hiveconfig.inc" <<EOF
HIVENAME="$HIVENAME"
LATITUDE="$LAT"
LONGITUDE="$LON"
# ... (all variables)
EOF
```

**Used by**: All scripts that need configuration

#### check.inc
**Purpose**: Data validation functions

**Key Function:**
```bash
function check() {
    local var_name="$1"
    local value="${!var_name}"

    # Check for invalid values
    if [ -z "$value" ] || [ "$value" = "----" ] || [ "$value" = "null" ]; then
        eval "$var_name=\"null\""
        return 1
    fi

    # Check for negative numbers
    if [[ "$value" =~ ^- ]]; then
        eval "$var_name=\"null\""
        return 1
    fi

    return 0
}

# Usage:
TEMP=25.5
check TEMP  # Returns 0 (valid)

TEMP="----"
check TEMP  # Sets TEMP="null", returns 1
```

#### logger.inc
**Purpose**: Logging functions

**Functions:**
```bash
function loglocal() {
    local date="$1"
    local program="$2"
    local type="$3"
    local message="$4"

    # Escape for SQL
    message=$(sql_escape "$message")

    # Insert into logs table
    sqlite3 "$DATABASE" "INSERT INTO logs (date,program,type,message) \
        VALUES ('$date','$program','$type','$message')"
}

function logsyslog() {
    local message="$1"
    logger -t "HiveControl" "$message"
}

# Usage:
loglocal "$DATE" "WEIGHT" "INFO" "Weight reading: $WEIGHT lbs"
logsyslog "Data collection completed successfully"
```

#### cloud.inc
**Purpose**: Hivetool.org data sharing

**Functions:**
```bash
function send_to_hivetool() {
    if [ "$SHARE_HIVETOOL" != "yes" ]; then
        return 0
    fi

    # Build XML payload
    local xml="<data>"
    xml+="<hive_id>$(xml_escape "$HIVEAPI")</hive_id>"
    xml+="<datetime>$(xml_escape "$DATE_UTC")</datetime>"
    xml+="<temp>$(xml_escape "$HIVETEMPF")</temp>"
    xml+="<weight>$(xml_escape "$HIVEWEIGHT")</weight>"
    # ... (all fields)
    xml+="</data>"

    # POST to Hivetool.org
    # SECURITY NOTE: Use .netrc file for credentials, not command line
    curl -s --netrc-file "$HOME/.netrc" \
        -X POST \
        -d "$xml" \
        "https://hivetool.org/api/upload"
}
```

**Security Improvement:**
Instead of passing credentials in URL (visible in process list):
```bash
# OLD (INSECURE):
curl "https://user:pass@hivetool.org/api/upload"

# NEW (SECURE):
# Create ~/.netrc file:
# machine hivetool.org
# login your_username
# password your_password
#
# chmod 600 ~/.netrc

curl --netrc-file ~/.netrc "https://hivetool.org/api/upload"
```

---

## Error Handling

### Design Patterns

**Fail-Fast with Recovery:**
```bash
# Individual sensors can fail without stopping collection
get_hive_weight || true
get_hive_temp || true
get_weather || true

# But critical failures exit immediately
if ! sqlite3 "$DB" "INSERT INTO allhivedata ..."; then
    loglocal "$DATE" MAIN ERROR "Database insert failed"
    exit 1
fi
```

**Retry Logic:**
```bash
# Retry sensor readings up to 3 times
function read_with_retry() {
    local max_retries=3
    local attempt=1

    while [ $attempt -le $max_retries ]; do
        if OUTPUT=$(sensor_read 2>&1); then
            echo "$OUTPUT"
            return 0
        fi
        echo "Retry $attempt/$max_retries" >&2
        ((attempt++))
        sleep 1
    done

    return 1
}
```

**Default Values:**
```bash
# Always provide defaults for critical variables
TIMEZONE="${TIMEZONE:-UTC}"
HOMEDIR="${HOMEDIR:-/home/HiveControl}"
DATABASE="${DATABASE:-$HOMEDIR/data/hive-data.db}"
```

**Error Logging:**
```bash
# Log all errors to database
if ! result=$(command 2>&1); then
    loglocal "$DATE" "SENSOR" "ERROR" "Failed: $result"
    return 1
fi
```

---

## Security Features

### SQL Injection Prevention

**Always escape user-controlled data:**
```bash
# Get input
MESSAGE="User's input with 'quotes'"

# Escape for SQL
MESSAGE_ESC=$(sql_escape "$MESSAGE")

# Safe to use in query
sqlite3 "$DB" "INSERT INTO logs (message) VALUES ('$MESSAGE_ESC')"
```

### XML Injection Prevention

**Escape XML special characters:**
```bash
TEMP="25°F"
TEMP_XML=$(xml_escape "$TEMP")
echo "<temperature>$TEMP_XML</temperature>"
# Output: <temperature>25°F</temperature>
```

### Credential Protection

**Use .netrc for passwords:**
```bash
# ~/.netrc (chmod 600)
machine hivetool.org
    login your_username
    password your_password

# In script:
curl --netrc-file ~/.netrc https://hivetool.org/api/upload
```

**Never pass passwords on command line:**
```bash
# BAD (visible in process list):
curl "https://user:password@site.com"

# GOOD:
curl --netrc-file ~/.netrc https://site.com
```

### File System Security

**Check file existence:**
```bash
if [ ! -f "$CONFIG_FILE" ]; then
    echo "ERROR: Config file not found: $CONFIG_FILE"
    exit 1
fi
```

**Use absolute paths:**
```bash
# BAD (relative paths can be hijacked):
source config.inc

# GOOD:
source "$HOMEDIR/scripts/config.inc"
```

**Validate permissions:**
```bash
# Check database is writable
if [ ! -w "$DATABASE" ]; then
    echo "ERROR: Database not writable: $DATABASE"
    exit 1
fi
```

---

## Cron Configuration

**Default Schedule:**
```cron
# Run data collection every 5 minutes
*/5 * * * * /home/HiveControl/scripts/system/currconditions.sh >> /var/log/hivecontrol.log 2>&1

# Run system maintenance daily
0 2 * * * /home/HiveControl/scripts/system/maintenance.sh
```

**Installation:**
```bash
# During install.sh
crontab -l > /tmp/cron.bak 2>/dev/null || true
echo "*/5 * * * * /home/HiveControl/scripts/system/currconditions.sh" | crontab -
```

---

## Testing Scripts

### Manual Execution

```bash
# Run main script manually
sudo /home/HiveControl/scripts/system/currconditions.sh

# Test individual sensor
sudo /home/HiveControl/scripts/weight/getweight.sh
# Output: 12345 98.5

# Test with debugging
bash -x /home/HiveControl/scripts/system/currconditions.sh
```

### Validation

```bash
# Check database was updated
sqlite3 /home/HiveControl/data/hive-data.db \
    "SELECT * FROM allhivedata ORDER BY date DESC LIMIT 1;"

# Check logs
sqlite3 /home/HiveControl/data/hive-data.db \
    "SELECT * FROM logs ORDER BY date DESC LIMIT 10;"

# Check cron is running
sudo systemctl status cron

# Check for errors in logs
grep -i error /var/log/hivecontrol.log
```

---

## Troubleshooting

### Script Not Running

```bash
# Check cron is active
sudo systemctl status cron

# Check crontab entry
crontab -l | grep currconditions

# Check script permissions
ls -la /home/HiveControl/scripts/system/currconditions.sh
# Should be executable: -rwxr-xr-x

# Make executable if needed
chmod +x /home/HiveControl/scripts/system/currconditions.sh
```

### Sensor Not Reading

```bash
# Test sensor script directly
sudo /home/HiveControl/scripts/weight/hx711.sh
# Look for error messages

# Check Python dependencies
python3 -c "import pigpio; print('OK')"

# Check I2C is enabled (for I2C sensors)
sudo i2cdetect -y 1

# Check GPIO permissions
sudo usermod -a -G gpio www-data
```

### Database Errors

```bash
# Check database exists
ls -la /home/HiveControl/data/hive-data.db

# Check permissions
sudo chown www-data:www-data /home/HiveControl/data/hive-data.db

# Test database
sqlite3 /home/HiveControl/data/hive-data.db "PRAGMA integrity_check;"
```

### Weather Station Not Responding

```bash
# Test WeatherFlow UDP listener
sudo tcpdump -i any -n port 50222
# Should see packets if station is broadcasting

# Test Ambient Weather API
curl "https://api.ambientweather.net/v1/devices" \
    -G --data-urlencode "apiKey=YOUR_KEY" \
    --data-urlencode "applicationKey=YOUR_APP_KEY"
```

---

## Version History

**Version 2026011705** (2026-01-17)
- Fixed WeatherFlow error handling
- Added 2>/dev/null to bc and jq commands
- Return null CSV when no weather data

**Version 2026011704** (2026-01-17)
- Fixed getWeatherFlowLocal.sh error cascade
- Added early exit with null values when no data

**Version 2026011703** (2026-01-17)
- Fixed script termination on sensor failures
- Added || true to sensor function calls

**Version 2026011702** (2026-01-17)
- Fixed unbound variable errors
- Moved set -eo pipefail after config loading

**Version 2026011701** (2026-01-17)
- Major security and performance refactoring
- Fixed SQL injection vulnerabilities
- Fixed XML injection vulnerabilities
- Added comprehensive error handling
- Refactored into modular functions
- Optimized parsing (reduced awk calls)
- Fixed all hardcoded paths

---

## See Also

- [Web Application Structure](WEB_APPLICATION.md)
- [Database Schema](DATABASE_SCHEMA.md)
- [Security Features](SECURITY.md)
- [API Reference](API.md)
- [Configuration Reference](CONFIGURATION.md)
