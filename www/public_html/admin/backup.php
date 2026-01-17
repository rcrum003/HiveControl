<?PHP

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

# Backup and Restore System for HiveControl
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
    $client  = @$_SERVER['HTTP_CLIENT_IP'];
    $forward = @$_SERVER['HTTP_X_FORWARDED_FOR'];
    $remote  = $_SERVER['REMOTE_ADDR'];

    if(filter_var($client, FILTER_VALIDATE_IP))
    {
        $ip = $client;
    }
    elseif(filter_var($forward, FILTER_VALIDATE_IP))
    {
        $ip = $forward;
    }
    else
    {
        $ip = $remote;
    }

    return $ip;
}

function loglocal($date, $program, $type, $message) {
  #Stores log entries locally
  include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

  $sth99 = $conn->prepare("insert into logs (date,program,type,message) values (\"$date\",\"$program\",\"$type\",\"$message\")");
  $sth99->execute();
  unset($sth99);
}

function formatBytes($bytes, $precision = 2) {
    $units = array('B', 'KB', 'MB', 'GB', 'TB');
    $bytes = max($bytes, 0);
    $pow = floor(($bytes ? log($bytes) : 0) / log(1024));
    $pow = min($pow, count($units) - 1);
    $bytes /= pow(1024, $pow);
    return round($bytes, $precision) . ' ' . $units[$pow];
}

// Get command
$command = test_input($_GET["command"] ?? "");
$action = test_input($_GET["action"] ?? "");
$filename = test_input($_GET["file"] ?? "");

#Get system time
$shortName = exec('date +%Z');
$longName = timezone_name_from_abbr($shortName);
date_default_timezone_set($longName);

$now = date('Y-m-d H:i:s');
$user_ip = getUserIP();

// Define backup directory
$backup_dir = $_SERVER["DOCUMENT_ROOT"] . "/../data/backups/";
$db_path = $_SERVER["DOCUMENT_ROOT"] . "/../data/hive-data.db";

// Create backup directory if it doesn't exist
if (!file_exists($backup_dir)) {
    mkdir($backup_dir, 0755, true);
    // Create .htaccess to deny web access
    file_put_contents($backup_dir . ".htaccess", "Deny from all");
}

// Handle Actions
$message = "";
$message_type = "";

if ($action == "create") {
    $backup_type = test_input($_POST["backup_type"] ?? "full");
    $timestamp = date('Y-m-d_His');
    $backup_file = "hivecontrol_{$backup_type}_{$timestamp}.db";
    $backup_path = $backup_dir . $backup_file;

    try {
        if ($backup_type == "full") {
            // Full database backup using SQLite backup command
            $backup_db = new SQLite3($backup_path);
            $source_db = new SQLite3($db_path);
            $source_db->backup($backup_db);
            $source_db->close();
            $backup_db->close();

            $size = filesize($backup_path);
            loglocal($now, "BACKUP", "SUCCESS", "Full backup created: $backup_file (" . formatBytes($size) . ") by IP $user_ip");
            $message = "Full backup created successfully: $backup_file (" . formatBytes($size) . ")";
            $message_type = "success";

        } elseif ($backup_type == "config") {
            // Configuration-only backup (export hiveconfig table)
            $backup_file = "hivecontrol_config_{$timestamp}.sql";
            $backup_path = $backup_dir . $backup_file;

            $source_db = new SQLite3($db_path);
            $result = $source_db->query("SELECT * FROM hiveconfig");
            $row = $result->fetchArray(SQLITE3_ASSOC);

            // Create SQL INSERT statement
            $columns = implode(', ', array_keys($row));
            $values = array_map(function($v) { return "'" . SQLite3::escapeString($v) . "'"; }, array_values($row));
            $values_str = implode(', ', $values);

            $sql = "-- HiveControl Configuration Backup\n";
            $sql .= "-- Created: $now\n";
            $sql .= "-- Type: Configuration Only\n\n";
            $sql .= "DELETE FROM hiveconfig;\n";
            $sql .= "INSERT INTO hiveconfig ($columns) VALUES ($values_str);\n";

            file_put_contents($backup_path, $sql);
            $source_db->close();

            $size = filesize($backup_path);
            loglocal($now, "BACKUP", "SUCCESS", "Config backup created: $backup_file (" . formatBytes($size) . ") by IP $user_ip");
            $message = "Configuration backup created successfully: $backup_file (" . formatBytes($size) . ")";
            $message_type = "success";
        }
    } catch (Exception $e) {
        loglocal($now, "BACKUP", "ERROR", "Backup failed: " . $e->getMessage() . " by IP $user_ip");
        $message = "Backup failed: " . $e->getMessage();
        $message_type = "danger";
    }
}

