<h1><i class="fa fa-wrench"></i> Troubleshooting Guide</h1>

<p>Common issues and solutions for HiveControl.</p>

<h2>Dashboard Shows No Data</h2>

<div class="feature-box">
    <h4>Symptoms</h4>
    <ul>
        <li>Dashboard panels show zeros or "NA"</li>
        <li>Charts are empty</li>
        <li>No recent data displayed</li>
    </ul>

    <h4>Solutions</h4>

    <p><span class="step-number">1</span> <strong>Complete Initial Setup</strong></p>
    <ul>
        <li>Verify all required fields in Basic Settings are filled</li>
        <li>Ensure Hive Name, API Key, City, and State are set</li>
        <li>If any are missing, the setup wizard should appear</li>
    </ul>

    <p><span class="step-number">2</span> <strong>Check Instruments Configuration</strong></p>
    <ul>
        <li>Go to Settings → Instruments</li>
        <li>Ensure instruments are Enabled</li>
        <li>Click "Read Sensors" to test live readings</li>
        <li>If readings appear, sensors are working</li>
    </ul>

    <p><span class="step-number">3</span> <strong>Verify Data Collection</strong></p>
    <ul>
        <li>Data collection script may not be running</li>
        <li>Check system logs for errors</li>
        <li>Restart data collection service</li>
    </ul>

    <p><span class="step-number">4</span> <strong>Wait for Data Accumulation</strong></p>
    <ul>
        <li>Charts need time to populate</li>
        <li>Allow 1-2 hours for initial data collection</li>
        <li>Check back after several data collection cycles</li>
    </ul>
</div>

<h2>Sensor Not Reading</h2>

<div class="feature-box">
    <h4>Temperature/Humidity Sensor Issues</h4>

    <p><strong>Check Physical Connection:</strong></p>
    <ul>
        <li>Verify sensor is plugged in</li>
        <li>Check USB connection is secure</li>
        <li>For GPIO sensors, verify wiring and pin numbers</li>
        <li>Try different USB port if using USB sensor</li>
    </ul>

    <p><strong>Verify Configuration:</strong></p>
    <ul>
        <li>Sensor type selected correctly</li>
        <li>Device path is correct (e.g., /dev/hidraw1)</li>
        <li>GPIO pin number is correct</li>
        <li>Sensor is Enabled in settings</li>
    </ul>

    <p><strong>Test Commands:</strong></p>
    <ul>
        <li>For USB sensors: <code>lsusb</code> to see if detected</li>
        <li>For DHT sensors: Test with standalone script</li>
        <li>Check system logs for errors</li>
    </ul>
</div>

<div class="feature-box">
    <h4>Weight Scale Issues</h4>

    <p><strong>Scale Reads Zero or Incorrect:</strong></p>
    <ul>
        <li>Run calibration wizard</li>
        <li>Check load cell connections</li>
        <li>Verify HX711 wiring if using that amplifier</li>
        <li>Ensure platform is level</li>
        <li>Check for binding or friction on platform</li>
    </ul>

    <p><strong>Erratic Readings:</strong></p>
    <ul>
        <li>Check for wind interference</li>
        <li>Verify load cells are not damaged</li>
        <li>Check electrical connections</li>
        <li>Look for moisture in electronics</li>
        <li>Recalibrate scale</li>
    </ul>

    <p><strong>Scale Not Responding:</strong></p>
    <ul>
        <li>Verify power to scale/amplifier</li>
        <li>Check GPIO pin connections (for HX711)</li>
        <li>Test with minimal weight (trigger response)</li>
        <li>Replace HX711 if suspected faulty</li>
    </ul>
</div>

<h2>Weather Data Not Showing</h2>

<div class="feature-box">
    <h4>WX Underground Issues</h4>
    <ul>
        <li>Verify Station ID is correct</li>
        <li>Check station is currently reporting on wunderground.com</li>
        <li>Ensure internet connection is working</li>
        <li>Try different nearby station</li>
    </ul>

    <h4>Local Weather Station Issues</h4>
    <ul>
        <li>Verify IP address is correct</li>
        <li>Ping station from HiveControl system</li>
        <li>Check firewall settings</li>
        <li>Ensure station is powered on</li>
        <li>Verify station is on same network</li>
    </ul>

    <h4>Local Sensor Weather Issues</h4>
    <ul>
        <li>Check sensor connections (same as hive sensors)</li>
        <li>Verify sensor is placed outdoors</li>
        <li>Check device path or GPIO configuration</li>
        <li>Test with "Read Sensors" button</li>
    </ul>
</div>

<h2>Page Won't Load / Error Messages</h2>

<div class="feature-box">
    <h4>PHP Errors Visible on Page</h4>
    <ul>
        <li>Check PHP error logs</li>
        <li>Verify all required PHP extensions are installed</li>
        <li>Check file permissions</li>
        <li>Ensure database file is accessible</li>
    </ul>

    <h4>Database Connection Errors</h4>
    <ul>
        <li>Verify database file exists at /home/HiveControl/data/hive-data.db</li>
        <li>Check file permissions on database</li>
        <li>Ensure SQLite is installed</li>
        <li>Check disk space</li>
    </ul>

    <h4>404 Not Found Errors</h4>
    <ul>
        <li>Verify web server is running</li>
        <li>Check document root is correct</li>
        <li>Ensure all files are in correct locations</li>
        <li>Check .htaccess files if using Apache</li>
    </ul>
</div>

<h2>Setup Wizard Keeps Appearing</h2>

