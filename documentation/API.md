# API Reference

**HiveControl API Documentation**
Version 2.10 - 2026-01-17

---

## Table of Contents

- [Overview](#overview)
- [Authentication](#authentication)
- [Chart Data Endpoints](#chart-data-endpoints)
- [System Status Endpoints](#system-status-endpoints)
- [Data Export](#data-export)
- [Error Handling](#error-handling)
- [Rate Limiting](#rate-limiting)
- [Examples](#examples)

---

## Overview

HiveControl provides REST-ful JSON API endpoints for accessing sensor data, chart visualizations, and system status. All endpoints require HTTP Basic Authentication.

**Base URL**: `http://your-hive-ip/`

**Supported Formats**:
- JSON (primary)
- CSV (data export)
- JSONP (cross-domain requests)

**HTTP Methods**:
- `GET` - Retrieve data
- `POST` - Submit commands (admin only, CSRF token required)

---

## Authentication

All API requests require HTTP Basic Authentication using your admin credentials.

**Authentication Method**: HTTP Basic Auth

**Headers**:
```http
Authorization: Basic base64(username:password)
```

**Example with curl**:
```bash
curl -u admin:your_password http://your-hive-ip/pages/datawidgets/getdata.php?type=hiveweight&period=week&chart=all
```

**Example with JavaScript**:
```javascript
fetch('http://your-hive-ip/pages/datawidgets/getdata.php?type=hiveweight&period=week&chart=all', {
    headers: {
        'Authorization': 'Basic ' + btoa('admin:password')
    }
})
.then(response => response.json())
.then(data => console.log(data));
```

---

## Chart Data Endpoints

### GET /pages/datawidgets/getdata.php

**Description**: Retrieve sensor data for charting

**Parameters**:

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `type` | string | Yes | Data type to retrieve (see table below) |
| `period` | string | Yes | Time period (see table below) |
| `chart` | string | Yes | Chart type (all, weight, temp, environment, gdd, light, air, beecount) |
| `callback` | string | No | JSONP callback function name |

**Data Types (`type` parameter)**:

| Value | Description | Unit |
|-------|-------------|------|
| `hiveweight` | Hive weight (net) | pounds |
| `hiverawweight` | Hive weight (gross) | pounds |
| `hivetempf` | Hive temperature | Fahrenheit |
| `hivetempc` | Hive temperature | Celsius |
| `hiveHum` | Hive humidity | percent |
| `outsidetemp` | Outside temperature | Fahrenheit |
| `outsideHum` | Outside humidity | percent |
| `pressure` | Barometric pressure | inHg |
| `rain` | Rainfall | inches |
| `wind` | Wind speed | mph |
| `solar` | Solar radiation | W/m² |
| `uvindex` | UV index | index |
| `luxlevel` | Light level | lux |
| `IN_COUNT` | Bees entering hive | count |
| `OUT_COUNT` | Bees leaving hive | count |
| `pm25` | PM2.5 air quality | µg/m³ |
| `pm10` | PM10 air quality | µg/m³ |
| `gdd` | Growing degree days | degree-days |
| `pollen` | Pollen count | count |

**Time Periods (`period` parameter)**:

| Value | Description |
|-------|-------------|
| `time` | Today (since midnight) |
| `2days` | Last 2 days |
| `week` | Last 7 days |
| `month` | Last 30 days |
| `year` | Last 365 days |
| `all` | All available data |

**Response Format** (JSON):

```json
{
    "data": [
        [timestamp_ms, value],
        [timestamp_ms, value],
        ...
    ],
    "config": {
        "hivename": "My Hive",
        "smoothing": "yes"
    }
}
```

**Example Request**:
```http
GET /pages/datawidgets/getdata.php?type=hiveweight&period=week&chart=all
```

**Example Response**:
```json
{
    "data": [
        [1673971200000, 98.5],
        [1673971500000, 98.6],
        [1673971800000, 98.4]
    ],
    "config": {
        "hivename": "Backyard Hive #1",
        "smoothing": "yes"
    }
}
```

### GET /pages/datawidgets/all_chart.php

**Description**: Get combined data for main dashboard (weight, temp, humidity, bee counts)

**Parameters**:

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `period` | string | Yes | Time period (time, 2days, week, month, year, all) |

**Response Format**:

```json
{
    "weight": [[timestamp, value], ...],
    "temperature": [[timestamp, value], ...],
    "humidity": [[timestamp, value], ...],
    "beein": [[timestamp, value], ...],
    "beeout": [[timestamp, value], ...],
    "rain": [[timestamp, value], ...]
}
```

**Example Request**:
```http
GET /pages/datawidgets/all_chart.php?period=week
```

### GET /pages/datawidgets/weight_chart.php

**Description**: Get weight data with detailed metadata

**Parameters**:

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `period` | string | Yes | Time period |

**Response Format**:

```json
{
    "weight": [[timestamp, net_weight], ...],
    "raw_weight": [[timestamp, gross_weight], ...],
    "metadata": {
        "max_weight": 125.5,
        "min_weight": 95.2,
        "current_weight": 98.5,
        "weight_change_24h": -0.8
    }
}
```

### GET /pages/datawidgets/temp_chart.php

**Description**: Get temperature and humidity data

**Parameters**:

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `period` | string | Yes | Time period |

**Response Format**:

```json
{
    "hive_temp": [[timestamp, temp_f], ...],
    "outside_temp": [[timestamp, temp_f], ...],
    "hive_humidity": [[timestamp, humidity], ...],
    "outside_humidity": [[timestamp, humidity], ...]
}
```

### GET /pages/datawidgets/environment_chart.php

**Description**: Get environmental data (weather, pressure, etc.)

**Response Format**:

```json
{
    "temperature": [[timestamp, value], ...],
    "humidity": [[timestamp, value], ...],
    "pressure": [[timestamp, value], ...],
    "rain": [[timestamp, value], ...],
    "wind": [[timestamp, value], ...],
    "solar": [[timestamp, value], ...],
    "uv": [[timestamp, value], ...]
}
```

### GET /pages/datawidgets/gdd_chart.php

**Description**: Get growing degree days and pollen data

**Response Format**:

```json
{
    "gdd": [[timestamp, cumulative_gdd], ...],
    "pollen": [[timestamp, pollen_count], ...],
    "config": {
        "gdd_base_temp": 50,
        "gdd_start_date": "2026-01-01"
    }
}
```

### GET /pages/datawidgets/light_chart.php

**Description**: Get light/lux sensor data

**Response Format**:

```json
{
    "lux": [[timestamp, lux_value], ...],
    "solar": [[timestamp, solar_radiation], ...]
}
```

### GET /pages/datawidgets/air_chart.php

**Description**: Get air quality data

**Response Format**:

```json
{
    "pm25": [[timestamp, pm25_value], ...],
    "pm10": [[timestamp, pm10_value], ...]
}
```

### GET /pages/datawidgets/beecount_chart.php

**Description**: Get bee flight activity data

**Response Format**:

```json
{
    "bees_in": [[timestamp, count], ...],
    "bees_out": [[timestamp, count], ...],
    "net_activity": [[timestamp, in_minus_out], ...]
}
```

---

## System Status Endpoints

### GET /pages/status.php

**Description**: Get current system status

**Response Format** (HTML):

```html
<div class="status">
    <h2>System Status</h2>
    <ul>
        <li>Disk Usage: 45%</li>
        <li>Memory Usage: 512MB / 1024MB</li>
        <li>CPU Load: 0.5</li>
        <li>Uptime: 15 days</li>
    </ul>
</div>
```

### GET /admin/livevalue.php

**Description**: Get latest sensor reading (real-time)

**Response Format** (JSON):

```json
{
    "date": "2026-01-17 14:30:00",
    "hiveweight": 98.5,
    "hivetempf": 72.5,
    "hiveHum": 55.0,
    "outsidetemp": 65.2,
    "outsideHum": 60.0,
    "pressure": 29.92,
    "luxlevel": 15000,
    "IN_COUNT": 45,
    "OUT_COUNT": 42,
    "pm25": 8.5,
    "pm10": 12.3
}
```

**Example Request**:
```bash
curl -u admin:password http://your-hive-ip/admin/livevalue.php
```

### GET /admin/statusrefresh.php

**Description**: Get upgrade/command status (AJAX polling)

**Parameters**:

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `id` | integer | Yes | Message queue ID to check |

**Response Format** (HTML/Text):

```html
<div class="status-update">
    Status: In Progress<br>
    Progress: 45%<br>
    Message: Downloading updates...
</div>
```

---

## Data Export

### GET /pages/datawidgets/datajob.php

**Description**: Export data in CSV format

**Parameters**:

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `period` | string | Yes | Time period |
| `format` | string | Yes | Export format (csv) |

**Response Format** (CSV):

```csv
Date,Weight,Temperature,Humidity,Outside Temp,Pressure
2026-01-17 14:30:00,98.5,72.5,55.0,65.2,29.92
2026-01-17 14:35:00,98.6,72.6,55.1,65.3,29.91
...
```

**Example Request**:
```bash
curl -u admin:password "http://your-hive-ip/pages/datawidgets/datajob.php?period=week&format=csv" > export.csv
```

### Backup Download

**Endpoint**: `/admin/backup.php?action=download&file=backup_name.db`

**Method**: GET (requires authentication)

**Description**: Download database backup file

**Example**:
```bash
curl -u admin:password "http://your-hive-ip/admin/backup.php?action=download&file=backup_2026-01-17.db" -o backup.db
```

---

## Error Handling

### Error Response Format

**HTTP Status Codes**:

| Code | Meaning |
|------|---------|
| 200 | Success |
| 400 | Bad Request (invalid parameters) |
| 401 | Unauthorized (authentication failed) |
| 403 | Forbidden (CSRF token missing/invalid) |
| 404 | Not Found (endpoint doesn't exist) |
| 500 | Internal Server Error (database error, etc.) |

**Error Response (JSON)**:

```json
{
    "error": true,
    "message": "Invalid period parameter",
    "code": 400
}
```

**Error Response (Text)**:

```text
ERROR: Must specify type
```

### Common Error Messages

| Error | Cause | Solution |
|-------|-------|----------|
| "ERROR: Type can't be empty" | Missing `type` parameter | Add `?type=hiveweight` |
| "ERROR: Must specify period" | Missing `period` parameter | Add `&period=week` |
| "ERROR: Invalid period" | Invalid time period | Use: time, 2days, week, month, year, all |
| "CSRF token validation failed" | Missing CSRF token on POST | Include token from `csrf_field()` |
| "Database connection error" | Database unavailable | Check database file permissions |
| "401 Unauthorized" | Authentication failed | Check username/password |

---

## Rate Limiting

**Current Implementation**: No rate limiting

**Recommendations**:
- Self-hosted: No limits needed
- Public access: Implement rate limiting at web server level (Apache mod_ratelimit or nginx limit_req)

**Example Apache Rate Limiting**:

```apache
<Location /pages/datawidgets/>
    SetOutputFilter RATE_LIMIT
    SetEnv rate-limit 400
    SetEnv rate-initial-burst 10
</Location>
```

---

## JSONP Support

All chart data endpoints support JSONP for cross-domain requests.

**Usage**:

Add `callback` parameter to the query string:

```http
GET /pages/datawidgets/getdata.php?type=hiveweight&period=week&chart=all&callback=myCallback
```

**Response**:

```javascript
myCallback({
    "data": [[1673971200000, 98.5], [1673971500000, 98.6]],
    "config": {"hivename": "My Hive"}
});
```

**JavaScript Example**:

```javascript
function myCallback(data) {
    console.log('Received data:', data);
    // Process data
}

// Create script tag for JSONP
var script = document.createElement('script');
script.src = 'http://your-hive-ip/pages/datawidgets/getdata.php?type=hiveweight&period=week&chart=all&callback=myCallback';
document.head.appendChild(script);
```

---

## Examples

### Get Today's Weight Data

```bash
curl -u admin:password \
  "http://192.168.1.100/pages/datawidgets/getdata.php?type=hiveweight&period=time&chart=all" \
  | jq .
```

### Get Last Week's Temperature

```bash
curl -u admin:password \
  "http://192.168.1.100/pages/datawidgets/getdata.php?type=hivetempf&period=week&chart=all" \
  | jq .
```

### Get All Data for Last Month

```bash
curl -u admin:password \
  "http://192.168.1.100/pages/datawidgets/all_chart.php?period=month" \
  | jq .
```

### Get Latest Sensor Reading

```bash
curl -u admin:password \
  "http://192.168.1.100/admin/livevalue.php" \
  | jq .
```

### Export Last Year to CSV

```bash
curl -u admin:password \
  "http://192.168.1.100/pages/datawidgets/datajob.php?period=year&format=csv" \
  > hive_data_2025.csv
```

### JavaScript Fetch Example

```javascript
async function getHiveWeight() {
    const response = await fetch('http://192.168.1.100/pages/datawidgets/getdata.php?type=hiveweight&period=week&chart=all', {
        headers: {
            'Authorization': 'Basic ' + btoa('admin:password')
        }
    });

    if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
    }

    const data = await response.json();
    console.log('Weight data:', data);
    return data;
}

getHiveWeight()
    .then(data => {
        // Process data
        data.data.forEach(([timestamp, weight]) => {
            console.log(`${new Date(timestamp)}: ${weight} lbs`);
        });
    })
    .catch(error => console.error('Error:', error));
```

### Python Example

```python
import requests
import json
from datetime import datetime

# Authentication
auth = ('admin', 'password')

# Get weight data
url = 'http://192.168.1.100/pages/datawidgets/getdata.php'
params = {
    'type': 'hiveweight',
    'period': 'week',
    'chart': 'all'
}

response = requests.get(url, params=params, auth=auth)
data = response.json()

# Process data
for timestamp_ms, weight in data['data']:
    dt = datetime.fromtimestamp(timestamp_ms / 1000)
    print(f"{dt}: {weight} lbs")
```

### Node.js Example

```javascript
const axios = require('axios');

async function getHiveData() {
    const url = 'http://192.168.1.100/pages/datawidgets/all_chart.php';
    const auth = {
        username: 'admin',
        password: 'password'
    };

    try {
        const response = await axios.get(url, {
            params: { period: 'week' },
            auth: auth
        });

        console.log('Weight data:', response.data.weight);
        console.log('Temperature data:', response.data.temperature);
        console.log('Humidity data:', response.data.humidity);

        return response.data;
    } catch (error) {
        console.error('Error fetching data:', error.message);
        throw error;
    }
}

getHiveData();
```

### Chart Integration (HighCharts)

```javascript
// Fetch data and create HighCharts chart
async function createWeightChart() {
    const response = await fetch('http://192.168.1.100/pages/datawidgets/getdata.php?type=hiveweight&period=month&chart=all', {
        headers: {
            'Authorization': 'Basic ' + btoa('admin:password')
        }
    });

    const result = await response.json();

    Highcharts.chart('container', {
        chart: {
            type: 'line',
            zoomType: 'x'
        },
        title: {
            text: 'Hive Weight - Last Month'
        },
        xAxis: {
            type: 'datetime',
            title: {
                text: 'Date'
            }
        },
        yAxis: {
            title: {
                text: 'Weight (lbs)'
            }
        },
        series: [{
            name: 'Hive Weight',
            data: result.data
        }],
        tooltip: {
            xDateFormat: '%Y-%m-%d %H:%M',
            pointFormat: '<b>{point.y:.1f} lbs</b>'
        }
    });
}

createWeightChart();
```

---

## Data Smoothing

The API supports data smoothing for cleaner chart visualization.

**Configuration**: Set in database (`hiveconfig.chart_smoothing`)

**Behavior**:
- When enabled, zero values are returned as `null` instead of `0`
- Prevents chart lines from dropping to zero for missing/invalid readings
- Particularly useful for GDD and weight trends

**Example**:

```json
// Smoothing OFF:
[[1673971200000, 98.5], [1673971500000, 0], [1673971800000, 98.4]]

// Smoothing ON:
[[1673971200000, 98.5], [1673971500000, null], [1673971800000, 98.4]]
```

**HighCharts Handling**:
```javascript
series: [{
    data: dataWithNulls,
    connectNulls: false  // Don't connect across null values
}]
```

---

## Security Considerations

### Authentication

- All API endpoints require HTTP Basic Authentication
- Use HTTPS in production to protect credentials
- Change default password after installation

### CSRF Protection

- POST endpoints require valid CSRF token
- Token generated by `csrf_field()` function
- Token validated by `require_csrf_token()` function

### Input Validation

- All parameters validated and sanitized
- Whitelisted values for `type` and `period` parameters
- SQL injection prevented with parameterized queries

### Rate Limiting

- Consider implementing rate limiting for public access
- Monitor for unusual API usage patterns
- Use IP-based restrictions if needed

---

## Migration Notes

### v2.10 Changes

**New Security Features**:
- CSRF tokens required for POST requests
- All database queries use parameterized statements
- Output sanitized with `htmlspecialchars()`

**Breaking Changes**:
- None (backward compatible)

**Deprecated**:
- None

---

## See Also

- [Web Application Structure](WEB_APPLICATION.md)
- [Database Schema](DATABASE_SCHEMA.md)
- [Security Features](SECURITY.md)
- [Bash Scripts Reference](BASH_SCRIPTS.md)

---

## Support

For API-related issues:

1. Check authentication credentials
2. Verify parameter format
3. Review error messages
4. Check database connectivity
5. Review Apache error logs: `/var/log/apache2/error.log`

**GitHub Issues**: [https://github.com/rcrum003/HiveControl/issues](https://github.com/rcrum003/HiveControl/issues)
