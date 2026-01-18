# System Commands

## Admin Interface

Access via web interface: `/admin/system.php`

## Available Commands

### Clear Logs
Clears all entries from the logs table.

**Usage**: Admin → System Commands → Clear Logs

### Clear All Data
Removes all sensor data from allhivedata table.

**Warning**: This operation cannot be undone. Create a backup first.

**Usage**: Admin → System Commands → Clear All Data → Confirm

### Remove Zero Values
Removes zero/null values for a specific sensor column.

**Columns**:
- `hivetempf` - Hive temperature
- `hiveHum` - Hive humidity
- `hiveweight` - Hive weight
- `IN_COUNT` - Bee counter

**Usage**: Admin → System Commands → Remove Zeros → Select Column → Confirm

### Upgrade System
Triggers system upgrade from GitHub repository.

**Usage**: Admin → System Commands → Upgrade

**Manual**: `sudo /home/HiveControl/upgrade.sh`

### Pause Data Collection
Temporarily stops data collection without disabling cron.

**Usage**: Admin → System Commands → Pause/Resume

### View Logs
Displays last 1000 log entries.

**Usage**: Admin → System Commands → View Logs

### Server Status
Shows system information (disk space, memory, CPU).

**Usage**: Admin → System Commands → Server Status

## Command Line

```bash
# Manual data collection
sudo /home/HiveControl/scripts/system/currconditions.sh

# System upgrade
sudo /home/HiveControl/upgrade.sh

# View logs
sqlite3 /home/HiveControl/data/hive-data.db "SELECT * FROM logs ORDER BY date DESC LIMIT 100;"

# Check cron status
sudo systemctl status cron
```

For complete documentation, see:
- [Web Application Structure](WEB_APPLICATION.md)
- [Bash Scripts Reference](BASH_SCRIPTS.md)
