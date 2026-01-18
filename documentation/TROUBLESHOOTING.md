# Troubleshooting Guide

## Common Issues

### Sensors Not Reading

**Check I2C is enabled**:
```bash
sudo raspi-config
# Navigate to: Interface Options → I2C → Enable
```

**Test sensor connections**:
```bash
i2cdetect -y 1
```

**Review logs**:
- Web: Admin → System Commands → View Logs
- CLI: `sudo tail -f /var/log/syslog | grep -i hive`

**Run collection manually**:
```bash
sudo /home/HiveControl/scripts/system/currconditions.sh
```

### Web Interface Not Loading

**Check Apache**:
```bash
sudo systemctl status apache2
sudo systemctl restart apache2
```

**Check PHP**:
```bash
php -v
```

**Check file permissions**:
```bash
sudo chown -R www-data:www-data /home/HiveControl/www
```

**Review error logs**:
```bash
sudo tail -f /var/log/apache2/error.log
```

### Database Errors

**Check database exists**:
```bash
ls -la /home/HiveControl/data/hive-data.db
```

**Fix permissions**:
```bash
sudo chown www-data:www-data /home/HiveControl/data/hive-data.db
sudo chmod 664 /home/HiveControl/data/hive-data.db
```

**Test database**:
```bash
sqlite3 /home/HiveControl/data/hive-data.db "SELECT * FROM hiveconfig;"
```

**Check for corruption**:
```bash
sqlite3 /home/HiveControl/data/hive-data.db "PRAGMA integrity_check;"
```

### Data Not Collecting

**Check cron is running**:
```bash
sudo systemctl status cron
```

**Verify crontab entry**:
```bash
crontab -l | grep currconditions
```

**Check script permissions**:
```bash
chmod +x /home/HiveControl/scripts/system/currconditions.sh
```

**Check for pause status**:
- Web: Admin → System Commands → Resume

### Blank Pages

**Check PHP errors**:
```bash
sudo tail -f /var/log/apache2/error.log
```

**Common causes**:
- Missing `include` files
- PHP syntax errors
- Malformed HTML structure
- Missing closing tags

**Fix**:
- Check all `include()` paths are correct
- Validate HTML structure
- Review PHP error log

### Authentication Issues

**Reset password**:
```bash
# Using web interface (recommended)
# Navigate to /admin/changepassword.php

# Using command line
sudo htpasswd /home/HiveControl/www/.htpasswd admin
```

**Check .htpasswd exists**:
```bash
ls -la /home/HiveControl/www/.htpasswd
```

### CSRF Token Errors

**Clear browser cookies**:
- Chrome: Settings → Privacy → Clear browsing data
- Firefox: Preferences → Privacy → Clear Data

**Check session directory**:
```bash
ls -la /var/lib/php/sessions
sudo chmod 1733 /var/lib/php/sessions
```

**Verify security-init.php is included**:
Check that pages include: `include($_SERVER["DOCUMENT_ROOT"] . "/include/security-init.php");`

## Performance Issues

### Slow Page Loading

**Check database size**:
```bash
du -h /home/HiveControl/data/hive-data.db
```

**Optimize database**:
```bash
sqlite3 /home/HiveControl/data/hive-data.db "VACUUM;"
```

**Clear old data**:
- Web: Admin → System Commands → Clear All Data

### High CPU Usage

**Check running processes**:
```bash
top
# Look for currconditions.sh or sensor scripts
```

**Check cron frequency**:
```bash
crontab -l
# Default is */5 (every 5 minutes)
```

## Getting Help

1. Check this guide
2. Review system logs (web interface or CLI)
3. Search [GitHub Issues](https://github.com/rcrum003/HiveControl/issues)
4. Post new issue with:
   - HiveControl version
   - Raspberry Pi model
   - OS version
   - Sensors being used
   - Error messages
   - Steps to reproduce

For detailed documentation, see:
- [Web Application Structure](WEB_APPLICATION.md)
- [Bash Scripts Reference](BASH_SCRIPTS.md)
- [Database Schema](DATABASE_SCHEMA.md)
- [Security Features](SECURITY.md)
