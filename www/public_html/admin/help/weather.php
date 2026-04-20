<h1><i class="fa fa-cloud"></i> Weather Sources Guide</h1>

<p>HiveControl can pull weather data from multiple sources. Choose the option that best fits your setup.</p>

<h2>Recommended: Cloud APIs (No Hardware Needed)</h2>

<div class="feature-box">
    <h4>1. Open-Meteo <span class="label label-success">Recommended</span></h4>
    <p><strong>Best for:</strong> Most users &mdash; free, no API key, global coverage</p>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Set your latitude/longitude in <a href="hiveconfig.php">Hive Config</a></li>
        <li>Select "Open-Meteo" as weather source</li>
        <li>That's it &mdash; no API key or account needed</li>
    </ol>

    <p><strong>Data Provided:</strong> Temperature, humidity, wind, pressure, solar radiation, UV, precipitation, dew point</p>

    <p><strong>Pros:</strong></p>
    <ul>
        <li>Completely free with no rate limits</li>
        <li>No account or API key required</li>
        <li>Global coverage (works anywhere in the world)</li>
        <li>Full data set including solar radiation and UV</li>
        <li>Backed by ECMWF, DWD, and NOAA weather models</li>
    </ul>

    <p><strong>Cons:</strong></p>
    <ul>
        <li>Model-based data (not a local station measurement)</li>
        <li>Requires internet connection</li>
    </ul>
</div>

<div class="feature-box">
    <h4>2. NWS weather.gov <span class="label label-info">US Only</span></h4>
    <p><strong>Best for:</strong> US-based users wanting government weather station data</p>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Set your latitude/longitude in <a href="hiveconfig.php">Hive Config</a></li>
        <li>Select "NWS weather.gov" as weather source</li>
        <li>Station is auto-discovered from your location</li>
    </ol>

    <p><strong>Data Provided:</strong> Temperature, humidity, wind, pressure, dew point, precipitation</p>

    <p><strong>Pros:</strong></p>
    <ul>
        <li>Free, no API key required</li>
        <li>Official NOAA/NWS observation stations</li>
        <li>Reliable government infrastructure</li>
    </ul>

    <p><strong>Cons:</strong></p>
    <ul>
        <li>US locations only</li>
        <li>No solar radiation or UV data</li>
        <li>Station may be some distance from your hive</li>
    </ul>
</div>

<h2>Cloud APIs (Key Required)</h2>

<div class="feature-box">
    <h4>3. OpenWeatherMap</h4>
    <p><strong>Best for:</strong> Users who want a well-known API with generous free tier</p>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Create account at <a href="https://openweathermap.org/api" target="_blank">openweathermap.org</a></li>
        <li>Generate a free API key</li>
        <li>Enter API key in HiveControl</li>
    </ol>

    <p><strong>Free tier:</strong> 1,000 calls/day</p>
    <p><strong>Data Provided:</strong> Temperature, humidity, wind, pressure, dew point, precipitation</p>
</div>

<div class="feature-box">
    <h4>4. WeatherAPI.com</h4>
    <p><strong>Best for:</strong> Users who need high call volume</p>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Create account at <a href="https://www.weatherapi.com/signup.aspx" target="_blank">weatherapi.com</a></li>
        <li>Generate a free API key</li>
        <li>Enter API key in HiveControl</li>
    </ol>

    <p><strong>Free tier:</strong> 1,000,000 calls/month</p>
    <p><strong>Data Provided:</strong> Temperature, humidity, wind, pressure, UV, dew point, precipitation</p>
</div>

<div class="feature-box">
    <h4>5. Visual Crossing</h4>
    <p><strong>Best for:</strong> Users who need solar radiation data with an API key source</p>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Create account at <a href="https://www.visualcrossing.com/sign-up" target="_blank">visualcrossing.com</a></li>
        <li>Generate a free API key</li>
        <li>Enter API key in HiveControl</li>
    </ol>

    <p><strong>Free tier:</strong> 1,000 calls/day</p>
    <p><strong>Data Provided:</strong> Temperature, humidity, wind, pressure, solar radiation, UV, dew point, precipitation</p>
</div>

<h2>Personal Weather Stations</h2>

<div class="feature-box">
    <h4>6. AmbientWeather.net</h4>
    <p><strong>Best for:</strong> Users with Ambient Weather stations</p>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Create account at AmbientWeather.net</li>
        <li>Register your station</li>
        <li>Go to Account Settings &rarr; API Keys</li>
        <li>Generate API key</li>
        <li>Note your Station MAC address</li>
        <li>Enter both API Key and Station MAC in HiveControl</li>
    </ol>

    <p><strong>Data Provided:</strong> Temperature, humidity, wind, pressure, solar radiation, UV, dew point, precipitation</p>

    <p><strong>Pros:</strong></p>
    <ul>
        <li>Your own weather station &mdash; exact location data</li>
        <li>Full data set including solar and UV</li>
    </ul>

    <p><strong>Cons:</strong></p>
    <ul>
        <li>Requires purchasing Ambient Weather station</li>
        <li>Station must be internet-connected</li>
    </ul>
</div>

<div class="feature-box">
    <h4>7. WX Underground</h4>
    <p><strong>Best for:</strong> Users near a WX Underground personal weather station</p>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Visit <a href="https://www.wunderground.com/" target="_blank">wunderground.com</a></li>
        <li>Search for your location</li>
        <li>Browse "Personal Weather Stations" near you</li>
        <li>Copy the Station ID (e.g., "KORPORTL123")</li>
        <li>Enter Station ID in HiveControl</li>
    </ol>

    <div class="alert alert-warning">
        <strong>Note:</strong> WX Underground's free API has been unreliable since IBM's acquisition. Consider using Open-Meteo or NWS as a more reliable alternative.
    </div>
