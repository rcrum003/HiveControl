<h1><i class="fa fa-cog"></i> Basic Settings Guide</h1>

<p>Access Basic Settings from the admin menu: <strong>Settings → Basic</strong></p>

<h2>Required Settings</h2>

<div class="feature-box">
    <h4>Hive API Key</h4>
    <p><strong>Purpose:</strong> Authenticates your hive with HiveControl.org services</p>
    <p><strong>Format:</strong> Alphanumeric string</p>
    <p><strong>How to get it:</strong></p>
    <ol>
        <li>Register at <a href="https://www.hivecontrol.org/" target="_blank">HiveControl.org</a></li>
        <li>Go to "Your Settings" → "API"</li>
        <li>Create a new API key</li>
        <li>Copy and paste into this field</li>
    </ol>
    <p><strong>Note:</strong> One API key can be used for multiple hives</p>
</div>

<div class="feature-box">
    <h4>Hive Name</h4>
    <p><strong>Purpose:</strong> Identifies your hive in the system and on shared platforms</p>
    <p><strong>Examples:</strong> "Hive-01", "BackyardHive", "TopBar-Main"</p>
    <p><strong>Requirements:</strong> Letters, numbers, hyphens only (no spaces)</p>
</div>

<div class="feature-box">
    <h4>Hive ID</h4>
    <p><strong>Purpose:</strong> Unique system identifier</p>
    <p><strong>Note:</strong> Auto-generated, cannot be changed</p>
</div>

<h2>Location Settings</h2>

<div class="feature-box">
    <h4>City</h4>
    <p>Closest city to your hive location. Used for weather data and regional mapping.</p>
</div>

<div class="feature-box">
    <h4>State</h4>
    <p>State or province where your hive is located.</p>
</div>

<div class="feature-box">
    <h4>Postal Code / ZIP</h4>
    <p><strong>Purpose:</strong> Used for ambient pollen count data</p>
    <p><strong>Optional:</strong> Leave blank if you don't want to collect pollen data</p>
</div>

<div class="feature-box">
    <h4>Country</h4>
    <p>Country where your hive is located.</p>
</div>

<div class="feature-box">
    <h4>Latitude & Longitude</h4>
    <p><strong>Purpose:</strong> Precise location for solar calculations and mapping</p>
    <p><strong>Format:</strong> Decimal degrees (e.g., 45.5231, -122.6765)</p>
    <p><strong>How to find:</strong> Use Google Maps - right-click your location and select coordinates</p>
</div>

<h2>Timezone</h2>

<div class="feature-box">
    <h4>Time Zone Setting</h4>
    <p><strong>Critical Setting:</strong> Ensures accurate timestamps on all data</p>
    <p><strong>Select:</strong> Choose your local timezone from the dropdown</p>
    <p><strong>Impact:</strong> Affects:</p>
    <ul>
        <li>Data timestamps</li>
        <li>Chart time axes</li>
        <li>Growing Degree Day calculations</li>
        <li>Sunrise/sunset calculations</li>
    </ul>
</div>

<h2>Growing Degree Days (GDD)</h2>

<div class="feature-box">
    <h4>GDD Base Temp</h4>
    <p><strong>Recommended:</strong> 50°F (10°C)</p>
    <p><strong>Purpose:</strong> Base temperature for calculating accumulated heat units</p>
    <p><strong>Why it matters:</strong> GDD helps predict:</p>
    <ul>
        <li>Plant bloom times (nectar sources)</li>
        <li>Pest emergence</li>
        <li>Colony development rates</li>
    </ul>
</div>

<div class="feature-box">
    <h4>GDD Start Date</h4>
    <p><strong>Format:</strong> YYYYMMDD (e.g., 20250301 for March 1, 2025)</p>
    <p><strong>Recommended:</strong> March 1st (typical start of growing season)</p>
    <p><strong>Must be current year:</strong> Update annually</p>
    <p><strong>Note:</strong> If date is more than 1 year old, you'll see a warning</p>
</div>

<h2>System Information</h2>

<div class="feature-box">
    <h4>Power Source</h4>
    <p><strong>Options:</strong> AC or Solar</p>
    <p><strong>Purpose:</strong> Informational for HiveTool reporting</p>
</div>

<div class="feature-box">
    <h4>Internet Connection</h4>
    <p><strong>Options:</strong> Wi-Fi or Ethernet</p>
    <p><strong>Purpose:</strong> Informational for HiveTool reporting</p>
</div>

<div class="feature-box">
    <h4>Status</h4>
    <p><strong>Options:</strong> Testing, Online, Offline</p>
    <p><strong>Purpose:</strong> Indicates operational status for HiveTool.org</p>
    <ul>
        <li><strong>Testing:</strong> Initial setup/calibration phase</li>
        <li><strong>Online:</strong> Fully operational and collecting data</li>
        <li><strong>Offline:</strong> Temporarily not collecting data</li>
    </ul>
</div>

<div class="feature-box">
    <h4>Computer Type</h4>
    <p><strong>Options:</strong> Raspberry Pi, Arduino, Other x86</p>
    <p><strong>Purpose:</strong> Informational for HiveTool reporting</p>
</div>

<div class="feature-box">
    <h4>Start Date</h4>
    <p><strong>Purpose:</strong> Date when hive went "Online" for HiveTool.org</p>
    <p><strong>Use:</strong> Click field to select date from calendar</p>
</div>

<h2>Data Sharing</h2>

<div class="feature-box">
    <h4>Share Data with Hivetool.org</h4>
    <p><strong>Options:</strong> Yes or No</p>
    <p><strong>When enabled, you can configure:</strong></p>
    <ul>
        <li><strong>Username:</strong> Your HiveTool.org username</li>
        <li><strong>Password:</strong> Your HiveTool.org password</li>
        <li><strong>URL:</strong> HiveTool.org upload endpoint</li>
    </ul>
    <p><strong>Benefits of sharing:</strong></p>
    <ul>
        <li>Contribute to bee research</li>
        <li>Compare your hive to others regionally</li>
        <li>Access additional analysis tools</li>
        <li>Help scientists understand colony health patterns</li>
    </ul>
</div>

<h2>Saving Changes</h2>

<div class="alert alert-warning">
    <strong>Auto-Save:</strong> Settings auto-save when you change any field. Watch for the page reload confirming your changes were saved.
</div>

<h2>Tips</h2>

<ul>
    <li><strong>Keep API Key Secure:</strong> Don't share your API key publicly</li>
    <li><strong>Update GDD Start Date Annually:</strong> Set a calendar reminder for March 1st</li>
    <li><strong>Accurate Location Matters:</strong> Weather data accuracy depends on location settings</li>
    <li><strong>Test Before Going Online:</strong> Use "Testing" status until you're confident in your setup</li>
</ul>