<div class="feature-box">
    <h4>Symptoms</h4>
    <p>Setup wizard appears every time you visit dashboard</p>

    <h4>Causes & Solutions</h4>
    <ul>
        <li><strong>Required fields not saved:</strong> Complete all mandatory fields in wizard</li>
        <li><strong>Hive Name is "NOTSET":</strong> Change to actual hive name</li>
        <li><strong>API Key is blank:</strong> Enter valid API key</li>
        <li><strong>Database not writable:</strong> Check permissions on database file</li>
        <li><strong>Form not submitting:</strong> Check for JavaScript errors in browser console</li>
    </ul>
</div>

<h2>Charts Not Displaying</h2>

<div class="feature-box">
    <h4>Blank Chart Area</h4>

    <p><strong>JavaScript Issues:</strong></p>
    <ul>
        <li>Check browser console for JavaScript errors</li>
        <li>Ensure Highcharts library is loading</li>
        <li>Verify jQuery is loading</li>
        <li>Try different browser</li>
        <li>Clear browser cache</li>
    </ul>

    <p><strong>No Data to Plot:</strong></p>
    <ul>
        <li>Database may be empty</li>
        <li>Check that sensors are collecting data</li>
        <li>Try different time period</li>
        <li>Verify data collection script is running</li>
    </ul>
</div>

<h2>System Performance Issues</h2>

<div class="feature-box">
    <h4>Dashboard Loads Slowly</h4>
    <ul>
        <li>Large dataset taking time to query</li>
        <li>Use shorter time periods (Today, Week)</li>
        <li>Check system resources (CPU, memory, disk)</li>
        <li>Optimize database (vacuum/analyze)</li>
        <li>Clear old data if database is huge</li>
    </ul>

    <h4>Data Collection Delays</h4>
    <ul>
        <li>Check cron job scheduling</li>
        <li>Verify sensors respond quickly</li>
        <li>Look for network delays (weather data)</li>
        <li>Check system load</li>
    </ul>
</div>

<h2>Calibration Problems</h2>

<div class="feature-box">
    <h4>Scale Won't Calibrate</h4>
    <ul>
        <li>Ensure scale is stable before calibrating</li>
        <li>Remove all weight for zero reading</li>
        <li>Use accurate known weight for calibration</li>
        <li>Allow readings to stabilize between steps</li>
        <li>Check for mechanical issues (binding, friction)</li>
    </ul>

    <h4>Readings Still Wrong After Calibration</h4>
    <ul>
        <li>Verify calibration values were saved</li>
        <li>Try recalibrating from scratch</li>
        <li>Check for environmental issues (wind, temperature changes)</li>
        <li>Test with multiple known weights</li>
        <li>Consider hardware replacement if consistently wrong</li>
    </ul>
</div>

<h2>API / HiveTool.org Issues</h2>

<div class="feature-box">
    <h4>Data Not Uploading to HiveTool.org</h4>
    <ul>
        <li>Verify "Share Data" is enabled</li>
        <li>Check HiveTool.org username and password</li>
        <li>Ensure API key is valid</li>
        <li>Check internet connection</li>
        <li>Look for upload errors in system logs</li>
        <li>Verify HiveTool.org account is active</li>
    </ul>
</div>

<h2>Common Error Messages</h2>

<div class="code-block">
    <p><strong>"Trying to access array offset on false"</strong></p>
    <ul>
        <li>Database query returned no results</li>
        <li>Usually means no data collected yet</li>
        <li>Wait for data collection to run</li>
        <li>Verify sensors are configured and enabled</li>
    </ul>
</div>

<div class="code-block">
    <p><strong>"Database connection error"</strong></p>
    <ul>
        <li>Database file not accessible</li>
        <li>Check file path in db-connect.php</li>
        <li>Verify file permissions</li>
        <li>Ensure SQLite extension is installed</li>
    </ul>
</div>

<div class="code-block">
    <p><strong>"Permission denied"</strong></p>
    <ul>
        <li>File or directory permissions issue</li>
        <li>Check ownership of files</li>
        <li>Ensure web server has read/write access</li>
        <li>For device files, check udev rules</li>
    </ul>
</div>

<h2>Getting Additional Help</h2>

<div class="alert alert-info">
    <p><strong>If problems persist:</strong></p>
    <ul>
        <li><strong>Check System Logs:</strong> Look for detailed error messages</li>
        <li><strong>Community Forum:</strong> Visit <a href="https://www.hivecontrol.org/" target="_blank">HiveControl.org</a></li>
        <li><strong>GitHub Issues:</strong> Report bugs on GitHub repository</li>
        <li><strong>Documentation:</strong> Review full docs at HiveTool.org</li>
    </ul>
</div>

<h2>Diagnostic Checklist</h2>

<p>Before asking for help, gather this information:</p>

<ul>
    <li>HiveControl version</li>
    <li>Operating system and version</li>
    <li>Hardware type (Raspberry Pi model, etc.)</li>
    <li>Sensor types being used</li>
    <li>Error messages (exact text)</li>
    <li>What you've already tried</li>
    <li>Screenshots of issue</li>
    <li>Relevant log file entries</li>
</ul>

<h2>Prevention Tips</h2>

<ul>
    <li><strong>Regular Backups:</strong> Backup database regularly</li>
    <li><strong>Keep Updated:</strong> Update to latest HiveControl version</li>
    <li><strong>Monitor Logs:</strong> Periodically check for errors</li>
    <li><strong>Test Changes:</strong> Test in off-season when possible</li>
    <li><strong>Document Config:</strong> Keep notes on your setup</li>
    <li><strong>Verify Sensors:</strong> Periodically test sensor accuracy</li>
</ul>
