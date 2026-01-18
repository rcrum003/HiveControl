# HiveControl

![Version](https://img.shields.io/badge/version-2.10-blue.svg)
![Platform](https://img.shields.io/badge/platform-Raspberry%20Pi-red.svg)
![License](https://img.shields.io/badge/license-Open%20Source-green.svg)

**HiveControl** is a comprehensive beehive monitoring and management system designed to run on Raspberry Pi. It collects data from multiple sensors, provides a web-based dashboard for visualization, and integrates with Hivetool.org for data sharing.

## 🚀 Quick Start

### Basic Installation

Install HiveControl on your Raspberry Pi with a single command:

```bash
curl https://raw.githubusercontent.com/rcrum003/HiveControl/master/install.sh -o install.sh
chmod u+x install.sh
sudo ./install.sh
```

### Custom Installation Options

```bash
./install.sh -option

Options:
  -b    Beecounter Webcam Setup (Warning: Can take up to 8 hrs)
  -x    Install XRDP for Windows Remote Desktop users
  -k    Touch Screen Keyboard Install
  -h    Show help message
  -d    Turn on debug mode

# Install all options:
./install.sh -bxk
```

### Upgrading

```bash
sudo /home/HiveControl/upgrade.sh
```

**Note:** Upgrades preserve your data and configurations but will overwrite code changes in the scripts directory.

---

## 📋 Table of Contents

- [Features](#-features)
- [Supported Sensors](#-supported-sensors)
- [System Architecture](#-system-architecture)
- [Documentation](#-documentation)
- [Web Interface](#-web-interface)
- [Security](#-security)
- [Development](#-development)
- [Support](#-support)

---

## ✨ Features

### Data Collection & Monitoring
- **Real-time Sensor Data**: Weight, temperature, humidity, bee flight activity
- **Weather Integration**: WeatherFlow Tempest, Ambient Weather, OurWeather
- **Environmental Monitoring**: Light (LUX), air quality (PurpleAir)
- **Growing Degree Days (GDD)**: Track bee development and activity
- **Automated Data Collection**: Configurable collection intervals via cron

### Web Dashboard
- **Responsive Design**: Bootstrap-based interface for desktop and mobile
- **Interactive Charts**: HighCharts visualizations with multiple timeframes
- **Live Camera Feed**: Monitor hive activity with webcam integration
- **Historical Analysis**: View trends over days, weeks, months, or years

### Administration
- **Web-Based Configuration**: No command-line needed for most tasks
- **Backup & Restore**: Full database and configuration backups
- **Password Management**: Change admin password through web interface
- **System Monitoring**: View logs, disk space, and system status
- **Data Management**: Clear old data, remove zero values, pause collection

### Integration & Sharing
- **Hivetool.org Integration**: Share data with beekeeping research community
- **REST API**: JSON endpoints for external applications
- **Data Export**: Download backups and historical data

---

## 🔌 Supported Sensors

### Weight Sensors
- HX711 Load Cell Amplifier (Primary)
- Phidget 1046 Bridge Interface
- CPW-200+ Industrial Scale

### Temperature & Humidity
- DHT22 (AM2302) - External weather
- SHT31-D - Precision I2C sensor
- BME680 - Temperature/humidity/pressure/gas
- BME280 - Temperature/humidity/pressure
- BroodMinder Bluetooth sensors
- Temper USB sensors

### Weather Stations
- WeatherFlow Tempest (UDP broadcast)
- Ambient Weather (API)
- OurWeather stations
- Weather Underground (XML)

### Other Sensors
- TSL2591 / TSL2561 - Light/LUX sensors
- PurpleAir - Air quality monitoring
- Bee Counter - Optical flight activity counter

[Full Sensor Documentation](https://github.com/rcrum003/HiveControl/wiki/Sensor-Support)

---

## 🏗️ System Architecture

### Directory Structure

```
/home/HiveControl/
├── data/                           # Data storage
│   ├── hive-data.db               # SQLite database
│   └── backups/                   # Database backups
├── scripts/                        # Data collection scripts
│   ├── currconditions.sh          # Main data collection orchestrator
│   ├── weight/                    # Weight sensor scripts
│   ├── temp/                      # Temperature sensor scripts
│   ├── weather/                   # Weather station scripts
│   ├── lux/                       # Light sensor scripts
│   ├── air/                       # Air quality scripts
│   └── beecounter/                # Bee flight counter
├── www/public_html/               # Web application
│   ├── admin/                     # Admin interface
│   ├── pages/                     # User-facing pages
│   ├── include/                   # Shared PHP files
│   ├── charts/                    # Chart data endpoints
│   └── bower_components/          # Frontend libraries
├── install.sh                      # Installation script
└── upgrade.sh                      # Update script
```

### Data Flow

```
┌─────────────────────────────────────────────────┐
│  Physical Sensors (I2C, GPIO, USB, Network)     │
└────────────────┬────────────────────────────────┘
                 │
                 v
┌─────────────────────────────────────────────────┐
│  Bash Collection Scripts (scripts/)              │
│  - currconditions.sh (orchestrator)             │
│  - Individual sensor scripts                     │
└────────────────┬────────────────────────────────┘
                 │
                 v
┌─────────────────────────────────────────────────┐
│  SQLite Database (hive-data.db)                 │
│  - allhivedata (sensor readings)                │
│  - hiveconfig (configuration)                   │
│  - logs (audit trail)                           │
└────────────────┬────────────────────────────────┘
                 │
                 v
┌─────────────────────────────────────────────────┐
│  PHP Web Application (www/)                     │
│  - Real-time dashboard                          │
│  - Configuration interface                      │
│  - Chart data APIs                              │
└────────────────┬────────────────────────────────┘
                 │
                 v
┌─────────────────────────────────────────────────┐
│  External Integration                           │
│  - Hivetool.org data sharing                    │
│  - REST API access                              │
└─────────────────────────────────────────────────┘
```

### Technology Stack

**Backend:**
- **Language**: PHP 7.0+, Bash
- **Database**: SQLite 3
- **Web Server**: Apache 2.4
- **Platform**: Raspberry Pi OS (Raspbian Buster/Bullseye)

**Frontend:**
- **Framework**: Bootstrap 3
- **Charts**: HighCharts
- **jQuery**: 3.x
- **Data Tables**: DataTables plugin

**Python Dependencies:**
- pigpio (GPIO control)
- Adafruit libraries (I2C sensors)
- RPi.GPIO

---

## 📚 Documentation

### User Guides
- [Installation Guide](documentation/INSTALLATION.md)
- [Sensor Setup](https://github.com/rcrum003/HiveControl/wiki/Sensor-Support)
- [Backup & Restore](documentation/BACKUP_FEATURE.md)
- [Web Interface Guide](documentation/WEB_INTERFACE.md)

### Administration
- [Configuration Reference](documentation/CONFIGURATION.md)
- [System Commands](documentation/SYSTEM_COMMANDS.md)
- [Troubleshooting](documentation/TROUBLESHOOTING.md)

### Developer Documentation
- [Architecture Overview](documentation/ARCHITECTURE.md)
- [Web Application Structure](documentation/WEB_APPLICATION.md)
- [Bash Scripts Reference](documentation/BASH_SCRIPTS.md)
- [Database Schema](documentation/DATABASE_SCHEMA.md)
- [API Reference](documentation/API.md)
- [Security Features](documentation/SECURITY.md)

### Release Information
- [Version 2.10 Release Notes](RELEASE)
- [Security Audit Report](SECURITY_AUDIT_2026-01-17.md)
- [Security Fixes Applied](SECURITY_FIXES_APPLIED_2026-01-17.md)

---

## 🖥️ Web Interface

### Dashboard Features

**Main Dashboard** (`/pages/index.php`)
- Combined chart with weight, temperature, humidity, and bee activity
- Time range selector (Today, 2 days, 1 week, 1 month, 1 year, All)
- Current conditions display
- Webcam feed integration

**Specialized Views**
- **Weight Chart** - Track hive weight changes, nectar flow
- **Temperature Chart** - Internal and external temperature
- **Environment Chart** - Humidity, barometric pressure, weather
- **GDD Chart** - Growing degree days with pollen counts
- **Light Chart** - Solar radiation and daylight patterns
- **Air Quality** - PM2.5, PM10 pollution levels
- **Bee Counter** - Flight activity tracking

**Administration Pages** (`/admin/`)
- **Hive Configuration** - Basic settings, location, Hivetool.org integration
- **Instrument Configuration** - Sensor selection and calibration
- **Site Configuration** - Weather station and sensor-specific settings
- **System Commands** - Data management, logs, upgrades
- **Backup & Restore** - Database backup and recovery
- **Change Password** - Admin password management
- **Notifications** - Email alerts configuration

### Authentication

- **Method**: Apache Basic Authentication with .htpasswd
- **Default Username**: `admin`
- **Password Location**: `/home/HiveControl/www/.htpasswd`
- **Change Password**: Use web interface at `/admin/changepassword.php`

---

## 🔒 Security

### Version 2.10 Security Enhancements

HiveControl 2.10 includes comprehensive security improvements:

**✅ SQL Injection Protection**
- Parameterized queries throughout application
- Input validation and whitelisting
- Prepared statements with PDO

**✅ Cross-Site Scripting (XSS) Prevention**
- All output properly escaped with `htmlspecialchars()`
- Content Security Policy headers
- Input sanitization

**✅ CSRF Protection**
- Token-based validation on all POST forms
- Session-based token generation
- Automatic token regeneration

**✅ Session Security**
- HTTPOnly cookies prevent JavaScript access
- SameSite=Strict prevents cross-site attacks
- Session ID regeneration every 5 minutes
- 1-hour session timeout

**✅ Additional Protections**
- IP spoofing prevention
- Information disclosure prevention
- Security headers (X-Frame-Options, CSP, etc.)
- Password protection (no plaintext exposure)

[Complete Security Documentation](SECURITY_FIXES_APPLIED_2026-01-17.md)

---

## 👨‍💻 Development

### Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Test thoroughly on Raspberry Pi
4. Submit a pull request

### Development Setup

```bash
# Clone repository
git clone https://github.com/rcrum003/HiveControl.git
cd HiveControl

# Create test branch
git checkout -b feature/your-feature

# Test on Raspberry Pi
sudo ./install.sh -d  # Debug mode
```

### Coding Standards

**PHP:**
- Use prepared statements for all database queries
- Escape all output with `htmlspecialchars()`
- Include CSRF tokens in all POST forms
- Follow existing code structure and naming conventions

**Bash:**
- Use `set -eo pipefail` for error handling
- Quote all variable expansions
- Provide error checking for all operations
- Log errors to syslog or logs table

### Testing

- Test all sensor integrations on actual hardware
- Verify database operations don't corrupt data
- Check web interface on desktop and mobile
- Validate security features (CSRF, XSS, SQL injection)

---

## 🐛 Troubleshooting

### Common Issues

**Sensors Not Reading**
- Check I2C is enabled: `sudo raspi-config`
- Verify sensor connections and power
- Test sensor directly: `i2cdetect -y 1`
- Review logs: `/admin/system.php` → View Logs

**Web Interface Not Loading**
- Check Apache status: `sudo systemctl status apache2`
- Verify PHP installed: `php -v`
- Check file permissions: `sudo chown -R www-data:www-data /home/HiveControl/www`
- Review Apache error log: `/var/log/apache2/error.log`

**Database Errors**
- Check database file exists: `ls -la /home/HiveControl/data/hive-data.db`
- Verify permissions: `sudo chown www-data:www-data /home/HiveControl/data/hive-data.db`
- Test SQLite: `sqlite3 /home/HiveControl/data/hive-data.db "SELECT * FROM hiveconfig;"`

**Data Not Collecting**
- Check cron is running: `sudo systemctl status cron`
- Verify currconditions.sh executable: `chmod +x /home/HiveControl/scripts/currconditions.sh`
- Run manually to see errors: `sudo /home/HiveControl/scripts/currconditions.sh`
- Check for "pause" status in system settings

[Complete Troubleshooting Guide](documentation/TROUBLESHOOTING.md)

---

## 🆘 Support

### Resources

- **Website**: [https://hivecontrol.org](https://hivecontrol.org)
- **GitHub Issues**: [https://github.com/rcrum003/HiveControl/issues](https://github.com/rcrum003/HiveControl/issues)
- **Wiki**: [https://github.com/rcrum003/HiveControl/wiki](https://github.com/rcrum003/HiveControl/wiki)
- **Hivetool.org**: [https://hivetool.org](https://hivetool.org)

### Getting Help

1. Check the [Troubleshooting Guide](documentation/TROUBLESHOOTING.md)
2. Search [existing issues](https://github.com/rcrum003/HiveControl/issues)
3. Review system logs in the web interface
4. Post detailed issue report with:
   - HiveControl version
   - Raspberry Pi model and OS version
   - Sensors being used
   - Error messages from logs
   - Steps to reproduce

---

## 📊 Database Schema

### Core Tables

**allhivedata** - Sensor readings
- `id`, `date`, `hivetempf`, `hiveHum`, `hiveweight`, `luxlevel`
- `outsidetemp`, `outsideHum`, `pressure`, `rain`, `wind`
- `solar`, `uvindex`, `IN_COUNT`, `OUT_COUNT`
- `pm25`, `pm10`, and more

**hiveconfig** - System configuration
- Hive name, location, timezone
- Hivetool.org credentials
- GDD settings, sensor enable/disable flags

**logs** - Audit trail
- All system operations logged with timestamp
- IP address tracking for security
- Error and success messages

**msgqueue** - System command queue
- Upgrade requests
- Data export jobs
- Background tasks

[Complete Database Documentation](documentation/DATABASE_SCHEMA.md)

---

## 📜 License

HiveControl is open source software. See individual files for specific license information.

---

## 🙏 Credits

**Author**: Ryan Crum
**Website**: [hivecontrol.org](https://hivecontrol.org)
**Related Project**: [Hivetool.org](https://hivetool.org)

Special thanks to the beekeeping and open source communities for their contributions, testing, and feedback.

---

## 🐝 Platform Support

**Supported Raspberry Pi Models:**
- Raspberry Pi 4 Model B
- Raspberry Pi 3 Model B/B+
- Raspberry Pi 2 Model B
- Raspberry Pi Zero/Zero W (limited)

**Supported Operating Systems:**
- Raspberry Pi OS (Raspbian) Buster
- Raspberry Pi OS (Raspbian) Bullseye
- Debian-based distributions

**Minimum Requirements:**
- 1GB RAM (2GB+ recommended)
- 8GB SD card (16GB+ recommended)
- Active internet connection for weather and Hivetool.org integration

---

**Latest Release**: Version 2.10 (2026-01-17)
**Status**: Production Ready ✅
