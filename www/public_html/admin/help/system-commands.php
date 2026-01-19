<h1><i class="fa fa-terminal"></i> System Commands</h1>

<div class="alert alert-warning">
    <strong><i class="fa fa-exclamation-triangle"></i> Important:</strong> System commands perform critical operations on your HiveControl installation. Use these commands carefully and only when necessary.
</div>

<h2>What are System Commands?</h2>
<p>System commands allow you to perform administrative tasks on your HiveControl installation, including:</p>
<ul>
    <li>Database maintenance and optimization</li>
    <li>System updates and upgrades</li>
    <li>Data cleanup and archiving</li>
    <li>Service restarts and diagnostics</li>
    <li>Log management</li>
</ul>

<h2>Available Commands</h2>

<div class="row">
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-database"></i> Database Maintenance</h4>
            <p><strong>Optimize Database</strong></p>
            <p>Optimizes database tables to improve performance and reclaim unused space.</p>
            <ul>
                <li>Run monthly for best performance</li>
                <li>Takes 1-5 minutes depending on data size</li>
                <li>Safe to run anytime</li>
            </ul>

            <p><strong>Repair Database</strong></p>
            <p>Repairs corrupted database tables if errors occur.</p>
            <ul>
                <li>Use only if you encounter database errors</li>
                <li>Backup before running</li>
                <li>May take several minutes</li>
            </ul>
        </div>
    </div>
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-trash"></i> Data Management</h4>
            <p><strong>Clean Old Data</strong></p>
            <p>Removes sensor readings older than specified date to free up space.</p>
            <ul>
                <li>Creates automatic backup first</li>
                <li>Cannot be undone without backup</li>
                <li>Useful for long-running installations</li>
            </ul>

            <p><strong>Archive Data</strong></p>
            <p>Archives old data to compressed files for long-term storage.</p>
            <ul>
                <li>Keeps data accessible but offline</li>
                <li>Reduces database size</li>
                <li>Archives stored in /backups/archives/</li>
            </ul>
        </div>
    </div>
</div>

<div class="row">
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-refresh"></i> Service Management</h4>
            <p><strong>Restart Data Collection</strong></p>
            <p>Restarts the background service that collects sensor data.</p>
            <ul>
                <li>Use if sensors stop reporting</li>
                <li>Brief interruption in data collection</li>
                <li>Automatically reconnects to sensors</li>
            </ul>

            <p><strong>Restart Web Server</strong></p>
            <p>Restarts the web interface (requires system permissions).</p>
            <ul>
                <li>Use after configuration changes</li>
                <li>Will disconnect all users briefly</li>
                <li>Automatic reconnection</li>
            </ul>
        </div>
    </div>
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-file-text"></i> Log Management</h4>
            <p><strong>View System Logs</strong></p>
            <p>Display recent system logs for troubleshooting.</p>
            <ul>
                <li>Shows errors, warnings, and info messages</li>
                <li>Filterable by date and severity</li>
                <li>Useful for diagnosing issues</li>
            </ul>

            <p><strong>Clear Logs</strong></p>
            <p>Archives and clears old log files.</p>
            <ul>
                <li>Frees disk space</li>
                <li>Logs archived before deletion</li>
                <li>Run when logs grow too large</li>
            </ul>
        </div>
    </div>
</div>

<h2>System Updates</h2>

<div class="panel panel-default">
    <div class="panel-heading">
        <strong><i class="fa fa-cloud-download"></i> Updating HiveControl</strong>
    </div>
    <div class="panel-body">
        <p><span class="step-number">1</span> <strong>Check for Updates:</strong> System will check GitHub for newer versions</p>
        <p><span class="step-number">2</span> <strong>Review Changes:</strong> Read release notes to understand what's new</p>
        <p><span class="step-number">3</span> <strong>Backup First:</strong> Always create a backup before updating</p>
        <p><span class="step-number">4</span> <strong>Download Update:</strong> System downloads and verifies update files</p>
        <p><span class="step-number">5</span> <strong>Apply Update:</strong> Installation automatically applies updates</p>
        <p><span class="step-number">6</span> <strong>Verify:</strong> Check system status page to confirm successful update</p>
    </div>
</div>

<div class="alert alert-warning">
    <strong>Before Updating:</strong>
    <ul style="margin-bottom: 0;">
        <li>Create a complete backup (Settings > Backup & Restore)</li>
        <li>Note your current version number</li>
        <li>Ensure you have database credentials available</li>
        <li>Plan for 5-10 minutes of downtime</li>
    </ul>
