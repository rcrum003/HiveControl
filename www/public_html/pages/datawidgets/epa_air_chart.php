<?php
// EPA Gaseous Pollutants Detail Chart — O3 and NO2 from AirNow

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

switch ($period) {
    case "today": $sqlperiod = "start of day"; break;
    case "day": $sqlperiod = "-1 days"; break;
    case "week": $sqlperiod = "-7 days"; break;
    case "month": $sqlperiod = "-1 months"; break;
    case "year": $sqlperiod = "-1 years"; break;
    case "all": $sqlperiod = "-20 years"; break;
}

$sth = $conn->prepare("SELECT o3_ppm, o3_aqi, no2_ppb, no2_aqi, pm25_aqi, pm10_aqi, reporting_area, strftime('%s',date)*1000 AS datetime FROM airquality_epa WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
$sth->execute();
$epa_data = $sth->fetchAll(PDO::FETCH_ASSOC);

if (empty($epa_data)) {
    echo '<div class="panel panel-default"><div class="panel-heading">EPA Pollutant Data (O3 / NO2)</div><div class="panel-body"><p class="text-muted">No EPA AirNow data available for this period. Enable AirNow in <a href="/admin/instrumentconfig.php">Instrument Config</a>.</p></div></div>';
    return;
}

$area = htmlspecialchars($epa_data[0]['reporting_area'] ?? 'Unknown', ENT_QUOTES);

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");
?>

<div class="panel panel-default">
    <div class="panel-heading">EPA Pollutant Detail &mdash; <?php echo $area; ?></div>
    <div class="panel-body">
        <div id="epa-detail-chart"></div>
    </div>
</div>

<!-- Highcharts -->
<script src="/js/highcharts/highcharts.js"></script>
<script src="/js/highcharts/modules/exporting.js"></script>

<script>
Highcharts.chart('epa-detail-chart', {
    chart: { zoomType: 'x' },
    title: { text: '' },
    xAxis: { type: 'datetime' },
    yAxis: [{
        title: { text: 'Concentration' },
        labels: { format: '{value}' },
        min: 0
    }, {
        title: { text: 'AQI', style: { color: '#FF6347' } },
        labels: { format: '{value}', style: { color: '#FF6347' } },
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
        name: 'O3 (ppm)',
        data: [<?php foreach($epa_data as $r) {
            $val = is_numeric($r['o3_ppm']) ? round(floatval($r['o3_ppm']), 4) : null;
            echo "[".$r['datetime'].",".($val !== null ? $val : "null")."],";
        } ?>],
        color: '#9370DB',
        tooltip: { valueSuffix: ' ppm' }
    }, {
        name: 'NO2 (ppb)',
        data: [<?php foreach($epa_data as $r) {
            $val = is_numeric($r['no2_ppb']) ? round(floatval($r['no2_ppb']), 1) : null;
            echo "[".$r['datetime'].",".($val !== null ? $val : "null")."],";
        } ?>],
        color: '#20B2AA',
        tooltip: { valueSuffix: ' ppb' }
    }, {
        name: 'O3 AQI',
        yAxis: 1,
        data: [<?php foreach($epa_data as $r) {
            $val = is_numeric($r['o3_aqi']) ? intval($r['o3_aqi']) : null;
            echo "[".$r['datetime'].",".($val !== null ? $val : "null")."],";
        } ?>],
        color: '#9370DB',
        dashStyle: 'Dash'
    }, {
        name: 'NO2 AQI',
        yAxis: 1,
        data: [<?php foreach($epa_data as $r) {
            $val = is_numeric($r['no2_aqi']) ? intval($r['no2_aqi']) : null;
            echo "[".$r['datetime'].",".($val !== null ? $val : "null")."],";
        } ?>],
        color: '#20B2AA',
        dashStyle: 'Dash'
    }, {
        name: 'PM2.5 AQI (EPA)',
        yAxis: 1,
        data: [<?php foreach($epa_data as $r) {
            $val = is_numeric($r['pm25_aqi']) ? intval($r['pm25_aqi']) : null;
            echo "[".$r['datetime'].",".($val !== null ? $val : "null")."],";
        } ?>],
        color: '#FF6347',
        dashStyle: 'ShortDot',
        visible: false
    }]
});
</script>
