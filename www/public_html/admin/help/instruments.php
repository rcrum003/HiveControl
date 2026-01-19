<h1><i class="fa fa-tachometer"></i> Instruments & Sensors Guide</h1>

<p>Access Instrument Settings from the admin menu: <strong>Settings → Instruments</strong></p>

<div class="alert alert-info">
    <strong>Important:</strong> Each instrument must be enabled and configured before it will collect data.
</div>

<h2>Temperature & Humidity Sensors</h2>

<div class="feature-box">
    <h4>Enable/Disable</h4>
    <p>Use the dropdown to enable or disable temperature/humidity monitoring.</p>
</div>

<div class="feature-box">
    <h4>Supported Sensor Types</h4>
    <ul>
        <li><strong>TemperHum:</strong> USB temperature + humidity sensor</li>
        <li><strong>Temper:</strong> USB temperature-only sensor</li>
        <li><strong>DHT22:</strong> Digital humidity and temperature sensor (GPIO)</li>
        <li><strong>DHT21:</strong> Digital humidity and temperature sensor (GPIO)</li>
        <li><strong>SHT31-D:</strong> High-precision I2C sensor</li>
        <li><strong>BME280:</strong> Pressure, temperature, humidity I2C sensor</li>
        <li><strong>BME680:</strong> Air quality, pressure, temperature, humidity I2C sensor</li>
        <li><strong>BroodMinder T/TH:</strong> Bluetooth Low Energy sensors</li>
    </ul>
</div>

<div class="feature-box">
    <h4>Configuration Options</h4>
    <p><strong>For TemperHum/Temper:</strong></p>
    <ul>
        <li><strong>Device:</strong> Path to device (e.g., /dev/hidraw1)</li>
        <li>Use <code>tempered -e</code> from console to see available devices</li>
        <li><strong>Sub-Device:</strong> If sensor has multiple temperature probes</li>
    </ul>

    <p><strong>For DHT22/DHT21:</strong></p>
    <ul>
        <li><strong>GPIO:</strong> GPIO pin number where sensor is connected</li>
    </ul>

    <p><strong>For BroodMinder:</strong></p>
    <ul>
        <li><strong>Device:</strong> Full MAC address (lowercase, format: 06:09:42:1c:8a)</li>
        <li>Address is written on the device</li>
    </ul>
</div>

<h2>Weight Scale</h2>

<div class="feature-box">
    <h4>Supported Scale Types</h4>
    <ul>
        <li><strong>Phidget 1046:</strong> PhidgetBridge with load cells</li>
        <li><strong>HX711:</strong> Low-cost load cell amplifier (GPIO)</li>
        <li><strong>CPW 200 Plus:</strong> Industrial scale interface</li>
        <li><strong>None:</strong> Disable weight monitoring</li>
    </ul>
</div>

<div class="feature-box">
    <h4>Calibration</h4>
    <p><strong>HX711 users:</strong> Click "Calibrate Wizard" button to run calibration</p>
    <p>See <a href="?topic=calibration">Calibration Guide</a> for detailed instructions</p>
</div>

<h2>Light Sensors</h2>

<div class="feature-box">
    <h4>Supported Light Sensors</h4>
    <ul>
        <li><strong>TSL2591:</strong> High dynamic range light sensor</li>
        <li><strong>TSL2561:</strong> Digital light sensor</li>
        <li><strong>WX Station:</strong> Use light data from weather station</li>
    </ul>
</div>

<div class="feature-box">
    <h4>Why Monitor Light?</h4>
    <ul>
        <li>Track solar radiation for foraging conditions</li>
        <li>Understand light exposure at hive location</li>
        <li>Correlate activity with ambient light levels</li>
    </ul>
</div>

<h2>Bee Counter</h2>

<div class="feature-box">
    <h4>Counter Types</h4>
    <ul>
        <li><strong>PI Camera:</strong> Vision-based counting using Raspberry Pi camera</li>
        <li><strong>Gates:</strong> Physical gate sensors at entrance</li>
    </ul>
</div>

<div class="feature-box">
    <h4>Note</h4>
    <p>Bee counting requires additional software configuration beyond this settings page.</p>
