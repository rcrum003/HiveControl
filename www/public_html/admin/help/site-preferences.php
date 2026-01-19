<h1><i class="fa fa-sliders"></i> Site Preferences</h1>

<div class="alert alert-info">
    <strong>About Site Preferences:</strong> Customize how HiveControl displays information, handles units, and presents your dashboard to match your beekeeping style and preferences.
</div>

<h2>Available Preferences</h2>

<div class="row">
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-arrows-h"></i> Site Orientation</h4>
            <p>Choose how the navigation menu displays:</p>
            <ul>
                <li><strong>Normal:</strong> Left sidebar navigation (default)</li>
                <li><strong>Wide:</strong> Top horizontal navigation bar</li>
            </ul>
            <p><em>Tip: Wide orientation is better for wide monitors or tablets in landscape mode.</em></p>
        </div>
    </div>
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-compress"></i> Site Type</h4>
            <p>Control dashboard density:</p>
            <ul>
                <li><strong>Normal:</strong> Full dashboard with all widgets</li>
                <li><strong>Compact:</strong> Streamlined view with essential data only</li>
            </ul>
            <p><em>Compact mode is ideal for smaller screens or quick checks.</em></p>
        </div>
    </div>
</div>

<h2>Unit Preferences</h2>

<div class="feature-box">
    <h4><i class="fa fa-balance-scale"></i> Measurement Units</h4>
    <p>Choose between Imperial and Metric units:</p>

    <table class="table table-bordered">
        <thead>
            <tr>
                <th>Measurement</th>
                <th>Imperial</th>
                <th>Metric</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td>Temperature</td>
                <td>Fahrenheit (°F)</td>
                <td>Celsius (°C)</td>
            </tr>
            <tr>
                <td>Weight</td>
                <td>Pounds (lb)</td>
                <td>Kilograms (kg)</td>
            </tr>
            <tr>
                <td>Distance</td>
                <td>Miles</td>
                <td>Kilometers</td>
            </tr>
        </tbody>
    </table>

    <div class="alert alert-warning">
        <strong>Note:</strong> Changing units affects all displays but doesn't change your stored data. Historical data will be converted for display automatically.
    </div>
</div>

<h2>Display Preferences</h2>

<h3><i class="fa fa-area-chart"></i> Chart Settings</h3>
<ul>
    <li><strong>Chart Rounding:</strong> Set decimal precision for data points</li>
    <li><strong>Data Smoothing:</strong> Apply averaging to reduce noise in charts</li>
    <li><strong>Color Schemes:</strong> Customize chart colors to match your preference</li>
    <li><strong>Default Time Period:</strong> Set default view (day, week, month, year)</li>
</ul>

<h3><i class="fa fa-clock-o"></i> Time & Date</h3>
<ul>
    <li><strong>Time Zone:</strong> Automatically detected from your location settings</li>
    <li><strong>Date Format:</strong> Choose your preferred date display format</li>
    <li><strong>Time Format:</strong> 12-hour or 24-hour clock</li>
</ul>

<h2>Configuring Site Preferences</h2>

<div class="panel panel-default">
    <div class="panel-heading">
        <strong>How to Change Preferences</strong>
    </div>
    <div class="panel-body">
        <p><span class="step-number">1</span> Navigate to <strong>Settings > Site Preferences</strong></p>
        <p><span class="step-number">2</span> Review the current settings</p>
        <p><span class="step-number">3</span> Adjust any preferences you want to change</p>
        <p><span class="step-number">4</span> Click <strong>Save Changes</strong> at the bottom</p>
        <p><span class="step-number">5</span> Refresh your browser to see changes take effect</p>
    </div>
</div>

<h2>Dashboard Customization</h2>

<h3>Widget Visibility</h3>
<p>Control which data widgets appear on your dashboard:</p>
<ul>
    <li>Temperature gauges</li>
    <li>Weight trends</li>
    <li>Humidity displays</li>
    <li>Light intensity</li>
    <li>Growing Degree Days (GDD)</li>
    <li>Bee flight activity</li>
    <li>Air quality metrics</li>
</ul>

<div class="alert alert-info">
    <strong>Pro Tip:</strong> Hide widgets for sensors you don't have installed to keep your dashboard clean and focused.
</div>

<h2>Data Refresh Settings</h2>

<div class="feature-box">
    <h4><i class="fa fa-refresh"></i> Auto-Refresh</h4>
    <p>The dashboard automatically refreshes to show current data:</p>
    <ul>
        <li><strong>Default:</strong> Every 5 minutes</li>
        <li><strong>Configurable:</strong> Set refresh interval from 1-60 minutes</li>
        <li><strong>Manual Refresh:</strong> Click the refresh button anytime</li>
    </ul>
</div>

<h2>Notification Preferences</h2>

<h3>Alert Settings</h3>
<ul>
    <li><strong>Email Notifications:</strong> Receive alerts via email</li>
    <li><strong>Alert Thresholds:</strong> Set custom thresholds for temperature, weight changes, etc.</li>
    <li><strong>Quiet Hours:</strong> Disable notifications during specific times</li>
    <li><strong>Alert Priority:</strong> Choose which events trigger notifications</li>
</ul>

<h2>Best Practices</h2>

<div class="row">
    <div class="col-md-6">
        <h3>For Desktop Users</h3>
        <ul>
            <li>Use "Normal" or "Wide" orientation based on screen size</li>
            <li>Enable all widgets you find useful</li>
            <li>Set refresh to 5-10 minutes</li>
            <li>Use detailed charts with more data points</li>
        </ul>
    </div>
    <div class="col-md-6">
        <h3>For Mobile Users</h3>
        <ul>
            <li>Use "Compact" site type</li>
            <li>Hide non-essential widgets</li>
            <li>Use simpler charts with less detail</li>
            <li>Consider longer refresh intervals to save battery</li>
        </ul>
    </div>
</div>

<h2>Common Questions</h2>

<h3>Will changing preferences affect my historical data?</h3>
<p>No, preferences only affect how data is displayed. Your raw sensor data remains unchanged in the database.</p>

<h3>Can I save different preference profiles?</h3>
<p>Currently, HiveControl supports one preference set per installation. However, different browsers or devices will maintain their own view states.</p>

<h3>How do I reset preferences to defaults?</h3>
<p>Navigate to Site Preferences and click the "Reset to Defaults" button at the bottom of the page.</p>

<h3>Why don't my changes take effect immediately?</h3>
<p>Some preference changes require a browser refresh (F5 or Ctrl+R) to take effect. If changes still don't appear, clear your browser cache.</p>
