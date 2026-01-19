<h1><i class="fa fa-rocket"></i> Initial Setup Guide</h1>

<div class="alert alert-info">
    <strong>New Installation?</strong> Follow this step-by-step guide to get your HiveControl system up and running.
</div>

<h2>Setup Wizard</h2>
<p>On first installation, HiveControl will automatically guide you through a setup wizard that collects essential information:</p>

<div class="feature-box">
    <h4><span class="step-number">1</span> Hive Name</h4>
    <p>Choose a unique, identifiable name for your hive (e.g., "Hive-01", "Backyard-Hive", "TopBar-Main").</p>
    <p><strong>Why it matters:</strong> This name appears on charts, reports, and helps you identify this hive if you manage multiple locations.</p>
</div>

<div class="feature-box">
    <h4><span class="step-number">2</span> Hive API Key</h4>
    <p>Register at <a href="https://www.hivecontrol.org/" target="_blank">HiveControl.org</a> to get your API key:</p>
    <ol>
        <li>Create an account on HiveControl.org</li>
        <li>Click on your profile/beekeeper icon</li>
        <li>Navigate to "Your Settings" → "API"</li>
        <li>Enter a name for your key and click "Create"</li>
        <li>Copy the generated key and paste it into the setup form</li>
    </ol>
    <p><strong>Note:</strong> You can use one API key for all your hives.</p>
</div>

<div class="feature-box">
    <h4><span class="step-number">3</span> Location (City & State)</h4>
    <p>Enter the city and state/province where your hive is located.</p>
    <p><strong>Why it matters:</strong> This information is used to:</p>
    <ul>
        <li>Pull accurate weather data for your location</li>
        <li>Display your hive on community maps</li>
        <li>Connect with other beekeepers in your area</li>
        <li>Contribute to regional research data</li>
    </ul>
</div>

<h2>After Setup Wizard</h2>
<p>Once you complete the setup wizard, you'll be ready to configure additional settings:</p>

<h3>Recommended Next Steps</h3>

<div class="code-block">
    <p><strong>1. Configure Instruments</strong></p>
    <p>Go to <a href="/admin/instrumentconfig.php">Settings → Instruments</a> to set up your sensors:</p>
    <ul>
        <li>Temperature/Humidity sensors</li>
        <li>Weight scale</li>
        <li>Light sensors</li>
        <li>Camera (if installed)</li>
    </ul>
</div>

<div class="code-block">
    <p><strong>2. Set Weather Source</strong></p>
    <p>Choose your weather data source in the Instruments page:</p>
    <ul>
        <li>WX Underground (requires station ID)</li>
        <li>AmbientWeather.net</li>
        <li>Local weather station</li>
        <li>Local sensors</li>
    </ul>
</div>

<div class="code-block">
    <p><strong>3. Additional Basic Settings</strong></p>
    <p>Visit <a href="/admin/hiveconfig.php">Settings → Basic</a> to configure:</p>
    <ul>
        <li>Timezone</li>
        <li>Growing Degree Days (GDD) settings</li>
        <li>Power source (AC/Solar)</li>
        <li>Internet connection type</li>
        <li>HiveTool.org data sharing</li>
    </ul>
</div>

<h2>Testing Your Setup</h2>

<p>After configuring your instruments, verify everything is working:</p>

<ol>
    <li><strong>Visit the Dashboard:</strong> Go to <a href="/pages/index.php">Dashboard</a></li>
    <li><strong>Check Current Readings:</strong> All panels should show data (may be zero initially)</li>
    <li><strong>Read Live Sensors:</strong> In Instruments settings, click "Read Sensors" to see live values</li>
    <li><strong>Wait for Data Collection:</strong> Allow the system to collect data for a few hours</li>
    <li><strong>View Charts:</strong> Charts will populate as data is collected over time</li>
</ol>

<h2>Common First-Time Issues</h2>

<div class="alert alert-warning">
    <strong>No Data Showing?</strong>
    <ul>
        <li>Ensure all required fields in Basic Settings are filled</li>
        <li>Check that instruments are properly configured and enabled</li>
        <li>Verify sensors are physically connected</li>
        <li>Allow time for the system to collect initial readings</li>
    </ul>
</div>

<h2>Need Help?</h2>
<p>If you encounter issues during setup:</p>
<ul>
    <li>Check the <a href="?topic=troubleshooting">Troubleshooting Guide</a></li>
    <li>Visit the <a href="?topic=instruments">Instruments Guide</a> for sensor-specific help</li>
    <li>Ask the community at <a href="https://www.hivecontrol.org/" target="_blank">HiveControl.org</a></li>
</ul>
