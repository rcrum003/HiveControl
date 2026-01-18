# Security Features

**HiveControl Security Documentation**
Version 2.10 - 2026-01-17

---

## Overview

HiveControl version 2.10 includes comprehensive security improvements addressing all CRITICAL and HIGH severity vulnerabilities identified in the security audit. This document provides an overview of the security features and best practices.

For complete security audit and fixes documentation, see:
- [Security Audit Report](../SECURITY_AUDIT_2026-01-17.md)
- [Security Fixes Applied](../SECURITY_FIXES_APPLIED_2026-01-17.md)

---

## Security Features

### SQL Injection Protection

**Implementation**: Parameterized queries throughout the application

**Before (Vulnerable)**:
```php
$query = "INSERT INTO logs VALUES (\"$date\",\"$program\",\"$type\",\"$message\")";
$conn->query($query);
```

**After (Secure)**:
```php
$stmt = $conn->prepare("INSERT INTO logs (date,program,type,message) VALUES (?,?,?,?)");
$stmt->execute([$date, $program, $type, $message]);
```

**Files Fixed**:
- `/admin/system.php` - loglocal() function, removezero command, message queue
- `/admin/changepassword.php` - loglocal() function
- `/admin/backup.php` - loglocal() function
- All database operations use PDO prepared statements

**Column Whitelisting** (for dynamic queries):
```php
$allowed_columns = ['hivetempf', 'hiveHum', 'hiveweight', 'IN_COUNT'];
if (!in_array($column, $allowed_columns)) {
    die("Invalid column name");
}
```

---

### Cross-Site Scripting (XSS) Prevention

**Implementation**: All output escaped with `htmlspecialchars()`

**Before (Vulnerable)**:
```php
echo '<td>' . $row['message'] . '</td>';
```

**After (Secure)**:
```php
echo '<td>' . htmlspecialchars($row['message']) . '</td>';
```

**Files Fixed**:
- `/admin/system.php` - getlog() function, error messages
- `/admin/hiveconfig.php` - configuration display
- `/admin/notifications.php` - notification output

**Content Security Policy**:
```php
header("Content-Security-Policy: default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'; img-src 'self' data:;");
```

---

### CSRF Protection

**Implementation**: Token-based validation on all POST forms

**security-init.php Functions**:
```php
// Generate token
get_csrf_token()           // Returns current token

// Form field
csrf_field()               // Returns <input type="hidden" name="csrf_token" value="...">

// Validation
verify_csrf_token()        // Returns true/false
require_csrf_token()       // Dies if token invalid
```

**Usage in Forms**:
```php
<form method="POST" action="?">
    <?php echo csrf_field(); ?>
    <!-- form fields -->
</form>
```

**Usage in Handlers**:
```php
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    require_csrf_token();  // Dies if token invalid
    // Process form
}
```

**Files Protected**:
- `/admin/backup.php` - Backup operations
- `/admin/changepassword.php` - Password changes
- All POST forms in admin interface

---

### Session Security

**Configuration** (`/include/security-init.php`):
```php
ini_set('session.cookie_httponly', 1);      // Prevent JavaScript access
ini_set('session.cookie_samesite', 'Strict'); // CSRF protection
ini_set('session.use_strict_mode', 1);       // Reject bad session IDs
ini_set('session.gc_maxlifetime', 3600);     // 1 hour timeout
```

**Session Regeneration**:
```php
// Every 5 minutes
if (time() - $_SESSION['last_regeneration'] > 300) {
    session_regenerate_id(true);
    $_SESSION['last_regeneration'] = time();
}
```

**HTTPS Requirement** (optional):
```php
// Uncomment if using HTTPS:
// ini_set('session.cookie_secure', 1);
```

---

### IP Spoofing Prevention

**Implementation**: Only trust REMOTE_ADDR

