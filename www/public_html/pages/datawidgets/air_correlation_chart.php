<?php
// Air Quality Correlation Chart — PM2.5/O3 vs Foraging Activity and Weight Gain

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

switch ($period) {
    case "today": $sqlperiod = "start of day"; break;
    case "day": $sqlperiod = "-1 days"; break;
    case "week": $sqlperiod = "-7 days"; break;
    case "month": $sqlperiod = "-1 months"; break;
    case "year": $sqlperiod = "-1 years"; break;
    case "all": $sqlperiod = "-20 years"; break;
}

// Foraging (bee count) + PM2.5 data
$sth = $conn->prepare("SELECT IN_COUNT, OUT_COUNT, air_pm2_5_raw, air_pm2_5_aqi, air_aqi, hiveweight, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
$sth->execute();
$hive_data = $sth->fetchAll(PDO::FETCH_ASSOC);

// O3 AQI from EPA table
$epa_sth = $conn->prepare("SELECT o3_aqi, strftime('%s',date)*1000 AS datetime FROM airquality_epa WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
$epa_sth->execute();
$epa_data = $epa_sth->fetchAll(PDO::FETCH_ASSOC);

// Daily weight change for multi-day correlation
$daily_sth = $conn->prepare("SELECT strftime('%Y-%m-%d', date) as day, MAX(hiveweight) - MIN(hiveweight) as weight_change, AVG(air_pm2_5_aqi) as avg_pm25_aqi, AVG(air_pm2_5_raw) as avg_pm25_raw, strftime('%s', date) * 1000 as datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') AND hiveweight > 0 GROUP BY strftime('%Y-%m-%d', date) ORDER BY day ASC");
$daily_sth->execute();
$daily_data = $daily_sth->fetchAll(PDO::FETCH_ASSOC);

$smoke_threshold = 100;
$cfg_sth = $conn->prepare("SELECT alert_smoke_aqi_threshold FROM hiveconfig WHERE id=1");
$cfg_sth->execute();
$cfg = $cfg_sth->fetch(PDO::FETCH_ASSOC);
if ($cfg && is_numeric($cfg['alert_smoke_aqi_threshold'])) {
    $smoke_threshold = intval($cfg['alert_smoke_aqi_threshold']);
}

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");
?>

<!-- Foraging vs AQ Chart -->
<div class="panel panel-default">
    <div class="panel-heading">Foraging Activity vs Air Quality</div>
    <div class="panel-body">
        <div id="forage-aq-chart"></div>
    </div>
</div>

<!-- Weight vs Multi-Day AQI -->
<div class="panel panel-default">
    <div class="panel-heading">Daily Weight Change vs Average AQI</div>
    <div class="panel-body">
        <div id="weight-aq-chart"></div>
    </div>
</div>

<script>
// Chart 1: Foraging vs PM2.5 and O3
Highcharts.chart('forage-aq-chart', {
    chart: { zoomType: 'x' },
    title: { text: '' },
    xAxis: { type: 'datetime' },
    yAxis: [{
        title: { text: 'Bee Count' },
        min: 0
    }, {
        title: { text: 'AQI' },
        opposite: true,
        min: 0,
        plotBands: [
            { from: 0, to: 50, color: 'rgba(0,228,0,0.06)' },
            { from: 51, to: 100, color: 'rgba(255,255,0,0.06)' },
            { from: 101, to: 150, color: 'rgba(255,126,0,0.06)' },
            { from: 151, to: 500, color: 'rgba(255,0,0,0.06)' }
        ]
    }],
    tooltip: { shared: true },
    plotOptions: { line: { marker: { enabled: true, radius: 2 } } },
    series: [{
        name: 'Bee Activity (IN+OUT)',
        data: [<?php foreach($hive_data as $r) {
            $count = null;
            if (is_numeric($r['IN_COUNT']) && is_numeric($r['OUT_COUNT'])) {
                $count = intval($r['IN_COUNT']) + intval($r['OUT_COUNT']);
            }
            echo "[".$r['datetime'].",".($count !== null ? $count : "null")."],";
        } ?>],
        color: '#FFD700'
    }, {
        name: 'PM2.5 AQI',
        yAxis: 1,
        data: [<?php foreach($hive_data as $r) {
            $val = is_numeric($r['air_pm2_5_aqi']) ? intval($r['air_pm2_5_aqi']) : (is_numeric($r['air_aqi']) ? intval($r['air_aqi']) : null);
            echo "[".$r['datetime'].",".($val !== null ? $val : "null")."],";
        } ?>],
        color: '#FF6347',
        dashStyle: 'Dash'
    }, {
        name: 'O3 AQI (EPA)',
        yAxis: 1,
        data: [<?php foreach($epa_data as $r) {
            $val = is_numeric($r['o3_aqi']) ? intval($r['o3_aqi']) : null;
            echo "[".$r['datetime'].",".($val !== null ? $val : "null")."],";
        } ?>],
        color: '#9370DB',
        dashStyle: 'ShortDot'
    }]
});

// Chart 2: Daily Weight Change vs Avg AQI
Highcharts.chart('weight-aq-chart', {
    chart: { zoomType: 'x' },
    title: { text: '' },
    xAxis: { type: 'datetime' },
    yAxis: [{
        title: { text: 'Weight Change (<?php echo ($SHOW_METRIC == "on") ? "kg" : "lb"; ?>)' }
    }, {
        title: { text: 'Avg PM2.5 AQI' },
        opposite: true,
        min: 0,
        plotLines: [{
            value: <?php echo $smoke_threshold; ?>,
            color: '#FF0000',
            width: 2,
            dashStyle: 'Dash',
            label: { text: 'Smoke Threshold', style: { color: '#FF0000', fontSize: '10px' } }
        }]
    }],
    tooltip: { shared: true },
    series: [{
        type: 'column',
        name: 'Daily Weight Change',
        data: [<?php foreach($daily_data as $r) {
            $val = is_numeric($r['weight_change']) ? round(floatval($r['weight_change']), 2) : null;
            if ($SHOW_METRIC == "on" && $val !== null) { $val = round($val * 0.453592, 2); }
            echo "[".$r['datetime'].",".($val !== null ? $val : "null")."],";
        } ?>],
        color: '#5cb85c'
    }, {
        type: 'line',
        name: 'Avg PM2.5 AQI',
        yAxis: 1,
        data: [<?php foreach($daily_data as $r) {
            $val = is_numeric($r['avg_pm25_aqi']) ? round(floatval($r['avg_pm25_aqi']), 0) : null;
            echo "[".$r['datetime'].",".($val !== null ? $val : "null")."],";
        } ?>],
        color: '#FF6347'
    }]
});
</script>