</div>

<h2>System Diagnostics</h2>

<h3><i class="fa fa-heartbeat"></i> Health Check</h3>
<p>Run a comprehensive system health check to verify:</p>
<ul>
    <li>Database connectivity</li>
    <li>Sensor communication</li>
    <li>Disk space availability</li>
    <li>Service status</li>
    <li>API connectivity (weather, etc.)</li>
    <li>File permissions</li>
</ul>

<h3><i class="fa fa-info-circle"></i> System Information</h3>
<p>View detailed information about your HiveControl installation:</p>
<ul>
    <li>HiveControl version</li>
    <li>PHP version and modules</li>
    <li>Database version and size</li>
    <li>Operating system details</li>
    <li>Available disk space</li>
    <li>Memory usage</li>
</ul>

<h2>Routine Maintenance Schedule</h2>

<table class="table table-bordered">
    <thead>
        <tr>
            <th>Frequency</th>
            <th>Task</th>
            <th>Purpose</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><strong>Weekly</strong></td>
            <td>Review system logs</td>
            <td>Catch issues early</td>
        </tr>
        <tr>
            <td><strong>Monthly</strong></td>
            <td>Optimize database</td>
            <td>Maintain performance</td>
        </tr>
        <tr>
            <td><strong>Monthly</strong></td>
            <td>Create backup</td>
            <td>Protect your data</td>
        </tr>
        <tr>
            <td><strong>Quarterly</strong></td>
            <td>Run health check</td>
            <td>Verify system integrity</td>
        </tr>
        <tr>
            <td><strong>Yearly</strong></td>
            <td>Archive old data</td>
            <td>Manage database size</td>
        </tr>
        <tr>
            <td><strong>As needed</strong></td>
            <td>Check for updates</td>
            <td>Stay current with features</td>
        </tr>
    </tbody>
</table>

<h2>Command Safety Guidelines</h2>

<div class="alert alert-danger">
    <strong><i class="fa fa-exclamation-circle"></i> High-Risk Commands</strong>
    <p>The following commands can cause data loss if used incorrectly:</p>
    <ul>
        <li><strong>Clean Old Data:</strong> Permanently deletes historical records</li>
        <li><strong>Repair Database:</strong> Can corrupt data if interrupted</li>
        <li><strong>Factory Reset:</strong> Erases all configuration and data</li>
    </ul>
    <p style="margin-bottom: 0;"><strong>Always backup before running high-risk commands!</strong></p>
</div>

<h2>Troubleshooting Common Issues</h2>

<h3>Sensors Not Reporting Data</h3>
<ol>
    <li>Check sensor connections and power</li>
    <li>Review system logs for errors</li>
    <li>Restart data collection service</li>
    <li>Verify sensor configuration in Instruments settings</li>
</ol>

<h3>Slow Dashboard Performance</h3>
<ol>
    <li>Run database optimization</li>
    <li>Clear old logs</li>
    <li>Archive historical data (older than 2 years)</li>
    <li>Check available disk space</li>
</ol>

<h3>Database Errors</h3>
<ol>
    <li>Create backup immediately</li>
    <li>Review error logs for specific issues</li>
    <li>Run database repair if corruption is suspected</li>
    <li>Check disk space - full disk can corrupt databases</li>
    <li>Contact support if errors persist</li>
</ol>

<h2>Best Practices</h2>

<ul>
    <li><strong>Schedule Maintenance:</strong> Set reminders for routine tasks</li>
    <li><strong>Monitor Logs:</strong> Check weekly for warnings or errors</li>
    <li><strong>Keep Updated:</strong> Apply updates within 1-2 months of release</li>
    <li><strong>Test in Backup:</strong> If unsure, test commands on a backup first</li>
    <li><strong>Document Changes:</strong> Keep notes on when you run commands</li>
    <li><strong>Have Rollback Plan:</strong> Know how to restore from backup</li>
</ul>

<h2>Getting Help</h2>

<p>If you encounter issues with system commands:</p>
<ul>
    <li>Check the Troubleshooting section of this help guide</li>
    <li>Review system logs for error details</li>
    <li>Visit the HiveControl GitHub issues page</li>
    <li>Join the HiveControl community forum</li>
    <li>Include log excerpts when reporting issues</li>
</ul>
