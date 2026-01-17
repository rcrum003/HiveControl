# HiveControl Backup & Restore Feature

## Overview
Version 1.0 - Created 2026-01-17

The Backup & Restore feature provides a web-based interface for creating, managing, and restoring backups of your HiveControl system directly from the WebGUI.

## Features

### Backup Types

1. **Full Backup** (Recommended)
   - Complete SQLite database file copy using optimized file copy method
   - Includes all historical measurements
   - Includes all configuration settings
   - Includes logs, notifications, and queue data
   - Supports large databases (60MB+) without timeout issues
   - Typical size: 26-60MB+ (varies with data collection duration)
   - Uses WAL checkpoint before copy to ensure database consistency

2. **Configuration Only**
   - Exports only the `hiveconfig` table as SQL
   - Preserves all settings without historical data
   - Much smaller file size (~50KB)
   - Useful for cloning hive configurations

### Safety Features

- **Automatic Safety Backup**: Before every restore operation, the system automatically creates a safety backup
- **Data Collection Pause**: Data collection is automatically paused during restore to prevent corruption
- **Auto-Resume**: Data collection automatically resumes after successful restore
- **Rollback Protection**: If restore fails, safety backup can be used for recovery
- **Audit Logging**: All backup/restore operations logged with IP addresses

### User Interface

#### Three Main Tabs:

1. **Create Backup**
   - Select backup type (Full or Config Only)
   - View estimated backup size
   - One-click backup creation
   - Success/failure notifications

2. **Restore**
   - List of available backups
   - Backup metadata (type, date, size)
   - Confirmation dialog before restore
   - Automatic safety backup creation

3. **Manage Backups**
   - View all existing backups
   - Download backups to local computer
   - Delete old/unwanted backups
   - Sort by date (newest first)

## File Locations

### Backup Directory
```
/home/HiveControl/data/backups/
```

### Backup Filename Format
```
hivecontrol_[type]_YYYY-MM-DD_HHmmss.db
hivecontrol_[type]_YYYY-MM-DD_HHmmss.sql
```

Examples:
- `hivecontrol_full_2026-01-17_143022.db`
- `hivecontrol_config_2026-01-17_120500.sql`
- `hivecontrol_prerestore_2026-01-17_150133.db` (safety backup)

### Security
- **`.htaccess` Protection**: Backup directory protected from web access
- **File Permissions**: 0755 on directory, standard file permissions
- **No Public Access**: Backups only accessible through authenticated admin interface

## Technical Implementation

### Performance Optimizations

**Execution Time & Memory Limits:**
```php
set_time_limit(300); // 5 minutes for large databases
ini_set('memory_limit', '256M'); // Sufficient for file operations
```

These limits ensure that large database backups (60MB+) can complete without PHP timeout errors, which are common with the default 30-second execution limit.

### Database Operations

**Full Backup Method (Optimized for Large Databases):**

The system uses a simple file copy approach instead of SQLite's backup API for better performance and reliability:

```php
// Ensure WAL (Write-Ahead Logging) is synced to main database file
$conn->exec("PRAGMA wal_checkpoint(TRUNCATE)");

// Direct file copy - much faster than SQLite backup() for large databases
if (!copy($db_path, $backup_path)) {
    throw new Exception("Failed to copy database file");
}
```

**Why File Copy Instead of SQLite Backup API:**
- **Speed**: Direct file copy is significantly faster than SQLite's backup() method
- **Memory Efficient**: Doesn't load entire database into memory
- **Reliability**: Avoids timeout issues with 60MB+ databases
- **Simplicity**: Standard PHP copy() function is well-tested and reliable
- **WAL Safety**: PRAGMA wal_checkpoint(TRUNCATE) ensures all changes are in main file before copy

**Configuration Backup:**
```php
// Exports hiveconfig table as SQL INSERT statement
SELECT * FROM hiveconfig
// Generated SQL includes DELETE and INSERT
```

**Restore Operations:**
```php
// Full restore: Direct file copy
copy($backup_path, $db_path);

// Config restore: Execute SQL statements
$conn->exec($sql);
```

### Restore Workflow

1. User selects backup and clicks "Restore"
2. Confirmation modal displayed
3. User confirms restore
4. System creates safety backup automatically
5. Data collection paused (RUN='no')
6. Wait 2 seconds for running scripts to complete
7. Restore database from backup
8. Data collection resumed (RUN='yes')
9. Success/failure notification shown
10. Operation logged to database

### Error Handling

- Try/catch blocks around all database operations
- Automatic rollback to safety backup on failure
- Clear error messages displayed to user
- All errors logged to database with details

## Usage Guide

### Creating a Backup

1. Navigate to **Settings → Backup & Restore**
2. On the "Create Backup" tab:
   - Select backup type (Full recommended for complete safety)
   - Click "Create Backup" button
3. Backup created and listed in Manage Backups tab
4. Optionally download backup to your computer

### Restoring from Backup

1. Navigate to **Settings → Backup & Restore**
2. Click the "Restore" tab
3. Find the backup you want to restore
4. Click "Restore" button
5. **Confirm the operation** in the modal dialog
6. Wait for restore to complete
7. System automatically creates safety backup
8. Data collection pauses, then resumes after restore