if ($action == "restore" && !empty($filename)) {
    $backup_path = $backup_dir . basename($filename);

    if (!file_exists($backup_path)) {
        $message = "Backup file not found: $filename";
        $message_type = "danger";
    } else {
        try {
            // Create safety backup before restore
            $safety_timestamp = date('Y-m-d_His');
            $safety_file = "hivecontrol_prerestore_{$safety_timestamp}.db";
            $safety_path = $backup_dir . $safety_file;
            copy($db_path, $safety_path);

            loglocal($now, "RESTORE", "INFO", "Safety backup created: $safety_file before restore of $filename by IP $user_ip");

            // Pause data collection
            $conn->exec("UPDATE hiveconfig SET RUN='no' WHERE id=1");
            sleep(2); // Give running script time to finish

            // Restore based on file type
            if (pathinfo($filename, PATHINFO_EXTENSION) == 'db') {
                // Full database restore
                copy($backup_path, $db_path);
                $message = "Database restored successfully from: $filename";
                $message_type = "success";
            } elseif (pathinfo($filename, PATHINFO_EXTENSION) == 'sql') {
                // SQL file restore (config only)
                $sql = file_get_contents($backup_path);
                $conn->exec($sql);
                $message = "Configuration restored successfully from: $filename";
                $message_type = "success";
            }

            loglocal($now, "RESTORE", "SUCCESS", "Restore completed from $filename by IP $user_ip");

            // Resume data collection
            $conn->exec("UPDATE hiveconfig SET RUN='yes' WHERE id=1");

        } catch (Exception $e) {
            loglocal($now, "RESTORE", "ERROR", "Restore failed: " . $e->getMessage() . " by IP $user_ip");
            $message = "Restore failed: " . $e->getMessage();
            $message_type = "danger";

            // Try to restore from safety backup
            if (file_exists($safety_path)) {
                copy($safety_path, $db_path);
                $message .= " Safety backup has been restored.";
            }
        }
    }
}

if ($action == "delete" && !empty($filename)) {
    $backup_path = $backup_dir . basename($filename);

    if (file_exists($backup_path)) {
        unlink($backup_path);
        loglocal($now, "BACKUP", "INFO", "Backup deleted: $filename by IP $user_ip");
        $message = "Backup deleted: $filename";
        $message_type = "info";
    } else {
        $message = "Backup file not found: $filename";
        $message_type = "danger";
    }
}

if ($action == "download" && !empty($filename)) {
    $backup_path = $backup_dir . basename($filename);

    if (file_exists($backup_path)) {
        loglocal($now, "BACKUP", "INFO", "Backup downloaded: $filename by IP $user_ip");

        header('Content-Description: File Transfer');
        header('Content-Type: application/octet-stream');
        header('Content-Disposition: attachment; filename="'.basename($backup_path).'"');
        header('Expires: 0');
        header('Cache-Control: must-revalidate');
        header('Pragma: public');
        header('Content-Length: ' . filesize($backup_path));
        readfile($backup_path);
        exit;
    }
}

// Get list of existing backups
$backups = array();
if (is_dir($backup_dir)) {
    $files = scandir($backup_dir);
    foreach ($files as $file) {
        if ($file != "." && $file != ".." && $file != ".htaccess") {
            $filepath = $backup_dir . $file;
            $backups[] = array(
                'filename' => $file,
                'size' => filesize($filepath),
                'date' => date('Y-m-d H:i:s', filemtime($filepath)),
                'type' => (strpos($file, '_config_') !== false) ? 'Config Only' : ((strpos($file, '_prerestore_') !== false) ? 'Safety Backup' : 'Full Backup')
            );
        }
    }
    // Sort by date, newest first
    usort($backups, function($a, $b) {
        return strcmp($b['date'], $a['date']);
    });
}

// Get database size for estimates
$db_size = file_exists($db_path) ? filesize($db_path) : 0;

