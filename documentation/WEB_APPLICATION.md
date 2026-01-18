# Web Application Architecture

**HiveControl Web Application Documentation**
Version 2.10 - 2026-01-17

---

## Table of Contents

- [Overview](#overview)
- [Directory Structure](#directory-structure)
- [Application Flow](#application-flow)
- [Admin Pages](#admin-pages)
- [User Pages](#user-pages)
- [Include Files](#include-files)
- [Chart System](#chart-system)
- [Database Integration](#database-integration)
- [Security Implementation](#security-implementation)

---

## Overview

The HiveControl web application is a PHP-based dashboard for monitoring and managing beehive sensors. It provides:

- **Real-time Data Visualization**: Interactive charts using HighCharts
- **Configuration Interface**: Web-based setup for sensors and system settings
- **Administrative Tools**: Backup, restore, password management, system commands
- **Mobile Responsive**: Bootstrap 3 framework for desktop and mobile

**Technology Stack:**
- **Backend**: PHP 7.0+, SQLite 3
- **Frontend**: Bootstrap 3, jQuery 3.x, HighCharts
- **Web Server**: Apache 2.4 with .htaccess authentication
- **Session Management**: PHP sessions with CSRF protection

---

## Directory Structure

```
/home/HiveControl/www/public_html/
├── admin/                          # Administrative interface
│   ├── backup.php                  # Backup & restore system
│   ├── changepassword.php          # Password management
│   ├── hiveconfig.php              # Basic hive configuration
│   ├── hiveapisetup.php            # API key management
│   ├── hivebodyconfig.php          # Hive body/frame tracking
│   ├── instrumentconfig.php        # Sensor configuration
│   ├── siteconfig.php              # Weather/sensor site settings
│   ├── system.php                  # System commands & logs
│   ├── notifications.php           # Email alert configuration
│   ├── notify.php                  # Notification handler
│   ├── livevalue.php               # Real-time sensor readings
│   ├── statusrefresh.php           # Upgrade status AJAX
│   └── api.php                     # REST API endpoint
├── pages/                          # User-facing dashboard
│   ├── index.php                   # Main dashboard (all data)
│   ├── weight.php                  # Weight chart view
│   ├── temp.php                    # Temperature chart view
│   ├── environment.php             # Environment data view
│   ├── gdd.php                     # Growing degree days
│   ├── light.php                   # LUX/solar data
│   ├── air.php                     # Air quality data
│   ├── beecount.php                # Bee counter data
│   ├── video.php                   # Webcam view
│   ├── video-w-wxforecast.php      # Webcam + weather
│   └── status.php                  # System status
├── include/                        # Shared PHP files
│   ├── db-connect.php              # Database connection
│   ├── security-init.php           # Session & CSRF security
│   ├── navigation.php              # Sidebar menu
│   ├── check.inc                   # Data validation functions
│   ├── loglocal.inc                # Logging functions
│   ├── curl.inc                    # HTTP request wrapper
│   └── cloud.inc                   # Hivetool.org integration
├── charts/                         # Chart data API endpoints
│   ├── getchartdata.php            # Main chart data provider
│   ├── weight-chart-data.php       # Weight-specific data
│   ├── temp-chart-data.php         # Temperature data
│   └── ...                         # Other chart endpoints
├── bower_components/               # Frontend libraries
│   ├── bootstrap/                  # Bootstrap 3
│   ├── jquery/                     # jQuery
│   ├── datatables/                 # DataTables plugin
│   ├── highcharts/                 # Charting library
│   └── ...
├── dist/                           # Compiled CSS/JS
│   ├── css/sb-admin-2.css          # Admin theme
│   └── js/sb-admin-2.js            # Admin JS
├── .htaccess                       # Apache authentication
├── index.html                      # Redirect to pages/
└── 401.php                         # Unauthorized error page
```

---

## Application Flow

### 1. Authentication Flow

```
User Request
    ↓
Apache .htaccess
    ↓
.htpasswd validation
    ↓
[Authenticated] → PHP Application
    ↓
security-init.php (session, CSRF)
    ↓
Page Content
```

**Authentication Method:**
- Apache Basic Authentication
- Credentials stored in `/home/HiveControl/www/.htpasswd`
- Default username: `admin`
- Password hashed with bcrypt (APR1 MD5 for legacy)

### 2. Page Rendering Flow

```
index.php
    ↓
include security-init.php (CSRF tokens, session)
    ↓
include db-connect.php (SQLite connection)
    ↓
include navigation.php (sidebar menu)
    ↓
Query database for config/data
    ↓
Render HTML with Bootstrap
    ↓
Load HighCharts with AJAX data
    ↓
charts/getchartdata.php (JSON endpoint)
```

### 3. Form Submission Flow

```
User submits form
    ↓
POST to admin page
    ↓
verify_csrf_token() check
    ↓
[Valid] → Process form
    ↓
Update database
    ↓
Log action to logs table
    ↓
Redirect/refresh page
```

---

## Admin Pages

### backup.php
**Purpose**: Database backup and restore system

**Features:**
- Full database backup (file copy method)
- Configuration-only backup (SQL export)
- Restore with safety backup
- Download backups
- Delete old backups

**Key Functions:**
```php
// Create full backup
$conn->exec("PRAGMA wal_checkpoint(TRUNCATE)");
copy($db_path, $backup_path);

// Create config backup
$result = $conn->query("SELECT * FROM hiveconfig");
// Generate INSERT SQL

// Restore backup
rename($db_path, $safety_backup);
copy($backup_path, $db_path);
```

**Security:**
- CSRF protection on all actions
- IP logging for audit trail
- .htaccess protection on backup directory
- Safety backup before restore

[Full Documentation](BACKUP_FEATURE.md)

### changepassword.php
**Purpose**: Admin password management

**Features:**
- Current password verification
- Support for multiple hash formats (APR1, bcrypt, SHA1, crypt)
- Minimum 8 character requirement
- Password confirmation

**Key Functions:**
```php
// Apache MD5 verification
function apr1_md5_verify($password, $hash)

// Hash formats supported:
- $apr1$ → Apache MD5
- $2y$/$2a$ → bcrypt
- {SHA} → SHA1
- others → traditional crypt()

// New password creation
$new_hash = password_hash($new_password, PASSWORD_BCRYPT);
file_put_contents($htpasswd_file, "$username:$new_hash\n");
```

**Security:**
- CSRF token required
- Current password must be verified
- All attempts logged with IP
- Password not exposed in forms

### system.php
**Purpose**: System commands and log viewing

**Commands:**
- `clearlogs` - Clear log table
- `cleardata` - Clear all sensor data
- `removezero` - Remove zero values from specific column
- `upgrade` - Trigger system upgrade
- `pause` - Pause data collection
- `server_status` - Display system info

**Key Functions:**
```php
function loglocal($date, $program, $type, $message) {
    $sth99 = $conn->prepare("INSERT INTO logs (date,program,type,message) VALUES (?,?,?,?)");
    $sth99->execute([$date, $program, $type, $message]);
}

function getlog($conn) {
    // Returns last 1000 log entries
    // Escaped with htmlspecialchars() to prevent XSS
}

function getUserIP() {
    // SECURITY: Only trust REMOTE_ADDR
    return $_SERVER['REMOTE_ADDR'];
}
```

**Security Fixes (v2.10):**
- SQL injection fixed in loglocal()
- SQL injection fixed in removezero with column whitelisting
- XSS fixed in getlog() output
- IP spoofing prevented in getUserIP()

### hiveconfig.php
**Purpose**: Basic hive configuration

**Settings:**
- Hive name, API key, location
- Timezone configuration
- Hivetool.org integration (username, password, URL)
- GDD (Growing Degree Days) settings
- Power/internet/status tracking

**Security Fixes (v2.10):**
- Password field no longer pre-filled
- Password only updated if new value provided
- All output escaped with htmlspecialchars()

**Form Behavior:**
- Auto-submit on change (`onchange="this.form.submit()"`)
- Conditional fields (Hivetool.org settings only show if sharing enabled)

### instrumentconfig.php
**Purpose**: Sensor selection and configuration

**Sensor Types:**
- Weight sensors (HX711, Phidget, CPW-200+)
- Temperature sensors (DHT22, SHT31-D, BME680, BroodMinder)
- Weather stations (WeatherFlow, Ambient Weather, Weather Underground)
- Light sensors (TSL2591, TSL2561)
- Air quality (PurpleAir)
- Bee counter

**Features:**
- Enable/disable sensors
- Calibration values (weight slope/zero)
- Sensor-specific configuration
- Live sensor testing

### siteconfig.php
**Purpose**: Site-specific sensor configuration

**Settings:**
- Weather station API keys
- PurpleAir station IDs
- Pollen.com location codes
- Network settings for local sensors

---

## User Pages

### index.php (Main Dashboard)
**Purpose**: Combined view of all sensor data

**Chart Types:**
- Line chart: Weight, temperature, humidity
- Column chart: Bee in/out counts
- Area chart: Rain
- Combined overlay

**Time Ranges:**
- Today (since midnight)
- 2 days
- 1 week
- 1 month
- 1 year
- All data

**Data Flow:**
```javascript
// AJAX request to chart endpoint
$.getJSON('charts/getchartdata.php?range=week', function(data) {
    // HighCharts configuration
    chart.series[0].setData(data.weight);
    chart.series[1].setData(data.temp);
    // ...
});
```

### Specialized Views

**weight.php** - Weight Chart
- Hive weight over time
- Identify nectar flows
- Track weight changes

**temp.php** - Temperature Chart
- Internal hive temperature
- External ambient temperature
- Temperature differential

**environment.php** - Environment Data
- Humidity (internal/external)
- Barometric pressure
- Weather conditions

**gdd.php** - Growing Degree Days
- Cumulative GDD calculation
- Pollen count overlay
- Bee development tracking

**light.php** - Light/Solar Data
- LUX levels
- Solar radiation
- Daylight patterns

**air.php** - Air Quality
- PM2.5 and PM10 levels
- PurpleAir integration
- Pollution trends

**beecount.php** - Bee Flight Activity
- Bees in vs bees out
- Flight activity patterns
- Weather correlation

**video.php** - Webcam View
- Live camera feed
- Motion JPEG stream
- Optional weather forecast overlay

---

## Include Files

### db-connect.php
**Purpose**: Database connection management

```php
try {
    $conn = new PDO("sqlite:/home/HiveControl/data/hive-data.db");
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch(PDOException $e) {
    // SECURITY: Don't expose error details
    error_log("Database connection failed: " . $e->getMessage());
    die("Database connection error. Please contact administrator.");
}
```

**Usage:**
```php
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
// $conn is now available for queries
```

### security-init.php
**Purpose**: Session security and CSRF protection

**Features:**
- Secure session configuration
- CSRF token generation
- Session regeneration (every 5 minutes)
- Security headers

**Functions:**
```php
get_csrf_token()           // Get current CSRF token
csrf_field()               // Generate hidden input field
verify_csrf_token()        // Check if token is valid
require_csrf_token()       // Check token or die
```

**Session Settings:**
```php
ini_set('session.cookie_httponly', 1);      // Prevent JS access
ini_set('session.cookie_samesite', 'Strict'); // CSRF protection
ini_set('session.use_strict_mode', 1);       // Reject bad IDs
ini_set('session.gc_maxlifetime', 3600);     // 1 hour timeout
```

**Security Headers:**
```php
X-Frame-Options: DENY
X-Content-Type-Options: nosniff
Referrer-Policy: strict-origin-when-cross-origin
Content-Security-Policy: default-src 'self'; ...
```

### navigation.php
**Purpose**: Sidebar menu and page header

**Sections:**
- Hive Dashboards (main, weight, temp, environment, etc.)
- Admin Pages (configuration, backups, system)
- Settings (notifications, API)

**Dynamic Menu:**
- Highlights current page
- Responsive collapse on mobile
- Bootstrap sidebar component

### check.inc
**Purpose**: Data validation functions

```bash
# Validate numeric sensor readings
function check() {
    # Rejects negative numbers
    # Rejects invalid characters (----)
    # Returns value or "null"
}
```

### cloud.inc
**Purpose**: Hivetool.org integration

**Functions:**
- Send data to Hivetool.org
- XML generation for data sharing
- Credential management
- Error handling

---

## Chart System

### getchartdata.php
**Purpose**: Main chart data API endpoint

**Parameters:**
- `range`: time|2days|week|month|year|all
- `sensor`: filter by sensor type (optional)

**Output Format:**
```json
{
    "weight": [[timestamp, value], ...],
    "temp": [[timestamp, value], ...],
    "humidity": [[timestamp, value], ...],
    "beein": [[timestamp, value], ...],
    "beeout": [[timestamp, value], ...],
    "rain": [[timestamp, value], ...]
}
```

**SQL Query Example:**
```php
$stmt = $conn->prepare("
    SELECT
        strftime('%s', date) * 1000 as timestamp,
        hiveweight,
        hivetempf,
        hiveHum
    FROM allhivedata
    WHERE date >= ?
    ORDER BY date ASC
");
$stmt->execute([$start_date]);
```

### HighCharts Integration

```javascript
var chart = new Highcharts.Chart({
    chart: {
        type: 'line',
        zoomType: 'x'
    },
    title: {
        text: 'Hive Data'
    },
    xAxis: {
        type: 'datetime'
    },
    yAxis: [{
        title: { text: 'Weight (lbs)' }
    }, {
        title: { text: 'Temperature (°F)' },
        opposite: true
    }],
    series: [{
        name: 'Weight',
        data: weightData,
        yAxis: 0
    }, {
        name: 'Temperature',
        data: tempData,
        yAxis: 1
    }]
});
```

---

## Database Integration

### Connection Pattern

```php
// Include database connection
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Execute query with parameterized statement
$stmt = $conn->prepare("SELECT * FROM hiveconfig WHERE id = ?");
$stmt->execute([1]);
$result = $stmt->fetch(PDO::FETCH_ASSOC);
```

### Common Queries

**Get Configuration:**
```php
$stmt = $conn->prepare("SELECT * FROM hiveconfig WHERE id=1");
$stmt->execute();
$config = $stmt->fetch(PDO::FETCH_ASSOC);
```

**Get Latest Sensor Reading:**
```php
$stmt = $conn->prepare("
    SELECT * FROM allhivedata
    ORDER BY date DESC LIMIT 1
");
$stmt->execute();
$latest = $stmt->fetch(PDO::FETCH_ASSOC);
```

**Insert Log Entry:**
```php
$stmt = $conn->prepare("
    INSERT INTO logs (date,program,type,message)
    VALUES (?,?,?,?)
");
$stmt->execute([$date, $program, $type, $message]);
```

**Time-Range Query:**
```php
$stmt = $conn->prepare("
    SELECT * FROM allhivedata
    WHERE date >= datetime('now', '-7 days')
    ORDER BY date ASC
");
$stmt->execute();
$data = $stmt->fetchAll(PDO::FETCH_ASSOC);
```

---

## Security Implementation

### CSRF Protection

**In Forms:**
```php
<form method="POST" action="?">
    <?php echo csrf_field(); ?>
    <!-- form fields -->
</form>
```

**In Handlers:**
```php
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    require_csrf_token();  // Dies if token invalid
    // Process form
}
```

### Output Escaping

**Always escape user-controlled data:**
```php
// WRONG (XSS vulnerability):
echo '<td>' . $row['message'] . '</td>';

// CORRECT:
echo '<td>' . htmlspecialchars($row['message']) . '</td>';
```

### SQL Injection Prevention

**Always use parameterized queries:**
```php
// WRONG (SQL injection vulnerability):
$query = "SELECT * FROM logs WHERE type='$type'";
$conn->query($query);

// CORRECT:
$stmt = $conn->prepare("SELECT * FROM logs WHERE type=?");
$stmt->execute([$type]);
```

### Input Validation

```php
function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}

// Usage:
$hivename = test_input($_POST["hivename"]);
```

### Whitelist Validation

```php
// For dynamic column names in SQL
$allowed_columns = ['hivetempf', 'hiveHum', 'hiveweight', 'IN_COUNT'];
if (!in_array($column, $allowed_columns)) {
    die("Invalid column name");
}
```

---

## Common Patterns

### Page Template

```php
<?php
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
include($_SERVER["DOCUMENT_ROOT"] . "/include/security-init.php");

// Process form submissions
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    require_csrf_token();
    // Handle POST data
}

// Query database
$stmt = $conn->prepare("SELECT * FROM hiveconfig WHERE id=1");
$stmt->execute();
$config = $stmt->fetch(PDO::FETCH_ASSOC);
?>
<!DOCTYPE html>
<html>
<?php include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
<div class="row">
    <div class="col-lg-12">
        <h1 class="page-header">Page Title</h1>
    </div>
</div>
<!-- Page content -->
</html>
```

### AJAX Data Endpoint

```php
<?php
header('Content-Type: application/json');
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$range = $_GET['range'] ?? 'week';

// Query data
$stmt = $conn->prepare("SELECT * FROM allhivedata WHERE date >= ?");
$stmt->execute([$start_date]);
$data = $stmt->fetchAll(PDO::FETCH_ASSOC);

// Format for HighCharts
$output = [];
foreach ($data as $row) {
    $timestamp = strtotime($row['date']) * 1000;
    $output[] = [$timestamp, floatval($row['hiveweight'])];
}

echo json_encode($output);
```

---

## File Reference Quick Guide

| File | Purpose | Security Features |
|------|---------|-------------------|
| `backup.php` | Database backup/restore | CSRF, IP logging |
| `changepassword.php` | Password management | CSRF, hash verification |
| `system.php` | System commands | CSRF, SQL injection fixed |
| `hiveconfig.php` | Basic configuration | Password not pre-filled |
| `instrumentconfig.php` | Sensor setup | Input validation |
| `siteconfig.php` | Site settings | Parameterized queries |
| `security-init.php` | Security initialization | Session security, CSRF |
| `db-connect.php` | Database connection | Error logging, no disclosure |
| `getchartdata.php` | Chart data API | Parameterized queries |

---

## Performance Considerations

**Database:**
- SQLite WAL mode enabled for better concurrency
- Indexes on `date` column for fast time-range queries
- Regular VACUUM for database maintenance

**Frontend:**
- Chart data cached by browser
- AJAX loading for charts (page loads faster)
- Minified CSS/JS in production

**PHP:**
- Execution time limits increased for large backups (300s)
- Memory limit increased for file operations (256M)
- Session garbage collection configured (1 hour)

---

## Troubleshooting

### Blank White Page
- Check PHP error log: `/var/log/apache2/error.log`
- Verify database exists and is readable
- Check file permissions (www-data needs read/write)

### Charts Not Loading
- Open browser console (F12) for JavaScript errors
- Check chart data endpoint: `charts/getchartdata.php`
- Verify database has data: `SELECT COUNT(*) FROM allhivedata;`

### CSRF Token Errors
- Clear browser cookies
- Check session directory is writable: `/var/lib/php/sessions`
- Verify security-init.php is included

### Authentication Fails
- Check .htpasswd file exists: `/home/HiveControl/www/.htpasswd`
- Verify password hash format
- Use changepassword.php to reset

---

## Version History

**Version 2.10** (2026-01-17)
- Added backup.php (backup & restore system)
- Added changepassword.php (password management)
- Added security-init.php (CSRF & session security)
- Fixed SQL injection vulnerabilities
- Fixed XSS vulnerabilities
- Improved session security

**Version 2.09 and earlier**
- See [RELEASE](../RELEASE) file

---

## See Also

- [Database Schema](DATABASE_SCHEMA.md)
- [Security Features](SECURITY.md)
- [API Reference](API.md)
- [Bash Scripts Reference](BASH_SCRIPTS.md)