### Managing Backups

1. Navigate to **Settings → Backup & Restore**
2. Click the "Manage Backups" tab
3. Available actions:
   - **Download**: Save backup to your computer
   - **Delete**: Remove backup from server
4. All backups sorted by date (newest first)

## Best Practices

### When to Create Backups

- **Before System Upgrades**: Create full backup before running upgrades
- **Before Major Configuration Changes**: Backup before changing hardware or sensors
- **Regular Schedule**: Weekly or monthly backups for long-term safety
- **Before Experiments**: Backup before testing new features or settings
- **Hardware Migration**: Full backup before moving to new Raspberry Pi

### Backup Retention

- Keep at least 3-5 recent full backups
- Keep configuration backups for major milestones
- Download important backups to separate storage
- Delete very old backups to save disk space
- Safety backups (_prerestore_) can be deleted after successful restores

### Recovery Scenarios

**Lost Configuration:**
- Restore from most recent configuration backup
- Historical data preserved

**Corrupted Database:**
- Restore from most recent full backup
- May lose data collected since backup

**Hardware Failure:**
- Download backup from old system
- Install HiveControl on new hardware
- Upload and restore backup
- Resume data collection

**Accidental Data Deletion:**
- Restore from backup before deletion occurred
- System logs show when deletion happened

## Logging

All operations logged to `logs` table with:
- **Date/Time**: When operation occurred
- **Program**: "BACKUP" or "RESTORE"
- **Type**: "SUCCESS", "ERROR", or "INFO"
- **Message**: Details including filename and user IP
- **User IP**: Source IP address of admin

Example log entries:
```
2026-01-17 14:30:22 | BACKUP | SUCCESS | Full backup created: hivecontrol_full_2026-01-17_143022.db (25.3 MB) by IP 192.168.1.100
2026-01-17 15:15:33 | RESTORE | INFO | Safety backup created: hivecontrol_prerestore_2026-01-17_151533.db before restore of hivecontrol_full_2026-01-17_143022.db by IP 192.168.1.100
2026-01-17 15:15:35 | RESTORE | SUCCESS | Restore completed from hivecontrol_full_2026-01-17_143022.db by IP 192.168.1.100
```

## Database Tables Backed Up

### Full Backup Includes:
- `hiveconfig` - All configuration settings
- `allhivedata` - All measurement data
- `hivedata` - Legacy measurement data
- `weather` - Weather station data
- `gdd` - Growing Degree Days
- `pollen` - Pollen data
- `logs` - System logs
- `msgqueue` - Background task queue
- `notifications` - Alert configurations
- `hiveequipmentweight` - Equipment weights
- Database schema and indexes

### Configuration Backup Includes:
- `hiveconfig` table only

## Troubleshooting

### Backup Creation Fails

**Blank Screen or Timeout:**
- This was a known issue with databases larger than 60MB using the SQLite backup API
- Now resolved by using direct file copy method
- If still experiencing issues, check PHP execution time limit in `/etc/php/php.ini`
- Script sets `max_execution_time = 300` but server config may override it

**General Failures:**
- Check disk space: `df -h`
- Verify backup directory exists and is writable: `ls -la /home/HiveControl/data/backups`
- Check PHP error logs: `/var/log/apache2/error.log` or `/var/log/php-fpm/error.log`
- Ensure database is not locked by other processes
- Verify database file is readable: `ls -la /home/HiveControl/data/hive-data.db`

### Restore Fails
- Check that backup file exists and is valid SQLite database
- Ensure sufficient disk space for restoration
- Check file permissions on database file
- Review logs table for error details
- Safety backup should be available for rollback

### Can't Access Backups
- Verify backup directory path is correct
- Check file permissions on `/data/backups/`
- Ensure `.htaccess` is not blocking admin access
- Verify user has admin privileges

### Backup Directory Not Created
- Manually create: `mkdir -p /home/HiveControl/data/backups`
- Set permissions: `chmod 755 /home/HiveControl/data/backups`
- Create protection: `echo "Deny from all" > /home/HiveControl/data/backups/.htaccess`

## Future Enhancements

Potential features for future versions:
- Scheduled automatic backups (cron integration)
- Cloud backup upload (to hivecontrol.org)
- Backup encryption
- Email notifications on backup success/failure
- Backup verification/integrity checking
- Selective table restore
- Backup compression
- Remote backup download via API
- Backup age warnings
- Automatic cleanup of old backups

## Version History

**Version 1.0** (2026-01-17)
- Initial release
- Full and configuration backup types
- Web-based restore functionality
- Safety backup system
- Audit logging
- Download/delete management
- Optimized for large databases (60MB+) using file copy method
- Native browser confirm dialogs (simpler and more reliable than Bootstrap modals)
- Tab state preservation after operations
- Increased PHP execution time and memory limits for large database support
- WAL checkpoint before backup for data consistency

## Support

For issues or questions:
- Check system logs in **Settings → System Commands → View Logs**
- Review audit trail in logs table
- Visit: https://hivecontrol.org
- GitHub: https://github.com/rcrum003/hivecontrol

## Credits

Developed as part of the HiveControl 2.10 release
Author: Ryan Crum
Website: hivecontrol.org