?>
<!DOCTYPE html>
<html lang="en">

    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

    <div id="wrapper">
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Backup & Restore</h1>
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

            <!-- Tabs -->
            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-body">
                            <!-- Nav tabs -->
                            <ul class="nav nav-tabs">
                                <li class="active"><a href="#backup-tab" data-toggle="tab">Create Backup</a></li>
                                <li><a href="#restore-tab" data-toggle="tab">Restore</a></li>
                                <li><a href="#manage-tab" data-toggle="tab">Manage Backups</a></li>
                            </ul>

                            <!-- Tab panes -->
                            <div class="tab-content">
                                <!-- Backup Tab -->
                                <div class="tab-pane fade in active" id="backup-tab">
                                    <br>
                                    <h4>Create New Backup</h4>
                                    <p>Create a backup of your HiveControl system. Choose between a full backup (database + configuration) or configuration only.</p>

                                    <div class="row">
                                        <div class="col-lg-6">
                                            <form method="POST" action="?action=create">
                                                <div class="form-group">
                                                    <label>Backup Type</label>
                                                    <select name="backup_type" class="form-control" id="backup-type-select">
                                                        <option value="full">Full Backup (Recommended)</option>
                                                        <option value="config">Configuration Only</option>
                                                    </select>
                                                    <p class="help-block" id="backup-help">
                                                        Full backup includes all historical data and configuration.
                                                        Estimated size: <strong><?php echo formatBytes($db_size); ?></strong>
                                                    </p>
                                                </div>
                                                <button type="submit" class="btn btn-primary">
                                                    <i class="fa fa-download"></i> Create Backup
                                                </button>
                                            </form>
                                        </div>
                                        <div class="col-lg-6">
                                            <div class="panel panel-info">
                                                <div class="panel-heading">
                                                    <i class="fa fa-info-circle"></i> Backup Information
                                                </div>
                                                <div class="panel-body">
                                                    <p><strong>Full Backup:</strong> Complete database including all historical measurements, configuration, and logs.</p>
                                                    <p><strong>Configuration Only:</strong> Just your settings (hive identity, sensors, API keys, UI preferences). Historical data not included.</p>
                                                    <p><strong>Current Database Size:</strong> <?php echo formatBytes($db_size); ?></p>
                                                </div>
                                            </div>
                                        </div>
                                    </div>
                                </div>

                                <!-- Restore Tab -->
                                <div class="tab-pane fade" id="restore-tab">
                                    <br>
                                    <h4>Restore from Backup</h4>
                                    <div class="alert alert-warning">
                                        <strong>Warning!</strong> Restoring will replace your current database. A safety backup will be created automatically before restore.
                                    </div>

                                    <?php if (count($backups) > 0): ?>
                                    <table class="table table-striped table-bordered">
                                        <thead>
                                            <tr>
                                                <th>Filename</th>
                                                <th>Type</th>
                                                <th>Date</th>
                                                <th>Size</th>
                                                <th>Actions</th>
                                            </tr>
                                        </thead>
                                        <tbody>
                                            <?php foreach ($backups as $backup): ?>
                                            <?php if (strpos($backup['filename'], '_prerestore_') === false): ?>
                                            <tr>
                                                <td><?php echo $backup['filename']; ?></td>
                                                <td>
                                                    <span class="label label-<?php echo ($backup['type'] == 'Full Backup') ? 'primary' : 'info'; ?>">
                                                        <?php echo $backup['type']; ?>
                                                    </span>
                                                </td>
                                                <td><?php echo $backup['date']; ?></td>
                                                <td><?php echo formatBytes($backup['size']); ?></td>
                                                <td>
                                                    <button class="btn btn-warning btn-sm" onclick="confirmRestore('<?php echo $backup['filename']; ?>')">
                                                        <i class="fa fa-refresh"></i> Restore
                                                    </button>
                                                </td>
                                            </tr>
                                            <?php endif; ?>
                                            <?php endforeach; ?>
                                        </tbody>
                                    </table>
                                    <?php else: ?>
                                    <p>No backups available. Create a backup first.</p>
                                    <?php endif; ?>
                                </div>

                                <!-- Manage Backups Tab -->
                                <div class="tab-pane fade" id="manage-tab">
                                    <br>
                                    <h4>Manage Existing Backups</h4>
                                    <p>Download or delete existing backups. Total backups: <strong><?php echo count($backups); ?></strong></p>

                                    <?php if (count($backups) > 0): ?>
                                    <table class="table table-striped table-bordered table-hover">
                                        <thead>
                                            <tr>
                                                <th>Filename</th>
                                                <th>Type</th>
                                                <th>Date</th>
                                                <th>Size</th>
                                                <th>Actions</th>
                                            </tr>
                                        </thead>
                                        <tbody>
                                            <?php foreach ($backups as $backup): ?>
                                            <tr>
                                                <td><?php echo $backup['filename']; ?></td>
                                                <td>
                                                    <span class="label label-<?php echo ($backup['type'] == 'Full Backup') ? 'primary' : (($backup['type'] == 'Config Only') ? 'info' : 'default'); ?>">
                                                        <?php echo $backup['type']; ?>
                                                    </span>
                                                </td>
                                                <td><?php echo $backup['date']; ?></td>
                                                <td><?php echo formatBytes($backup['size']); ?></td>
                                                <td>
                                                    <a href="?action=download&file=<?php echo urlencode($backup['filename']); ?>" class="btn btn-success btn-sm">
                                                        <i class="fa fa-download"></i> Download
                                                    </a>
                                                    <button class="btn btn-danger btn-sm" onclick="confirmDelete('<?php echo $backup['filename']; ?>')">
                                                        <i class="fa fa-trash"></i> Delete
                                                    </button>
                                                </td>
                                            </tr>
                                            <?php endforeach; ?>
                                        </tbody>
                                    </table>
                                    <?php else: ?>
                                    <p>No backups available.</p>
                                    <?php endif; ?>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>

    </div>
    <!-- /#wrapper -->

