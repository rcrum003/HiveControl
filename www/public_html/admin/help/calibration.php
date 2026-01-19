<h1><i class="fa fa-balance-scale"></i> Calibration Guide</h1>

<div class="alert alert-info">
    <strong>Why Calibrate?</strong> Calibration ensures your sensors provide accurate, reliable data for making informed beekeeping decisions.
</div>

<h2>Weight Scale Calibration (HX711)</h2>

<div class="feature-box">
    <h4>When to Calibrate</h4>
    <ul>
        <li>Initial installation</li>
        <li>After moving the hive</li>
        <li>After replacing load cells</li>
        <li>If readings seem inaccurate</li>
        <li>Seasonally (recommend twice yearly)</li>
    </ul>
</div>

<h3>HX711 Calibration Wizard</h3>

<p><span class="step-number">1</span> <strong>Access Calibration</strong></p>
<ul>
    <li>Go to Settings → Instruments</li>
    <li>Ensure HX711 scale is enabled</li>
    <li>Click "Read Sensors" button</li>
    <li>Click "Calibrate Wizard" button in the Weight Scale row</li>
</ul>

<p><span class="step-number">2</span> <strong>Zero the Scale</strong></p>
<ul>
    <li>Remove all weight from the scale (empty hive if possible)</li>
    <li>Click "Zero" or "Tare" in the wizard</li>
    <li>Wait for reading to stabilize</li>
    <li>Record the zero reading</li>
</ul>

<p><span class="step-number">3</span> <strong>Apply Known Weight</strong></p>
<ul>
    <li>Place a known weight on the scale (e.g., 50 lbs / 20 kg)</li>
    <li>Weight should be similar to expected hive weight changes</li>
    <li>Enter the known weight value in the wizard</li>
    <li>Click "Calibrate"</li>
</ul>

<p><span class="step-number">4</span> <strong>Verify Calibration</strong></p>
<ul>
    <li>Remove the known weight</li>
    <li>Scale should return close to zero</li>
    <li>Re-apply known weight to verify accuracy</li>
    <li>If accurate, save calibration values</li>
</ul>

<div class="alert alert-warning">
    <strong>Important:</strong> Calibration values (Slope and Intercept) are automatically calculated and saved when you complete the wizard.
</div>

<h2>Temperature Sensor Calibration</h2>

<div class="feature-box">
    <h4>Method: Comparison Calibration</h4>

    <p><span class="step-number">1</span> <strong>Get Reference Temperature</strong></p>
    <ul>
        <li>Use a calibrated thermometer as reference</li>
        <li>Place both sensors in same environment</li>
        <li>Wait 10-15 minutes for readings to stabilize</li>
    </ul>

    <p><span class="step-number">2</span> <strong>Calculate Offset</strong></p>
    <ul>
        <li>Note HiveControl sensor reading (A)</li>
        <li>Note reference thermometer reading (B)</li>
        <li>Calculate offset: B - A = Intercept</li>
    </ul>

    <p><span class="step-number">3</span> <strong>Apply Calibration</strong></p>
    <ul>
        <li>Go to Settings → Instruments</li>
        <li>Find Temperature Intercept field</li>
        <li>Enter calculated offset value</li>
        <li>Leave Slope at 1</li>
    </ul>
</div>

<div class="code-block">
    <strong>Example:</strong>
    <ul>
        <li>HiveControl reads: 72°F</li>
        <li>Reference reads: 75°F</li>
        <li>Intercept = 75 - 72 = 3</li>
        <li>Enter "3" in Temp Intercept field</li>
    </ul>
</div>

<h2>Humidity Sensor Calibration</h2>

