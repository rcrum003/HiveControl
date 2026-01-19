<h1><i class="fa fa-life-ring"></i> Backup & Restore</h1>

<div class="alert alert-info">
    <strong>Protect Your Data:</strong> Regular backups ensure you never lose your valuable hive monitoring data. Learn how to create, manage, and restore backups of your HiveControl system.
</div>

<h2>Why Backup?</h2>

<p>Backups protect against:</p>
<ul>
    <li><strong>Hardware Failures:</strong> SD card corruption, disk failures</li>
    <li><strong>Accidental Deletion:</strong> Mistakenly deleted configuration or data</li>
    <li><strong>System Updates:</strong> Issues during software updates</li>
    <li><strong>Configuration Errors:</strong> Incorrect settings that break the system</li>
    <li><strong>Migration:</strong> Moving to new hardware or reinstalling</li>
</ul>

<h2>What Gets Backed Up?</h2>

<div class="row">
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-database"></i> Database Content</h4>
            <ul>
                <li>All sensor readings (temperature, weight, humidity, etc.)</li>
                <li>Historical data and trends</li>
                <li>System configuration settings</li>
                <li>Instrument and sensor settings</li>
                <li>User accounts and passwords</li>
                <li>Calibration data</li>
            </ul>
        </div>
    </div>
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-cog"></i> Configuration Files</h4>
            <ul>
                <li>HiveControl settings</li>
                <li>API keys and credentials</li>
                <li>Custom scripts and modifications</li>
                <li>Alert rules and thresholds</li>
                <li>Display preferences</li>
            </ul>
        </div>
    </div>
</div>

<h2>Creating a Backup</h2>

<div class="panel panel-default">
    <div class="panel-heading">
        <strong>Manual Backup Process</strong>
    </div>
    <div class="panel-body">
        <p><span class="step-number">1</span> Navigate to <strong>Settings > Backup & Restore</strong></p>
        <p><span class="step-number">2</span> Click the <strong>"Create New Backup"</strong> button</p>
        <p><span class="step-number">3</span> Choose backup options:
            <ul>
                <li><strong>Full Backup:</strong> Everything (recommended)</li>
                <li><strong>Database Only:</strong> Just the data</li>
                <li><strong>Configuration Only:</strong> Settings and credentials</li>
            </ul>
        </p>
        <p><span class="step-number">4</span> Wait for backup to complete (1-10 minutes depending on data size)</p>
        <p><span class="step-number">5</span> Download the backup file to your computer for safekeeping</p>
        <p><span class="step-number">6</span> Verify the backup file size is reasonable</p>
    </div>
</div>

<h3><i class="fa fa-clock-o"></i> Automatic Backups</h3>
<p>Configure HiveControl to create automatic backups on a schedule:</p>

<table class="table table-bordered">
    <thead>
        <tr>
            <th>Frequency</th>
            <th>Recommended For</th>
            <th>Storage Impact</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><strong>Daily</strong></td>
            <td>Active monitoring during bee season</td>
            <td>High - clean up old backups weekly</td>
        </tr>
        <tr>
            <td><strong>Weekly</strong></td>
            <td>General use, balanced approach</td>
            <td>Medium - manageable long-term</td>
        </tr>
        <tr>
            <td><strong>Monthly</strong></td>
            <td>Winter/dormant periods</td>
            <td>Low - minimal storage needed</td>
        </tr>
    </tbody>
</table>

<div class="alert alert-warning">
    <strong>Storage Management:</strong> Automatic backups accumulate over time. Configure retention settings to keep only the most recent backups (e.g., keep last 30 days).
</div>

<h2>Backup Storage Options</h2>

<div class="row">
    <div class="col-md-4">
        <div class="feature-box">
            <h4><i class="fa fa-server"></i> Local Storage</h4>
            <p><strong>Default location:</strong> /var/backups/hivecontrol/</p>
            <ul>
                <li>Fast and automatic</li>
                <li>No internet required</li>
                <li>Vulnerable to hardware failure</li>
            </ul>
            <p><em>Good for: Quick recovery, recent backups</em></p>
        </div>
    </div>
    <div class="col-md-4">
        <div class="feature-box">
            <h4><i class="fa fa-download"></i> Manual Download</h4>
            <p>Download backups to your computer</p>
            <ul>
                <li>Complete control</li>
                <li>Safe from hardware issues</li>
                <li>Requires manual action</li>
            </ul>
            <p><em>Good for: Important milestones, major changes</em></p>
        </div>
    </div>
    <div class="col-md-4">
        <div class="feature-box">
            <h4><i class="fa fa-cloud"></i> Cloud Storage</h4>
            <p>Sync backups to cloud services</p>
            <ul>
                <li>Automatic offsite storage</li>
                <li>Protected from local failures</li>
                <li>Requires configuration</li>
            </ul>
            <p><em>Good for: Long-term archival, disaster recovery</em></p>
        </div>
    </div>
</div>

<h2>Restoring from Backup</h2>

<div class="panel panel-danger">
    <div class="panel-heading">
        <strong><i class="fa fa-exclamation-triangle"></i> Important Warning</strong>
    </div>
    <div class="panel-body">
        <p>Restoring a backup will <strong>overwrite all current data</strong> and settings. This action cannot be undone!</p>
        <p><strong>Before restoring:</strong></p>
        <ul>
            <li>Create a backup of the current system state</li>
            <li>Verify the backup file you're restoring is the correct one</li>
            <li>Check the backup date to ensure it's what you need</li>
            <li>Note that any data collected after the backup date will be lost</li>
        </ul>
    </div>