**Before (Vulnerable)**:
```php
function getUserIP() {
    if (!empty($_SERVER['HTTP_CLIENT_IP'])) {
        return $_SERVER['HTTP_CLIENT_IP'];
    } elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR'])) {
        return $_SERVER['HTTP_X_FORWARDED_FOR'];
    }
    return $_SERVER['REMOTE_ADDR'];
}
```

**After (Secure)**:
```php
function getUserIP() {
    // SECURITY: Only trust REMOTE_ADDR
    // HTTP_CLIENT_IP and HTTP_X_FORWARDED_FOR can be forged
    return $_SERVER['REMOTE_ADDR'];
}
```

**Impact**: Prevents attackers from forging IP addresses in logs

---

### Password Protection

**No Plaintext Exposure** (`/admin/hiveconfig.php`):

**Before (Vulnerable)**:
```php
<input type="password" value="<?php echo $password; ?>">
<input type="hidden" name="password" value="<?php echo $password; ?>">
```

**After (Secure)**:
```php
<input type="password" placeholder="Enter to change">
<input type="hidden" name="password" value="">
```

**Update Logic**:
```php
// Get current password from DB
$stmt = $conn->prepare("SELECT HT_PASSWORD FROM hiveconfig WHERE id=1");
$stmt->execute();
$current_pwd = $stmt->fetchColumn();

// Use new if provided, otherwise keep old
$HT_PASSWORD = !empty($_POST['HT_PASSWORD']) ? $_POST['HT_PASSWORD'] : $current_pwd;
```

---

### Information Disclosure Prevention

**Database Errors** (`/include/db-connect.php`):

**Before (Vulnerable)**:
```php
catch(PDOException $e) {
    echo "Connection failed: " . $e->getMessage();
}
```

**After (Secure)**:
```php
catch(PDOException $e) {
    error_log("Database connection failed: " . $e->getMessage());
    die("Database connection error. Please contact administrator.");
}
```

---

### Security Headers

**Implementation** (`/include/security-init.php`):

```php
header("X-Frame-Options: DENY");
header("X-Content-Type-Options: nosniff");
header("Referrer-Policy: strict-origin-when-cross-origin");
header("Content-Security-Policy: default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'; img-src 'self' data:;");
```

**Header Purposes**:
- `X-Frame-Options: DENY` - Prevents clickjacking attacks
- `X-Content-Type-Options: nosniff` - Prevents MIME type sniffing
- `Referrer-Policy` - Limits referrer information leakage
- `Content-Security-Policy` - Restricts resource loading

---

## Authentication

### Apache Basic Authentication

**Method**: HTTP Basic Auth with `.htpasswd` file

**Location**: `/home/HiveControl/www/.htpasswd`

**Default Username**: `admin`

**Supported Hash Formats**:
- bcrypt (`$2y$` prefix) - **Recommended**
- Apache MD5 (`$apr1$` prefix) - Legacy
- SHA1 (`{SHA}` prefix) - Legacy
- Traditional crypt() - Legacy

**Change Password**:
- Web interface: `/admin/changepassword.php`
- Command line: `htpasswd /home/HiveControl/www/.htpasswd admin`

---

## Best Practices

### For Developers

1. **Always Use Prepared Statements**:
   ```php
   // GOOD
   $stmt = $conn->prepare("SELECT * FROM logs WHERE type=?");
   $stmt->execute([$type]);

   // BAD
   $result = $conn->query("SELECT * FROM logs WHERE type='$type'");
   ```

2. **Always Escape Output**:
   ```php
   // GOOD
   echo htmlspecialchars($user_input);

   // BAD
   echo $user_input;
   ```

3. **Always Validate CSRF Tokens**:
   ```php
   // In forms
   <?php echo csrf_field(); ?>

   // In handlers
   require_csrf_token();
   ```

4. **Use Whitelists for Dynamic Values**:
   ```php
   $allowed = ['hivetempf', 'hiveweight'];
   if (!in_array($column, $allowed)) {
       die("Invalid column");
   }
   ```

### For Administrators