<div class="feature-box">
    <h4>Salt Test Method</h4>

    <p><span class="step-number">1</span> <strong>Prepare Salt Solution</strong></p>
    <ul>
        <li>Mix table salt with small amount of water to make thick slurry</li>
        <li>Place in small container (not touching sensor)</li>
    </ul>

    <p><span class="step-number">2</span> <strong>Create Test Chamber</strong></p>
    <ul>
        <li>Place salt container and sensor in sealed bag/container</li>
        <li>Wait 8-12 hours for humidity to stabilize</li>
        <li>Salt solution creates 75% relative humidity environment</li>
    </ul>

    <p><span class="step-number">3</span> <strong>Calculate and Apply</strong></p>
    <ul>
        <li>Sensor should read 75%</li>
        <li>If it reads differently, calculate offset: 75 - (reading) = Intercept</li>
        <li>Enter intercept in Humidity Intercept field</li>
    </ul>
</div>

<h2>Understanding Calibration Values</h2>

<div class="feature-box">
    <h4>The Calibration Formula</h4>
    <div class="code-block">
        <strong>Final Reading = (Raw Sensor Value × Slope) + Intercept</strong>
    </div>

    <p><strong>Slope (Multiplier):</strong></p>
    <ul>
        <li>Default: 1 (no adjustment)</li>
        <li>Use when sensor scale is wrong (e.g., reads double)</li>
        <li>Example: If sensor reads 100 but should read 50, Slope = 0.5</li>
    </ul>

    <p><strong>Intercept (Offset):</strong></p>
    <ul>
        <li>Default: 0 (no adjustment)</li>
        <li>Use when sensor has constant offset</li>
        <li>Example: If sensor always reads 5° high, Intercept = -5</li>
    </ul>
</div>

<h2>Calibration Best Practices</h2>

<ul>
    <li><strong>Stable Environment:</strong> Calibrate in stable temperature/humidity conditions</li>
    <li><strong>Wait for Stabilization:</strong> Allow sensors time to reach equilibrium</li>
    <li><strong>Multiple Points:</strong> For critical measurements, test at multiple values</li>
    <li><strong>Document Everything:</strong> Keep notes on when/how you calibrated</li>
    <li><strong>Regular Checks:</strong> Periodically verify calibration accuracy</li>
    <li><strong>Realistic Weights:</strong> Use calibration weights similar to measurement range</li>
</ul>

<h2>Troubleshooting Calibration</h2>

<div class="alert alert-warning">
    <strong>Readings Still Incorrect?</strong>
    <ul>
        <li>Verify sensor is working (check connections)</li>
        <li>Try re-calibrating from scratch</li>
        <li>Check for environmental interference (wind, sunlight)</li>
        <li>Ensure scale platform is level</li>
        <li>Look for mechanical issues (binding, friction)</li>
        <li>Consider sensor replacement if consistently inaccurate</li>
    </ul>
</div>

<h2>Advanced: Two-Point Calibration</h2>

<div class="feature-box">
    <h4>For Maximum Accuracy</h4>

    <p><span class="step-number">1</span> <strong>Collect Two Reference Points</strong></p>
    <ul>
        <li>Point 1: Low end of range (e.g., 0 lbs)</li>
        <li>Point 2: High end of range (e.g., 100 lbs)</li>
        <li>Note raw sensor readings (R1, R2) and true values (T1, T2)</li>
    </ul>

    <p><span class="step-number">2</span> <strong>Calculate Slope</strong></p>
    <div class="code-block">
        Slope = (T2 - T1) / (R2 - R1)
    </div>

    <p><span class="step-number">3</span> <strong>Calculate Intercept</strong></p>
    <div class="code-block">
        Intercept = T1 - (R1 × Slope)
    </div>

    <p><span class="step-number">4</span> <strong>Enter Values</strong></p>
    <ul>
        <li>Enter calculated Slope in Calibration/Slope field</li>
        <li>Enter calculated Intercept in Zero/Intercept field</li>
    </ul>
</div>

<h2>Testing Calibration</h2>

<div class="code-block">
    <p><strong>Always test after calibration:</strong></p>
    <ol>
        <li>Click "Read Sensors" to see live readings</li>
        <li>Apply known reference (weight, temperature, etc.)</li>
        <li>Verify reading matches expected value</li>
        <li>Test at multiple points if possible</li>
        <li>Allow time for data collection to confirm accuracy over time</li>
    </ol>
</div>
