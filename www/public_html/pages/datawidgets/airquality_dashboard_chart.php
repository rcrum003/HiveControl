<?php
include_once "dashboard_chart_shared.php";

$cfg_sth = $conn->prepare("SELECT alert_pm25_threshold, alert_smoke_aqi_threshold FROM hiveconfig WHERE id=1");
$cfg_sth->execute();
$cfg = $cfg_sth->fetch(PDO::FETCH_ASSOC);
$pm25_threshold = ($cfg && is_numeric($cfg['alert_pm25_threshold'])) ? floatval($cfg['alert_pm25_threshold']) : 35.5;
$smoke_aqi_threshold = ($cfg && is_numeric($cfg['alert_smoke_aqi_threshold'])) ? intval($cfg['alert_smoke_aqi_threshold']) : 150;
?>

<script>
$(function () {
    var aqiSeries = [{
        name: 'PM2.5 (ug/m3)', yAxis: 0,
        data: [<?php echo implode(',', $data_pm25); ?>],
        color: '<?php echo $color_pm2_5; ?>',
        visible: <?php echo $trend_pm2_5; ?>
    }, {
        name: 'PM10 (ug/m3)', yAxis: 0,
        data: [<?php echo implode(',', $data_pm10); ?>],
        color: '<?php echo $color_pm10; ?>',
        visible: <?php echo $trend_pm10; ?>
    }, {
        name: 'PM2.5 AQI', yAxis: 1,
        data: [<?php echo implode(',', $data_pm25_aqi); ?>],
        color: '<?php echo $color_pm2_5_aqi; ?>',
        dashStyle: 'Dash',
        visible: <?php echo $trend_pm2_5_aqi; ?>
    }];

    <?php if (!empty($chart_result_epa)) { ?>
    aqiSeries.push({
        name: 'O3 AQI (EPA)', yAxis: 1,
        data: [<?php echo implode(',', $data_epa_o3); ?>],
        color: '<?php echo $color_o3; ?>',
        dashStyle: 'ShortDot'
    }, {
        name: 'NO2 AQI (EPA)', yAxis: 1,
        data: [<?php echo implode(',', $data_epa_no2); ?>],
        color: '<?php echo $color_no2; ?>',
        dashStyle: 'ShortDot',
        visible: false
    });
    <?php } ?>

    var chart = Highcharts.chart('aqcontainer', {
        chart: { zoomType: 'x', height: 280 },
        title: { text: '' },
        xAxis: hcCommonXAxis,
        yAxis: [{
            gridLineWidth: 1,
            title: { text: 'Concentration (ug/m3)', style: { color: '#000' } },
            labels: { format: '{value}', style: { color: '#000' } },
            minRange: 10, floor: 0,
            plotLines: [{
                value: <?php echo $pm25_threshold; ?>,
                color: '#FF7E00', width: 1, dashStyle: 'Dash', zIndex: 3,
                label: { text: 'USG', style: { fontSize: '10px', color: '#FF7E00' }, align: 'right' }
            }]
        }, {
            gridLineWidth: 0,
            title: { text: 'AQI', style: { color: '#FF6347' } },
            labels: { format: '{value}', style: { color: '#FF6347' } },
            min: 0, max: 300, opposite: true,
            plotBands: [
                { from: 0, to: 50, color: 'rgba(0,228,0,0.12)', label: { text: 'Good', align: 'right', x: -5, style: { fontSize: '10px', color: '#00A000' } } },
                { from: 50, to: 100, color: 'rgba(255,255,0,0.10)', label: { text: 'Moderate', align: 'right', x: -5, style: { fontSize: '10px', color: '#B0B000' } } },
                { from: 100, to: 150, color: 'rgba(255,126,0,0.10)', label: { text: 'USG', align: 'right', x: -5, style: { fontSize: '10px', color: '#DD6600' } } },
                { from: 150, to: 200, color: 'rgba(255,0,0,0.08)', label: { text: 'Unhealthy', align: 'right', x: -5, style: { fontSize: '10px', color: '#DD0000' } } },
                { from: 200, to: 300, color: 'rgba(143,63,151,0.08)', label: { text: 'Very Unhealthy', align: 'right', x: -5, style: { fontSize: '9px', color: '#8F3F97' } } }
            ],
            plotLines: [{
                value: <?php echo $smoke_aqi_threshold; ?>,
                color: '#FF0000', width: 2, dashStyle: 'Dash', zIndex: 3,
                label: { text: 'Smoke', style: { fontSize: '10px', color: '#FF0000' }, align: 'left' }
            }]
        }],
        plotOptions: {
            <?php if ($chart_smoothing == "on") echo "series: { connectNulls: true },"; ?>
            line: { marker: { enabled: <?php echo $show_markers; ?>, radius: 1.5, symbol: 'circle' }, lineWidth: 2 }
        },
        tooltip: hcCommonTooltip,
        exporting: hcCommonExporting,
        series: aqiSeries
    });
    dashboardCharts.push(chart);
});
</script>