1. **Change Default Password**:
   - Use `/admin/changepassword.php` after installation
   - Use strong password (12+ characters, mixed case, numbers, symbols)

2. **Enable HTTPS** (recommended):
   - Install SSL certificate (Let's Encrypt)
   - Uncomment `session.cookie_secure` in `/include/security-init.php`

3. **Regular Backups**:
   - Use Admin → Backup & Restore
   - Store backups securely off-site
   - Test restores periodically

4. **Monitor Logs**:
   - Admin → System Commands → View Logs
   - Check for failed login attempts
   - Check for CSRF token failures
   - Review error messages

5. **Keep Software Updated**:
   - Run `sudo /home/HiveControl/upgrade.sh` regularly
   - Update Raspberry Pi OS: `sudo apt update && sudo apt upgrade`

6. **Restrict Network Access**:
   - Use firewall (ufw)
   - Only expose necessary ports
   - Consider VPN for remote access

---

## Security Checklist

### Installation
- [ ] Change default admin password
- [ ] Review `.htaccess` file
- [ ] Set proper file permissions
- [ ] Disable directory listing
- [ ] Review Apache configuration

### Configuration
- [ ] Use strong passwords
- [ ] Enable session security
- [ ] Configure HTTPS (if available)
- [ ] Set appropriate session timeout
- [ ] Review CSRF protection

### Maintenance
- [ ] Regular system updates
- [ ] Monitor security logs
- [ ] Review user access
- [ ] Test backup/restore
- [ ] Audit database permissions

### Monitoring
- [ ] Check for failed logins
- [ ] Review error logs
- [ ] Monitor system resources
- [ ] Check for suspicious activity
- [ ] Verify CSRF token validation

---

## Known Limitations

### Version 2.10

**Not Yet Implemented**:
1. Rate limiting on authentication
2. Two-factor authentication
3. Password complexity enforcement
4. Account lockout after failed attempts
5. Database encryption at rest
6. Backup file encryption

**Workarounds**:
- Use fail2ban for rate limiting
- Implement firewall rules
- Use VPN for remote access
- Enable full-disk encryption on Raspberry Pi

---

## Vulnerability Reporting

If you discover a security vulnerability:

1. **Do NOT** create a public GitHub issue
2. Email details to: security@hivecontrol.org
3. Include:
   - Description of vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if any)

We will respond within 48 hours and work on a fix.

---

## Security Audit History

### 2026-01-17 - Version 2.10

**Audit Results**:
- 13 vulnerabilities found
- 3 CRITICAL (SQL injection)
- 4 HIGH (IP spoofing, XSS, password exposure)
- 6 MEDIUM/LOW (CSRF, session, headers, errors)

**Status**: All CRITICAL and HIGH vulnerabilities fixed ✅

**Documentation**:
- [SECURITY_AUDIT_2026-01-17.md](../SECURITY_AUDIT_2026-01-17.md)
- [SECURITY_FIXES_APPLIED_2026-01-17.md](../SECURITY_FIXES_APPLIED_2026-01-17.md)

---

## References

- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [PHP Security Guide](https://www.php.net/manual/en/security.php)
- [SQL Injection Prevention](https://cheatsheetseries.owasp.org/cheatsheets/SQL_Injection_Prevention_Cheat_Sheet.html)
- [CSRF Prevention](https://cheatsheetseries.owasp.org/cheatsheets/Cross-Site_Request_Forgery_Prevention_Cheat_Sheet.html)
- [XSS Prevention](https://cheatsheetseries.owasp.org/cheatsheets/Cross_Site_Scripting_Prevention_Cheat_Sheet.html)

---

## See Also

- [Web Application Structure](WEB_APPLICATION.md)
- [Bash Scripts Reference](BASH_SCRIPTS.md)
- [Database Schema](DATABASE_SCHEMA.md)
- [Security Fixes Applied](../SECURITY_FIXES_APPLIED_2026-01-17.md)