</div>

<h2>Camera</h2>

<div class="feature-box">
    <h4>Camera Types</h4>
    <ul>
        <li><strong>PI Camera:</strong> Raspberry Pi camera module</li>
        <li><strong>USB:</strong> Standard USB webcam</li>
    </ul>
</div>

<div class="feature-box">
    <h4>Purpose</h4>
    <ul>
        <li>Time-lapse photography</li>
        <li>Activity monitoring</li>
        <li>Entrance observation</li>
        <li>Bee counting (with PI Camera)</li>
    </ul>
</div>

<h2>Weather Sources</h2>

<div class="feature-box">
    <h4>Available Weather Sources</h4>

    <p><strong>WX Underground</strong></p>
    <ul>
        <li>Requires station ID from Weather Underground</li>
        <li>Find stations at <a href="https://www.wunderground.com/" target="_blank">wunderground.com</a></li>
        <li>Choose station closest to your hive</li>
    </ul>

    <p><strong>AmbientWeather.net</strong></p>
    <ul>
        <li>Requires API key and station ID</li>
        <li>Get credentials from AmbientWeather.net account</li>
    </ul>

    <p><strong>Local Weather Station</strong></p>
    <ul>
        <li>Supports WS1400ip and OurWeather stations</li>
        <li>Enter local IP address of station</li>
    </ul>

    <p><strong>Local Hive Sensors</strong></p>
    <ul>
        <li>Use temperature/humidity sensor for weather data</li>
        <li>Mount sensor outside hive for ambient readings</li>
        <li>Configure same way as hive temp sensor</li>
    </ul>

    <p><strong>WF Tempest UDP</strong></p>
    <ul>
        <li>WeatherFlow Tempest station via local UDP</li>
        <li>Requires station serial number</li>
    </ul>
</div>

<h2>Air Quality Monitoring</h2>

<div class="feature-box">
    <h4>PurpleAir Integration</h4>

    <p><strong>From API:</strong></p>
    <ul>
        <li>Station ID from PurpleAir.com map</li>
        <li>API key from PurpleAir account</li>
    </ul>

    <p><strong>From Local Sensor:</strong></p>
    <ul>
        <li>Station ID (sensor MAC address)</li>
        <li>Local URL/IP of sensor</li>
    </ul>
</div>

<h2>Calibration Fields</h2>

<div class="feature-box">
    <h4>Zero/Intercept</h4>
    <p><strong>Purpose:</strong> Offset value subtracted from raw sensor reading</p>
    <p><strong>Example:</strong> If sensor reads 1020 and intercept is 20, final value is 1000</p>
    <p><strong>Default:</strong> Set to 0 if not needed</p>
</div>

<div class="feature-box">
    <h4>Calibration/Slope</h4>
    <p><strong>Purpose:</strong> Multiplier applied to sensor reading</p>
    <p><strong>Formula:</strong> Final = (Raw Reading × Slope) + Intercept</p>
    <p><strong>Default:</strong> Set to 1 if not needed</p>
</div>

<h2>Reading Live Sensors</h2>

<div class="alert alert-info">
    <strong>Test Your Setup:</strong> Click "Read Sensors" button to see live values from configured instruments.
    <br>This helps verify sensors are working before data collection begins.
</div>

<h2>Common Issues</h2>

<div class="alert alert-warning">
    <strong>Sensor Not Reading?</strong>
    <ul>
        <li>Verify sensor is physically connected</li>
        <li>Check device path/GPIO pin is correct</li>
        <li>Ensure sensor is enabled in settings</li>
        <li>Check system logs for errors</li>
        <li>Verify sensor has power (if applicable)</li>
    </ul>
</div>

<h2>Pro Tips</h2>

<ul>
    <li><strong>Start Simple:</strong> Enable one sensor at a time to troubleshoot issues</li>
    <li><strong>Use "Read Sensors":</strong> Frequently test live readings during setup</li>
    <li><strong>Document Your Setup:</strong> Note which GPIO pins are used for what</li>
    <li><strong>Weather First:</strong> Get weather data working before fine-tuning sensor calibration</li>
</ul>
