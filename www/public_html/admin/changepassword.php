<?PHP

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
include($_SERVER["DOCUMENT_ROOT"] . "/include/security-init.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

# Change Password System for HiveControl
# Version 1.0
# Author: Ryan Crum
# Date: 2026-01-17

//Check input for badness
function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}

function getUserIP()
{
    // SECURITY: Only trust REMOTE_ADDR to prevent IP spoofing
    // HTTP_CLIENT_IP and HTTP_X_FORWARDED_FOR can be easily forged by attackers
    return $_SERVER['REMOTE_ADDR'];
}

function loglocal($date, $program, $type, $message) {
  #Stores log entries locally
  include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

  // SECURITY FIX: Use proper parameterized query to prevent SQL injection
  $sth99 = $conn->prepare("INSERT INTO logs (date,program,type,message) VALUES (?,?,?,?)");
  $sth99->execute([$date, $program, $type, $message]);
  unset($sth99);
}

function apr1_md5($password, $salt) {
    // Apache MD5 algorithm implementation
    // This is the APR1 MD5 algorithm used by Apache htpasswd

    $len = strlen($password);
    $text = $password . '$apr1$' . $salt;
    $bin = pack("H32", md5($password . $salt . $password));

    for ($i = $len; $i > 0; $i -= 16) {
        $text .= substr($bin, 0, min(16, $i));
    }

    for ($i = $len; $i > 0; $i >>= 1) {
        $text .= ($i & 1) ? chr(0) : $password[0];
    }

    $bin = pack("H32", md5($text));

    for ($i = 0; $i < 1000; $i++) {
        $new = ($i & 1) ? $password : $bin;

        if ($i % 3) {
            $new .= $salt;
        }
        if ($i % 7) {
            $new .= $password;
        }

        $new .= ($i & 1) ? $bin : $password;
        $bin = pack("H32", md5($new));
    }

    $tmp = "";
    for ($i = 0; $i < 5; $i++) {
        $k = $i + 6;
        $j = $i + 12;
        if ($j == 16) {
            $j = 5;
        }
        $tmp = $bin[$i] . $bin[$k] . $bin[$j] . $tmp;
    }
    $tmp = chr(0) . chr(0) . $bin[11] . $tmp;
    $tmp = strtr(strrev(substr(base64_encode($tmp), 2)),
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
        "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    return "$" . "apr1" . "$" . $salt . "$" . $tmp;
}

function apr1_md5_verify($password, $hash) {
    // Apache MD5 verification
    if (strpos($hash, '$apr1$') !== 0) {
        return false;
    }

    $parts = explode('$', $hash);
    if (count($parts) < 4) {
        return false;
    }

    $salt = $parts[2];
    $computed_hash = apr1_md5($password, $salt);

    return $computed_hash === $hash;
}

// Get command
$action = test_input($_POST["action"] ?? "");

#Get system time
$shortName = exec('date +%Z');
$longName = timezone_name_from_abbr($shortName);
date_default_timezone_set($longName);

$now = date('Y-m-d H:i:s');
$user_ip = getUserIP();

// Define htpasswd file location
// DOCUMENT_ROOT is /home/HiveControl/www/public_html
// .htpasswd is at /home/HiveControl/www/.htpasswd (one level up from DOCUMENT_ROOT)
$htpasswd_file = $_SERVER["DOCUMENT_ROOT"] . "/../.htpasswd";
$username = "admin"; // HiveControl uses 'admin' as default username

// Handle Actions
$message = "";
$message_type = "";

if ($action == "change") {
    // SECURITY: Verify CSRF token for POST actions
    require_csrf_token();

    $current_password = $_POST["current_password"] ?? "";
    $new_password = test_input($_POST["new_password"] ?? "");
    $confirm_password = test_input($_POST["confirm_password"] ?? "");

    // Validation
    if (empty($current_password) || empty($new_password) || empty($confirm_password)) {
        $message = "All fields are required.";
        $message_type = "danger";
    } elseif ($new_password !== $confirm_password) {
        $message = "New password and confirmation do not match.";
        $message_type = "danger";
    } elseif (strlen($new_password) < 8) {
        $message = "Password must be at least 8 characters long.";
        $message_type = "danger";
    } else {
        // Verify current password
        if (!file_exists($htpasswd_file)) {
            $message = "Password file not found. Please contact administrator.";
            $message_type = "danger";
            loglocal($now, "PASSWORD", "ERROR", "Password file not found at $htpasswd_file by IP $user_ip");
        } else {
            // Read current htpasswd file
            $htpasswd_content = file_get_contents($htpasswd_file);
            $lines = explode("\n", trim($htpasswd_content));
            $current_hash = "";

            foreach ($lines as $line) {
                if (strpos($line, "$username:") === 0) {
                    $parts = explode(":", $line, 2);
                    if (count($parts) == 2) {
                        $current_hash = $parts[1];
                    }
                    break;
                }
            }

            // Verify current password using Apache's password verification
            $verified = false;
            if (!empty($current_hash)) {
                // Apache htpasswd can use different hash types:
                // $apr1$ = Apache MD5
                // $2y$ or $2a$ = bcrypt
                // {SHA} = SHA1
                // others = traditional crypt()

                if (strpos($current_hash, '$apr1$') === 0) {
                    // Apache MD5 format - use our custom verification function
                    $verified = apr1_md5_verify($current_password, $current_hash);
                } elseif (strpos($current_hash, '$2y$') === 0 || strpos($current_hash, '$2a$') === 0) {
                    // bcrypt format
                    if (password_verify($current_password, $current_hash)) {
                        $verified = true;
                    }
                } elseif (strpos($current_hash, '{SHA}') === 0) {
                    // SHA1 format (base64 encoded)
                    $sha_hash = '{SHA}' . base64_encode(sha1($current_password, true));
                    if ($sha_hash === $current_hash) {
                        $verified = true;
                    }
                } else {
                    // Try traditional crypt()
                    if (crypt($current_password, $current_hash) === $current_hash) {
                        $verified = true;
                    }
                }
            }

            if (!$verified) {
                // Determine hash type for error message
                $hash_type = "unknown";
                if (strpos($current_hash, '$apr1$') === 0) {
                    $hash_type = "Apache MD5";
                } elseif (strpos($current_hash, '$2y$') === 0 || strpos($current_hash, '$2a$') === 0) {
                    $hash_type = "bcrypt";
                } elseif (strpos($current_hash, '{SHA}') === 0) {
                    $hash_type = "SHA1";
                } else {
                    $hash_type = "crypt";
                }

                $message = "Current password is incorrect. (Hash type detected: $hash_type)";
                $message_type = "danger";
                loglocal($now, "PASSWORD", "ERROR", "Failed password change attempt - incorrect current password (hash type: $hash_type) by IP $user_ip");
            } else {
                // Generate new password hash using bcrypt
                $new_hash = password_hash($new_password, PASSWORD_BCRYPT);

                // Update htpasswd file
                $new_content = "$username:$new_hash\n";

                if (file_put_contents($htpasswd_file, $new_content) !== false) {
                    $message = "Password changed successfully. You will need to log in again with your new password.";
                    $message_type = "success";
                    loglocal($now, "PASSWORD", "SUCCESS", "Password changed successfully by IP $user_ip");
                } else {
                    $message = "Failed to write password file. Check file permissions.";
                    $message_type = "danger";
                    loglocal($now, "PASSWORD", "ERROR", "Failed to write password file by IP $user_ip");
                }
            }
        }
    }
}

?>
<!DOCTYPE html>
<html lang="en">

    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Change Password</h1>
                </div>
                <!-- /.col-lg-12 -->
            </div>

            <!-- Display Messages -->
            <?php if (!empty($message)): ?>
            <div class="row">
                <div class="col-lg-12">
                    <div class="alert alert-<?php echo $message_type; ?> alert-dismissible">
                        <button type="button" class="close" data-dismiss="alert">&times;</button>
                        <?php echo $message; ?>
                    </div>
                </div>
            </div>
            <?php endif; ?>

            <!-- Change Password Form -->
            <div class="row">
                <div class="col-lg-6">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-lock"></i> Change Admin Password
                        </div>
                        <div class="panel-body">
                            <form method="POST" action="?">
                                <input type="hidden" name="action" value="change">
                                <?php echo csrf_field(); ?>

                                <div class="form-group">
                                    <label>Current Password</label>
                                    <input type="password" name="current_password" class="form-control" required autocomplete="current-password">
                                    <p class="help-block">Enter your current admin password</p>
                                </div>

                                <div class="form-group">
                                    <label>New Password</label>
                                    <input type="password" name="new_password" class="form-control" required minlength="8" autocomplete="new-password">
                                    <p class="help-block">Minimum 8 characters</p>
                                </div>

                                <div class="form-group">
                                    <label>Confirm New Password</label>
                                    <input type="password" name="confirm_password" class="form-control" required minlength="8" autocomplete="new-password">
                                    <p class="help-block">Re-enter your new password</p>
                                </div>

                                <button type="submit" class="btn btn-primary">
                                    <i class="fa fa-key"></i> Change Password
                                </button>
                                <a href="/pages/index.php" class="btn btn-default">Cancel</a>
                            </form>
                        </div>
                    </div>
                </div>

                <div class="col-lg-6">
                    <div class="panel panel-info">
                        <div class="panel-heading">
                            <i class="fa fa-info-circle"></i> Password Requirements
                        </div>
                        <div class="panel-body">
                            <p><strong>Your password must meet the following requirements:</strong></p>
                            <ul>
                                <li>Minimum 8 characters long</li>
                                <li>Current password must be verified</li>
                                <li>New password must be confirmed by entering twice</li>
                            </ul>

                            <p><strong>Password Best Practices:</strong></p>
                            <ul>
                                <li>Use a mix of uppercase and lowercase letters</li>
                                <li>Include numbers and special characters</li>
                                <li>Avoid common words or personal information</li>
                                <li>Don't reuse passwords from other accounts</li>
                                <li>Consider using a password manager</li>
                            </ul>

                            <div class="alert alert-warning">
                                <i class="fa fa-warning"></i> <strong>Important:</strong> After changing your password, you will need to log in again using the new password.
                            </div>
                        </div>
                    </div>
                </div>
            </div>

    </div>
    <!-- /#wrapper -->

    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>

    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>

    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>
</html>
