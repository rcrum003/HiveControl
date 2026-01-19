<h1><i class="fa fa-dashboard"></i> Dashboard Guide</h1>

<p>The dashboard is your primary interface for monitoring your hive's health and activity.</p>

<h2>Dashboard Layout</h2>

<div class="feature-box">
    <h4>Top Panels - Current Conditions</h4>
    <p>Four panels display real-time metrics:</p>

    <p><strong>1. Temperature Panel (Green)</strong></p>
    <ul>
        <li>Left value: Hive internal temperature</li>
        <li>Right value: Ambient (outside) temperature</li>
        <li>Shows °F or °C based on your settings</li>
        <li>Click panel to view detailed temperature charts</li>
    </ul>

    <p><strong>2. Weight Panel (Yellow/Gold)</strong></p>
    <ul>
        <li>Left value: Current hive weight</li>
        <li>Right value: Weight change over selected period</li>
        <li>Shows lb or kg based on your settings</li>
        <li>Positive change = weight gain, Negative = weight loss</li>
        <li>Click panel to view detailed weight charts</li>
    </ul>

    <p><strong>3. Humidity Panel (Blue)</strong></p>
    <ul>
        <li>Left value: Hive internal humidity</li>
        <li>Right value: Ambient (outside) humidity</li>
        <li>Shows percentage (%)  </li>
        <li>Click panel to view detailed humidity charts</li>
    </ul>

    <p><strong>4. Growing Degree Days Panel (Brown)</strong></p>
    <ul>
        <li>Left value: Today's GDD accumulation</li>
        <li>Right value: Season total GDD</li>
        <li>Helps predict plant blooms and nectar flow</li>
        <li>Click panel to view detailed GDD charts</li>
    </ul>
</div>

<h2>Time Period Selector</h2>

<div class="feature-box">
    <h4>View Data Across Different Time Ranges</h4>
    <p>Use the button bar to change the time period displayed in charts:</p>

    <ul>
        <li><strong>Today:</strong> Midnight to now</li>
        <li><strong>24 hrs:</strong> Last 24 hours from current time</li>
        <li><strong>Week:</strong> Last 7 days</li>
        <li><strong>Month:</strong> Last 30 days</li>
        <li><strong>Year:</strong> Last 365 days</li>
        <li><strong>All:</strong> Complete history (all data)</li>
    </ul>

    <div class="alert alert-info">
        <strong>Tip:</strong> Use "Today" to see current day activity, "Week" for trends, and "All" for long-term patterns.
    </div>
</div>

<h2>Main Chart Area</h2>

<div class="feature-box">
    <h4>Combined Chart Display</h4>
    <p>The main chart shows multiple metrics overlaid:</p>

    <ul>
        <li><strong>Weight:</strong> Hive weight trend over time</li>
        <li><strong>Temperature:</strong> Hive and ambient temperatures</li>
        <li><strong>Humidity:</strong> Hive and ambient humidity levels</li>
        <li><strong>Light/Solar:</strong> Light intensity or solar radiation</li>
    </ul>

    <p><strong>Chart Features:</strong></p>
    <ul>
        <li><strong>Legend:</strong> Click legend items to show/hide specific data series</li>
        <li><strong>Zoom:</strong> Click and drag on chart to zoom into time period</li>
        <li><strong>Pan:</strong> After zooming, drag chart to pan left/right</li>
        <li><strong>Reset:</strong> Double-click chart to reset zoom</li>
        <li><strong>Export:</strong> Use hamburger menu (top right) to download chart as image</li>
        <li><strong>Fullscreen:</strong> Click fullscreen icon for expanded view</li>
    </ul>
</div>

<h2>Estimated Stores Gauge (Normal View)</h2>

<div class="feature-box">
    <h4>Honey Store Estimation</h4>
    <p>Visual gauge showing estimated hive contents:</p>

    <ul>
        <li><strong>Green Zone:</strong> Honey stores</li>
        <li><strong>Yellow Zone:</strong> Pollen</li>
        <li><strong>Blue Zone:</strong> Nectar (not yet converted to honey)</li>
    </ul>

    <p><strong>How it's calculated:</strong></p>
    <ul>
        <li>Based on weight changes over time</li>
        <li>Considers seasonal patterns</li>
        <li>Estimates total stores vs. consumed</li>
    </ul>

    <div class="alert alert-warning">
        <strong>Note:</strong> This is an estimate. Actual stores should be verified by physical inspection.
    </div>
</div>

<h2>System Status Panel (Normal View)</h2>

<div class="feature-box">
    <h4>Quick System Health Check</h4>
    <p>Shows operational status of various components:</p>

    <ul>
        <li><strong>Database Status:</strong> Connection and health</li>
        <li><strong>Last Update:</strong> When data was last collected</li>
        <li><strong>Sensor Status:</strong> Which sensors are active</li>
        <li><strong>API Status:</strong> Connection to external services</li>
    </ul>
</div>

