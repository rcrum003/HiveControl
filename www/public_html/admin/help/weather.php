<h1><i class="fa fa-cloud"></i> Weather Sources Guide</h1>

<p>HiveControl can pull weather data from multiple sources. Choose the option that best fits your setup.</p>

<h2>Weather Source Options</h2>

<div class="feature-box">
    <h4>1. WX Underground</h4>
    <p><strong>Best for:</strong> Most users without local weather stations</p>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Visit <a href="https://www.wunderground.com/" target="_blank">wunderground.com</a></li>
        <li>Search for your location</li>
        <li>Browse "Personal Weather Stations" near you</li>
        <li>Select station closest to your hive</li>
        <li>Copy the Station ID (e.g., "KORPORTL123")</li>
        <li>Enter Station ID in HiveControl settings</li>
    </ol>

    <p><strong>Pros:</strong></p>
    <ul>
        <li>Free to use</li>
        <li>Wide coverage of personal weather stations</li>
        <li>Reliable data</li>
        <li>No hardware needed</li>
    </ul>

    <p><strong>Cons:</strong></p>
    <ul>
        <li>Dependent on nearby station</li>
        <li>May not reflect micro-climate at hive</li>
        <li>Requires internet connection</li>
    </ul>
</div>

<div class="feature-box">
    <h4>2. AmbientWeather.net</h4>
    <p><strong>Best for:</strong> Users with Ambient Weather stations</p>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Create account at AmbientWeather.net</li>
        <li>Register your station</li>
        <li>Go to Account Settings → API Keys</li>
        <li>Generate API key</li>
        <li>Note your Station MAC address</li>
        <li>Enter both API Key and Station ID in HiveControl</li>
    </ol>

    <p><strong>Pros:</strong></p>
    <ul>
        <li>Your own weather station</li>
        <li>Exact location data</li>
        <li>Micro-climate specific</li>
        <li>Additional features via Ambient website</li>
    </ul>

    <p><strong>Cons:</strong></p>
    <ul>
        <li>Requires purchasing Ambient Weather station</li>
        <li>Station must be internet-connected</li>
    </ul>
</div>

<div class="feature-box">
    <h4>3. Local Weather Station</h4>
    <p><strong>Best for:</strong> Users with compatible local stations</p>

    <p><strong>Supported Stations:</strong></p>
    <ul>
        <li><strong>WS1400ip:</strong> Ambient Weather WS-1400-IP station</li>
        <li><strong>OurWeather:</strong> Compatible weather stations with local API</li>
    </ul>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Ensure weather station is on same network</li>
        <li>Find station's local IP address</li>
        <li>Select station type in HiveControl</li>
        <li>Enter local IP address</li>
    </ol>

    <p><strong>Pros:</strong></p>
    <ul>
        <li>No internet dependency for weather data</li>
        <li>Fast, local access</li>
        <li>Exact location data</li>
    </ul>

    <p><strong>Cons:</strong></p>
    <ul>
        <li>Requires compatible hardware</li>
        <li>Requires network configuration</li>
    </ul>
</div>

<div class="feature-box">
    <h4>4. Local Hive Sensors</h4>
    <p><strong>Best for:</strong> Users wanting simplest setup with existing sensors</p>

    <p><strong>How it Works:</strong></p>
    <ul>
        <li>Uses temperature/humidity sensor for weather data</li>
        <li>Mount sensor outside hive for ambient readings</li>
        <li>Configure just like hive temp sensor</li>
    </ul>

    <p><strong>Supported Sensors:</strong></p>
    <ul>
        <li>TemperHum</li>
        <li>DHT21</li>
        <li>DHT22</li>
        <li>SHT31-D</li>
        <li>BME280</li>
        <li>BME680</li>
    </ul>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Select "Local Hive Sensors" as weather source</li>
        <li>Choose your sensor type</li>
        <li>Configure device path or GPIO pin</li>
        <li>Mount sensor in shaded outdoor location</li>
    </ol>

    <p><strong>Pros:</strong></p>
    <ul>
        <li>Most accurate for your hive location</li>
        <li>No internet needed</li>
        <li>Uses sensors you may already have</li>
    </ul>

    <p><strong>Cons:</strong></p>
    <ul>
        <li>Only provides temp/humidity (no precipitation, wind, etc.)</li>
        <li>Requires additional sensor hardware</li>
        <li>Sensor placement critical for accuracy</li>
    </ul>
