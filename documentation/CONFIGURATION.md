# Configuration Reference

## Configuration Files

- **Database**: `/home/HiveControl/data/hive-data.db`
  See [Database Schema](DATABASE_SCHEMA.md)

- **Web Configuration**: Via web interface at `/admin/hiveconfig.php`

- **Security Settings**: `/home/HiveControl/www/public_html/include/security-init.php`
  See [Security Features](SECURITY.md)

## Web Interface Configuration

**Hive Configuration** (`/admin/hiveconfig.php`):
- Hive name, location, timezone
- Hivetool.org integration
- GDD (Growing Degree Days) settings

**Instrument Configuration** (`/admin/instrumentconfig.php`):
- Sensor selection and calibration
- Weight sensor slope/zero values
- GPIO pin assignments

**Site Configuration** (`/admin/siteconfig.php`):
- Weather station setup
- API keys (Ambient Weather, PurpleAir)
- Local sensor configuration

For complete details, see:
- [Web Application Structure](WEB_APPLICATION.md)
- [Database Schema](DATABASE_SCHEMA.md)