</div>

<div class="feature-box">
    <h4>8. WF Tempest UDP</h4>
    <p><strong>Best for:</strong> WeatherFlow Tempest station owners</p>

    <p><strong>Setup:</strong></p>
    <ol>
        <li>Ensure Tempest station is on same local network</li>
        <li>Find station serial number (on device or app)</li>
        <li>Enter serial number in HiveControl</li>
    </ol>

    <p><strong>Data Provided:</strong> Temperature, humidity, wind, pressure, solar radiation, UV, lightning, precipitation</p>

    <p><strong>Pros:</strong></p>
    <ul>
        <li>Local UDP connection (no internet required for weather data)</li>
        <li>Comprehensive data including lightning detection</li>
    </ul>

    <p><strong>Cons:</strong></p>
    <ul>
        <li>Requires Tempest hardware purchase</li>
        <li>Must be on same network as HiveControl</li>
    </ul>
</div>

<h2>Local Hardware</h2>

<div class="feature-box">
    <h4>9. Local Weather Station</h4>
    <p><strong>Best for:</strong> Users with compatible local stations on the same network</p>

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
</div>

<div class="feature-box">
    <h4>10. Local Hive Sensors</h4>
    <p><strong>Best for:</strong> Users wanting simplest setup with existing sensors</p>

    <p><strong>How it Works:</strong></p>
    <ul>
        <li>Uses temperature/humidity sensor for weather data</li>
        <li>Mount sensor outside hive for ambient readings</li>
    </ul>

    <p><strong>Supported Sensors:</strong></p>
    <ul>
        <li>TemperHum</li>
        <li>DHT21 / DHT22</li>
        <li>SHT31-D</li>
        <li>BME280 / BME680</li>
    </ul>

    <p><strong>Pros:</strong></p>
    <ul>
        <li>Most accurate for your hive micro-climate</li>
        <li>No internet needed</li>
    </ul>

    <p><strong>Cons:</strong></p>
    <ul>
        <li>Only provides temp/humidity (no precipitation, wind, etc.)</li>
        <li>Requires additional sensor hardware</li>
    </ul>
</div>

<h2>Choosing the Right Source</h2>

<div class="code-block">
    <p><strong>Decision Tree:</strong></p>
    <ul>
        <li><strong>No hardware, worldwide?</strong> &rarr; <strong>Open-Meteo</strong> (recommended default)</li>
        <li><strong>No hardware, US only?</strong> &rarr; NWS weather.gov</li>
        <li><strong>Have Ambient Weather station?</strong> &rarr; AmbientWeather.net</li>
        <li><strong>Have WeatherFlow Tempest?</strong> &rarr; WF Tempest UDP</li>
        <li><strong>Have local compatible station?</strong> &rarr; Local Weather Station</li>
        <li><strong>Have spare temp sensor?</strong> &rarr; Local Hive Sensors</li>
        <li><strong>Need solar/UV with API key?</strong> &rarr; Visual Crossing</li>
    </ul>
</div>

<h2>Weather Fallback</h2>

<div class="feature-box">
    <h4>Automatic Failover</h4>
    <p>HiveControl supports a <strong>fallback weather source</strong> that activates automatically when your primary source fails. This prevents gaps in weather data during API outages.</p>

    <p><strong>How it Works:</strong></p>
    <ul>
        <li>If the primary weather source returns no data, HiveControl tries the fallback</li>
        <li>Fallback events are logged so you can see when they occur</li>
        <li>Only cloud APIs are available as fallback sources</li>
    </ul>

    <p><strong>Recommended Fallback Configurations:</strong></p>
    <ul>
        <li><strong>US users:</strong> Primary: Open-Meteo, Fallback: NWS</li>
        <li><strong>International users:</strong> Primary: Open-Meteo, Fallback: OpenWeatherMap or WeatherAPI</li>
        <li><strong>Local station users:</strong> Primary: Your station, Fallback: Open-Meteo</li>
    </ul>

    <p>Configure in <strong>Instrument Configuration</strong> under the Weather Fallback dropdown.</p>
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

    <p><strong>For all cloud APIs (Open-Meteo, NWS, OpenWeatherMap, WeatherAPI, Visual Crossing):</strong></p>
    <ul>
        <li>Ensure internet connection is working: <code>ping google.com</code></li>
        <li>Verify latitude/longitude are set correctly in <a href="hiveconfig.php">Hive Config</a></li>
        <li>For key-based APIs: verify your API key is correct and active</li>
        <li>Check logs: <code>cat /home/HiveControl/logs/currconditions.log</code></li>
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

    <p><strong>Still not working?</strong></p>
    <ul>
        <li>Run the weather script manually: <code>sudo /home/HiveControl/scripts/weather/getwx.sh</code></li>
        <li>Check for errors in the output</li>
        <li>Configure a Weather Fallback source for automatic failover</li>
    </ul>
</div>

<h2>Best Practices</h2>

<ul>
    <li><strong>Set a Fallback Source:</strong> Configure a backup weather API for reliability</li>
    <li><strong>Use Free Sources:</strong> Open-Meteo and NWS are free with no rate limits</li>
    <li><strong>Shade Your Sensors:</strong> If using local sensors, keep out of direct sunlight</li>
    <li><strong>Verify Data:</strong> Periodically compare against other sources</li>
    <li><strong>Check Logs:</strong> Review weather logs if data looks incorrect</li>
</ul>