</div>

<div class="feature-box">
    <h4>5. WF Tempest UDP</h4>
    <p><strong>Best for:</strong> WeatherFlow Tempest station owners</p>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Ensure Tempest station is on same local network</li>
        <li>Find station serial number (on device or app)</li>
        <li>Enter serial number in HiveControl</li>
    </ol>

    <p><strong>Pros:</strong></p>
    <ul>
        <li>Local UDP connection (no internet required)</li>
        <li>Comprehensive weather data</li>
        <li>High-quality station</li>
    </ul>

    <p><strong>Cons:</strong></p>
    <ul>
        <li>Requires Tempest hardware purchase</li>
        <li>Must be on same network</li>
    </ul>
</div>

<h2>Choosing the Right Source</h2>

<div class="code-block">
    <p><strong>Decision Tree:</strong></p>
    <ul>
        <li><strong>Already have Ambient Weather?</strong> → Use AmbientWeather.net</li>
        <li><strong>Already have Tempest?</strong> → Use WF Tempest UDP</li>
        <li><strong>Have local compatible station?</strong> → Use Local Weather Station</li>
        <li><strong>Have spare temp sensor?</strong> → Use Local Hive Sensors</li>
        <li><strong>None of the above?</strong> → Use WX Underground</li>
    </ul>
</div>

<h2>Weather Data Usage</h2>

<div class="feature-box">
    <h4>What HiveControl Does with Weather Data</h4>
    <ul>
        <li><strong>Displays on Dashboard:</strong> Current ambient temperature and humidity</li>
        <li><strong>Charts:</strong> Plots weather alongside hive data for correlation</li>
        <li><strong>GDD Calculations:</strong> Computes growing degree days for bloom predictions</li>
        <li><strong>Analysis:</strong> Compare hive behavior to weather patterns</li>
        <li><strong>Alerts:</strong> (If configured) Notify on extreme weather</li>
    </ul>
</div>

<h2>Troubleshooting Weather Data</h2>

<div class="alert alert-warning">
    <strong>No Weather Data Showing?</strong>

    <p><strong>For WX Underground:</strong></p>
    <ul>
        <li>Verify Station ID is correct</li>
        <li>Check station is currently reporting (visit wunderground.com)</li>
        <li>Ensure internet connection is working</li>
    </ul>

    <p><strong>For AmbientWeather.net:</strong></p>
    <ul>
        <li>Verify API Key is correct</li>
        <li>Check Station MAC address</li>
        <li>Ensure station is uploading to AmbientWeather.net</li>
    </ul>

    <p><strong>For Local Sources:</strong></p>
    <ul>
        <li>Verify IP address is correct and reachable</li>
        <li>Ping the station from HiveControl system</li>
        <li>Check firewall settings</li>
        <li>For sensors: verify GPIO/device path and connections</li>
    </ul>
</div>

<h2>Best Practices</h2>

<ul>
    <li><strong>Choose Closest Station:</strong> For WX Underground, pick station nearest your hive</li>
    <li><strong>Shade Your Sensors:</strong> If using local sensors, keep out of direct sunlight</li>
    <li><strong>Verify Data:</strong> Periodically compare against other sources</li>
    <li><strong>Backup Option:</strong> Consider having fallback weather source configured</li>
    <li><strong>Document Setup:</strong> Note which station/sensor you're using for future reference</li>
</ul>

<h2>Advanced: Multiple Weather Sources</h2>

<div class="alert alert-info">
    <strong>Pro Tip:</strong> You can compare different weather sources by temporarily switching between them to see which provides most accurate data for your location.
</div>