<h2>View Modes</h2>

<div class="feature-box">
    <h4>Normal vs. Compact</h4>

    <p><strong>Normal View:</strong></p>
    <ul>
        <li>Shows charts, gauges, and status panels</li>
        <li>Best for desktop monitors</li>
        <li>Full feature display</li>
    </ul>

    <p><strong>Compact View:</strong></p>
    <ul>
        <li>Shows only essential charts</li>
        <li>Better for mobile devices or smaller screens</li>
        <li>Configured in settings (SITE_TYPE)</li>
    </ul>
</div>

<h2>Understanding the Data</h2>

<div class="feature-box">
    <h4>Reading Weight Trends</h4>

    <p><strong>Daily Patterns:</strong></p>
    <ul>
        <li><strong>Morning Gain:</strong> Bees returning with nectar</li>
        <li><strong>Midday Dip:</strong> Water evaporation, bees out foraging</li>
        <li><strong>Evening Weight:</strong> Daily nectar collection total</li>
    </ul>

    <p><strong>Seasonal Patterns:</strong></p>
    <ul>
        <li><strong>Spring:</strong> Steady gain during nectar flow</li>
        <li><strong>Summer:</strong> Peak weight, then plateau</li>
        <li><strong>Fall:</strong> Weight loss as bees consume stores</li>
        <li><strong>Winter:</strong> Slow, steady decline</li>
    </ul>
</div>

<div class="feature-box">
    <h4>Reading Temperature Patterns</h4>

    <p><strong>Normal Hive Temperature:</strong></p>
    <ul>
        <li><strong>Brood Area:</strong> 93-95°F (34-35°C) year-round</li>
        <li><strong>Non-Brood:</strong> Varies with ambient temperature</li>
        <li><strong>Winter Cluster:</strong> 50-80°F (10-27°C) depending on cluster activity</li>
    </ul>

    <p><strong>Warning Signs:</strong></p>
    <ul>
        <li><strong>Too Hot:</strong> >100°F (38°C) - ventilation issues or extreme weather</li>
        <li><strong>Too Cold:</strong> <50°F (10°C) in winter - weak colony or queen failure</li>
        <li><strong>Erratic:</strong> Wild temperature swings - possible colony issues</li>
    </ul>
</div>

<h2>Auto-Refresh</h2>

<div class="alert alert-info">
    <strong>Automatic Updates:</strong> The dashboard automatically refreshes every 5 minutes to show the latest data. You can manually refresh anytime by reloading the page.
</div>

<h2>Dashboard Tips</h2>

<ul>
    <li><strong>Check Daily:</strong> Review dashboard each morning to spot trends</li>
    <li><strong>Compare to Weather:</strong> Weight/activity often correlates with weather</li>
    <li><strong>Use Time Periods:</strong> Switch between periods to see different patterns</li>
    <li><strong>Look for Anomalies:</strong> Sudden changes may indicate issues or events</li>
    <li><strong>Track GDD:</strong> Use to predict nectar flows in your region</li>
    <li><strong>Correlate Metrics:</strong> Temperature spikes may cause weight loss (water evaporation)</li>
    <li><strong>Document Events:</strong> Note hive inspections, treatments, or weather events</li>
</ul>

<h2>Common Observations</h2>

<div class="code-block">
    <p><strong>Steady Weight Gain:</strong></p>
    <ul>
        <li>Active nectar flow</li>
        <li>Good foraging conditions</li>
        <li>Healthy, productive colony</li>
    </ul>

    <p><strong>Sudden Weight Loss:</strong></p>
    <ul>
        <li>Nectar flow ended</li>
        <li>Swarming event</li>
        <li>Robbing by other bees</li>
        <li>Honey harvest by beekeeper</li>
    </ul>

    <p><strong>Temperature Spikes:</strong></p>
    <ul>
        <li>Hot weather</li>
        <li>Poor ventilation</li>
        <li>Increased bee activity</li>
    </ul>

    <p><strong>Temperature Drops:</strong></p>
    <ul>
        <li>Cold weather</li>
        <li>Reduced bee cluster</li>
        <li>Possible colony loss</li>
    </ul>
</div>

<h2>Metric Reference Values</h2>

<table class="table table-bordered">
    <thead>
        <tr>
            <th>Metric</th>
            <th>Ideal Range</th>
            <th>Warning Level</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td>Hive Temp</td>
            <td>90-95°F (32-35°C)</td>
            <td><60°F or >100°F</td>
        </tr>
        <tr>
            <td>Hive Humidity</td>
            <td>50-70%</td>
            <td><40% or >80%</td>
        </tr>
        <tr>
            <td>Weight Change (Summer)</td>
            <td>+1 to +5 lbs/day</td>
            <td>Negative (loss)</td>
        </tr>
        <tr>
            <td>Weight Change (Winter)</td>
            <td>-0.5 to -1 lb/week</td>
            <td>>-2 lbs/week</td>
        </tr>
    </tbody>
</table>