<!-- Confirmation Modals -->
<div class="modal fade" id="restoreModal" tabindex="-1" role="dialog">
    <div class="modal-dialog">
        <div class="modal-content">
            <div class="modal-header">
                <button type="button" class="close" data-dismiss="modal">&times;</button>
                <h4 class="modal-title">Confirm Restore</h4>
            </div>
            <div class="modal-body">
                <p><strong>Are you sure you want to restore from this backup?</strong></p>
                <p>Filename: <span id="restore-filename"></span></p>
                <p class="text-danger">This will replace your current database. A safety backup will be created automatically.</p>
            </div>
            <div class="modal-footer">
                <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                <a href="#" id="restore-confirm-btn" class="btn btn-warning">Restore Now</a>
            </div>
        </div>
    </div>
</div>

<div class="modal fade" id="deleteModal" tabindex="-1" role="dialog">
    <div class="modal-dialog">
        <div class="modal-content">
            <div class="modal-header">
                <button type="button" class="close" data-dismiss="modal">&times;</button>
                <h4 class="modal-title">Confirm Delete</h4>
            </div>
            <div class="modal-body">
                <p><strong>Are you sure you want to delete this backup?</strong></p>
                <p>Filename: <span id="delete-filename"></span></p>
                <p class="text-danger">This action cannot be undone.</p>
            </div>
            <div class="modal-footer">
                <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                <a href="#" id="delete-confirm-btn" class="btn btn-danger">Delete</a>
            </div>
        </div>
    </div>
</div>

<script>
// Update backup help text based on selection
document.getElementById('backup-type-select').addEventListener('change', function() {
    var helpText = document.getElementById('backup-help');
    if (this.value == 'full') {
        helpText.innerHTML = 'Full backup includes all historical data and configuration. Estimated size: <strong><?php echo formatBytes($db_size); ?></strong>';
    } else {
        helpText.innerHTML = 'Configuration only backup includes just your settings (no historical data). Estimated size: <strong>~50 KB</strong>';
    }
});

function confirmRestore(filename) {
    document.getElementById('restore-filename').textContent = filename;
    document.getElementById('restore-confirm-btn').href = '?action=restore&file=' + encodeURIComponent(filename);
    $('#restoreModal').modal('show');
}

function confirmDelete(filename) {
    document.getElementById('delete-filename').textContent = filename;
    document.getElementById('delete-confirm-btn').href = '?action=delete&file=' + encodeURIComponent(filename);
    $('#deleteModal').modal('show');
}
</script>

</body>
</html>
