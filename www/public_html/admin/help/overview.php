<h1><i class="fa fa-home"></i> HiveControl Overview</h1>

<div class="alert alert-info">
    <strong>Welcome to HiveControl!</strong> This documentation will help you understand and configure your hive monitoring system.
</div>

<h2>What is HiveControl?</h2>
<p>HiveControl is a comprehensive hive monitoring system that tracks vital metrics about your beehive including:</p>

<div class="row">
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-thermometer-half"></i> Temperature Monitoring</h4>
            <p>Track internal hive temperature and ambient weather conditions to understand your colony's environment.</p>
        </div>
    </div>
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-balance-scale"></i> Weight Tracking</h4>
            <p>Monitor hive weight changes to track honey production, nectar flow, and colony activity.</p>
        </div>
    </div>
</div>

<div class="row">
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-tint"></i> Humidity Levels</h4>
            <p>Keep track of moisture levels inside and outside the hive for optimal colony health.</p>
        </div>
    </div>
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-sun-o"></i> Light & Solar</h4>
            <p>Measure light intensity and solar radiation to understand foraging conditions.</p>
        </div>
    </div>
</div>

<h2>System Components</h2>

<h3>Dashboard</h3>
<p>The main dashboard provides real-time and historical views of all your hive metrics with interactive charts and current condition panels.</p>

<h3>Configuration Pages</h3>
<ul>
    <li><strong>Basic Settings:</strong> Configure hive name, location, API keys, and general parameters</li>
    <li><strong>Instruments:</strong> Set up and calibrate sensors for temperature, weight, humidity, and light</li>
    <li><strong>Weather Sources:</strong> Choose between WX Underground, local sensors, or other weather data sources</li>
</ul>

<h2>How Data is Collected</h2>
<p>HiveControl continuously collects data from your configured sensors and stores it in a local database. The system:</p>

<ol>
    <li>Reads sensor data at regular intervals</li>
    <li>Applies calibration values for accuracy</li>
    <li>Stores readings in the database</li>
    <li>Generates charts and displays on the dashboard</li>
    <li>Optionally shares data with HiveTool.org for research</li>
</ol>

<h2>Getting Started</h2>
<div class="alert alert-warning">
    <strong>First Time Setup:</strong> If this is your first time using HiveControl, start with the <a href="?topic=initial-setup">Initial Setup Guide</a>.
</div>

<h2>Quick Links</h2>
<div class="row">
    <div class="col-md-4">
        <a href="?topic=initial-setup" class="btn btn-primary btn-block">
            <i class="fa fa-rocket"></i> Initial Setup
        </a>
    </div>
    <div class="col-md-4">
        <a href="?topic=instruments" class="btn btn-primary btn-block">
            <i class="fa fa-tachometer"></i> Configure Sensors
        </a>
    </div>
    <div class="col-md-4">
        <a href="?topic=troubleshooting" class="btn btn-primary btn-block">
            <i class="fa fa-wrench"></i> Troubleshooting
        </a>
    </div>
</div>

<h2>Support & Resources</h2>
<ul>
    <li><strong>Community Forum:</strong> Visit <a href="https://www.hivecontrol.org/" target="_blank">HiveControl.org</a> for community support</li>
    <li><strong>GitHub Issues:</strong> Report bugs and request features on GitHub</li>
    <li><strong>Documentation:</strong> Complete documentation available at HiveTool.org</li>
</ul>