</div>

<div class="panel panel-default">
    <div class="panel-heading">
        <strong>Restore Process</strong>
    </div>
    <div class="panel-body">
        <p><span class="step-number">1</span> Navigate to <strong>Settings > Backup & Restore</strong></p>
        <p><span class="step-number">2</span> In the "Restore" section, click <strong>"Choose File"</strong></p>
        <p><span class="step-number">3</span> Select your backup file (.zip or .sql format)</p>
        <p><span class="step-number">4</span> Verify the backup details shown (date, size, contents)</p>
        <p><span class="step-number">5</span> Check the confirmation box acknowledging data will be overwritten</p>
        <p><span class="step-number">6</span> Click <strong>"Restore Backup"</strong></p>
        <p><span class="step-number">7</span> Wait for restore to complete (do not close browser)</p>
        <p><span class="step-number">8</span> System will restart automatically</p>
        <p><span class="step-number">9</span> Log back in and verify data is correct</p>
    </div>
</div>

<h2>Backup File Management</h2>

<h3><i class="fa fa-folder-open"></i> Viewing Backups</h3>
<p>The Backup & Restore page shows all available backups:</p>
<ul>
    <li>Backup date and time</li>
    <li>File size</li>
    <li>Backup type (full, database only, etc.)</li>
    <li>Storage location</li>
    <li>Quick actions (download, restore, delete)</li>
</ul>

<h3><i class="fa fa-trash"></i> Deleting Old Backups</h3>
<p>Clean up old backups to free disk space:</p>
<ol>
    <li>Review backup list and identify backups you no longer need</li>
    <li>Always keep at least 2-3 recent backups</li>
    <li>Download important backups before deleting from server</li>
    <li>Use "Delete Selected" to remove multiple backups at once</li>
</ol>

<h2>Best Practices</h2>

<div class="row">
    <div class="col-md-6">
        <h3><i class="fa fa-check-circle"></i> Do's</h3>
        <ul>
            <li>Create backup before system updates</li>
            <li>Create backup before changing configuration</li>
            <li>Test restores periodically to ensure backups work</li>
            <li>Store backups in multiple locations</li>
            <li>Document what changes prompted each manual backup</li>
            <li>Keep at least one backup per month for a year</li>
            <li>Download backups before major system changes</li>
        </ul>
    </div>
    <div class="col-md-6">
        <h3><i class="fa fa-times-circle"></i> Don'ts</h3>
        <ul>
            <li>Don't rely solely on automatic backups</li>
            <li>Don't store backups only on the same device</li>
            <li>Don't delete backups without downloading first</li>
            <li>Don't restore backups without verifying the date</li>
            <li>Don't interrupt restore process</li>
            <li>Don't forget to backup before troubleshooting</li>
            <li>Don't assume backups work - test them!</li>
        </ul>
    </div>
</div>

<h2>Backup Strategies</h2>

<h3>3-2-1 Backup Rule</h3>
<div class="feature-box">
    <p>Professional backup strategy for critical data:</p>
    <ul>
        <li><strong>3</strong> copies of your data (original + 2 backups)</li>
        <li><strong>2</strong> different storage types (e.g., server + external drive)</li>
        <li><strong>1</strong> offsite backup (cloud or different physical location)</li>
    </ul>
    <p><em>Example: Live system + local backup + cloud backup</em></p>
</div>

<h3>Seasonal Strategy</h3>
<table class="table table-bordered">
    <thead>
        <tr>
            <th>Season</th>
            <th>Backup Frequency</th>
            <th>Reason</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><strong>Active Season (Spring/Summer)</strong></td>
            <td>Daily automatic, Weekly manual</td>
            <td>High activity, rapid data growth</td>
        </tr>
        <tr>
            <td><strong>Shoulder Season (Fall)</strong></td>
            <td>Weekly automatic</td>
            <td>Moderate activity</td>
        </tr>
        <tr>
            <td><strong>Winter</strong></td>
            <td>Monthly automatic</td>
            <td>Low activity, less frequent changes</td>
        </tr>
    </tbody>
</table>

<h2>Troubleshooting</h2>

<h3>Backup Creation Fails</h3>
<ul>
    <li>Check available disk space (need at least 2x database size)</li>
    <li>Verify backup directory has write permissions</li>
    <li>Review error logs for specific issues</li>
    <li>Try creating database-only backup to isolate issue</li>
</ul>

<h3>Restore Doesn't Complete</h3>
<ul>
    <li>Verify backup file is not corrupted (check file size)</li>
    <li>Ensure enough disk space for restore</li>
    <li>Check database credentials are correct</li>
    <li>Review restore log for error details</li>
</ul>

<h3>Restored Data Looks Wrong</h3>
<ul>
    <li>Verify you restored the correct backup file</li>
    <li>Check backup date matches expectations</li>
    <li>Clear browser cache and refresh</li>
    <li>Check timezone settings</li>
</ul>

<h2>Advanced: Command Line Backups</h2>

<p>For advanced users, backups can be created via command line:</p>

<div class="code-block">
# Create database backup<br>
mysqldump -u username -p hivecontrol > backup_$(date +%Y%m%d).sql<br>
<br>
# Create full backup with compression<br>
tar -czf hivecontrol_backup_$(date +%Y%m%d).tar.gz /var/www/html/hivecontrol /var/backups/hivecontrol
</div>

<p>Schedule automatic backups using cron:</p>
<div class="code-block">
# Daily backup at 2 AM<br>
0 2 * * * /usr/local/bin/hivecontrol-backup.sh
</div>
