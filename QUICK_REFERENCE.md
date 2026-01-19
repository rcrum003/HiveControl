# HiveControl Quick Reference Card

## Installation (One Command!)

```bash
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash
```

---

## Access Your System

**Web Interface**: `http://YOUR-PI-IP/`

**Admin Panel**: `http://YOUR-PI-IP/admin/`
- Username: `admin`
- Password: (set during installation)

**Find IP Address**:
```bash
hostname -I
```

---

## Common Commands

### System Control
```bash
sudo reboot              # Restart Raspberry Pi
sudo shutdown -h now     # Shut down safely
```

### HiveControl Management
```bash
sudo /home/HiveControl/upgrade.sh              # Update HiveControl
cat /home/HiveControl/VERSION                   # Check version
sudo /home/HiveControl/scripts/currconditions.sh # Test sensors
```

### System Maintenance
```bash
sudo apt update && sudo apt upgrade -y    # Update OS
sudo systemctl status apache2             # Check web server
sudo systemctl restart apache2            # Restart web server
```

### View Logs
```bash
sudo tail -50 /var/log/apache2/error.log     # Web server errors
sudo journalctl -xe                           # System logs
```

---

## Web Interface Quick Links

| Page | URL | Purpose |
|------|-----|---------|
| Dashboard | `/pages/index.php` | View data and charts |
| Weight Chart | `/pages/weightchart.php` | Hive weight over time |
| Temperature | `/pages/tempchart.php` | Temperature trends |
| Settings | `/admin/hiveconfig.php` | Basic configuration |
| Instruments | `/admin/instrumentconfig.php` | Sensor setup |
| System | `/admin/system.php` | Logs and maintenance |
| Backup | `/admin/backup.php` | Database backup/restore |
| Password | `/admin/changepassword.php` | Change admin password |

---

## Sensor Testing

### Test All Sensors
Go to: Admin → Settings → Instruments → Click "Read Sensors"

### Test Individual Sensors (Command Line)
```bash
# Temperature (DHT22)
sudo /home/HiveControl/scripts/temp/dhtget.sh

# Weight (HX711)
sudo /home/HiveControl/scripts/weight/gethx711.sh

# Light (TSL2591)
sudo /home/HiveControl/scripts/lux/gettsl2591.sh
```

---

## Troubleshooting Quick Fixes

### Web Interface Not Loading
```bash
sudo systemctl status apache2
sudo systemctl restart apache2
sudo chown -R www-data:www-data /home/HiveControl/www
```

### Database Errors
```bash
sudo chown www-data:www-data /home/HiveControl/data/hive-data.db
sudo chown www-data:www-data /home/HiveControl/data
```

### No Data Collecting
```bash
sudo crontab -l                                      # Check cron jobs
sudo /home/HiveControl/scripts/currconditions.sh    # Test manually
```

### Forgot Admin Password
```bash
cd /home/HiveControl/www/
sudo htpasswd -c .htpasswd admin
```

---

## File Locations

| Item | Location |
|------|----------|
| Database | `/home/HiveControl/data/hive-data.db` |
| Scripts | `/home/HiveControl/scripts/` |
| Web Files | `/home/HiveControl/www/public_html/` |
| Admin Password | `/home/HiveControl/www/.htpasswd` |
| Version | `/home/HiveControl/VERSION` |
| Backups | `/home/HiveControl/data/backups/` |
| Apache Config | `/etc/apache2/sites-available/000-default.conf` |
| Cron Jobs | `sudo crontab -l` |

---

## Backup & Restore

### Create Backup (Web Interface)
1. Go to: `http://YOUR-PI-IP/admin/backup.php`
2. Click "Create Backup"
3. Download the backup file

### Create Backup (Command Line)
```bash
cd /home/HiveControl/data
sudo sqlite3 hive-data.db ".backup /home/HiveControl/data/backups/backup-$(date +%Y%m%d).db"
```

### Restore Backup
1. Upload backup to: `/home/HiveControl/data/backups/`
2. Go to: `http://YOUR-PI-IP/admin/backup.php`
3. Select backup and click "Restore"

---

## Raspberry Pi OS Commands

### Network
```bash
ifconfig                    # Show network info
ping google.com             # Test internet
hostname -I                 # Show IP address
sudo raspi-config           # System configuration
```

### System Info
```bash
cat /etc/os-release         # OS version
uname -a                    # Kernel info
vcgencmd measure_temp       # CPU temperature
df -h                       # Disk space
free -h                     # Memory usage
```

### I2C (for sensors)
```bash
sudo raspi-config           # Enable I2C: Interface Options → I2C → Yes
i2cdetect -y 1              # Scan for I2C devices
```

---

## Sensor Pin Connections

### HX711 (Weight Sensor)
- VCC → 5V
- GND → Ground
- DT → GPIO 5 (Pin 29)
- SCK → GPIO 6 (Pin 31)

### DHT22 (Temperature/Humidity)
- VCC → 3.3V or 5V
- GND → Ground
- DATA → GPIO 4 (Pin 7)

### BME280/BME680 (I2C)
- VCC → 3.3V
- GND → Ground
- SDA → GPIO 2 (Pin 3)
- SCL → GPIO 3 (Pin 5)

### TSL2591 (Light Sensor - I2C)
- VCC → 3.3V
- GND → Ground
- SDA → GPIO 2 (Pin 3)
- SCL → GPIO 3 (Pin 5)

---

## Support Resources

| Resource | URL |
|----------|-----|
| Documentation | [README.md](README.md) |
| Installation Guide | [EASY_INSTALL_GUIDE.md](EASY_INSTALL_GUIDE.md) |
| Download Options | [DOWNLOAD_INSTRUCTIONS.md](DOWNLOAD_INSTRUCTIONS.md) |
| Community | [hivecontrol.org](https://hivecontrol.org) |
| Report Issues | [github.com/rcrum003/HiveControl/issues](https://github.com/rcrum003/HiveControl/issues) |
| HiveTool.org | [hivetool.org](https://hivetool.org) |

---

## Version Information

**Current Version**: 2.10

**Check Your Version**:
```bash
cat /home/HiveControl/VERSION
```

**Update to Latest**:
```bash
sudo /home/HiveControl/upgrade.sh
```

---

## Emergency Recovery

### Complete Reset (Reinstall)
```bash
sudo rm -rf /home/HiveControl
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash
```

### Restore from Backup After Reinstall
```bash
sudo cp /path/to/backup.db /home/HiveControl/data/hive-data.db
sudo chown www-data:www-data /home/HiveControl/data/hive-data.db
```

---

**Print this page and keep it handy for quick reference!**

*Last Updated: 2026-01-19 | Version 2.10*
