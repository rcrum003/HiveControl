
<!-- Air Quality Charts — Split: PM Concentrations + AQI Index -->

<?php
$period_map = [
    'today' => 'start of day', 'day' => '-1 days',
    'week'  => '-7 days',      'month' => '-1 months',
    'year'  => '-1 years',     'all'   => '-20 years',
];
$sqlperiod = isset($period_map[$period]) ? $period_map[$period] : 'start of day';

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sth = $conn->prepare("SELECT air_pm1, air_pm2_5, air_pm10, air_pm2_5_raw, air_pm10_raw, air_pm1_raw, air_pm2_5_aqi, air_pm10_aqi, air_aqi, air_pressure, weather_tempf, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', :p) ORDER by datetime ASC");
$sth->execute([':p' => $sqlperiod]);
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

$has_valid = false;
foreach ($result as $r) {
    if (is_numeric($r['air_pm2_5'] ?? null) || is_numeric($r['air_pm2_5_raw'] ?? null) || is_numeric($r['air_aqi'] ?? null)) {
        $has_valid = true;
        break;
    }
}
if (!$has_valid) {
    echo '<script>document.getElementById("pm-chart").parentElement.innerHTML = \'<div class="alert alert-info" style="margin:0"><i class="fa fa-info-circle"></i> <strong>No air quality data available</strong> for the selected time period. Data will appear here once an air quality sensor is configured and recording.</div>\'; var aqiPanel = document.getElementById("aqi-chart"); if (aqiPanel) aqiPanel.closest(".panel").style.display = "none";</script>';
    return;
}

$epa_result = [];
try {
    $epa_sth = $conn->prepare("SELECT o3_aqi, no2_aqi, pm25_aqi, strftime('%s',date)*1000 AS datetime FROM airquality_epa WHERE date > datetime('now', 'localtime', :p) ORDER by datetime ASC");
    $epa_sth->execute([':p' => $sqlperiod]);
    $epa_result = $epa_sth->fetchAll(PDO::FETCH_ASSOC);
} catch (PDOException $e) {
    // airquality_epa table may not exist on pre-2.16 databases
}

$cfg_sth = $conn->prepare("SELECT alert_pm25_threshold, alert_smoke_aqi_threshold FROM hiveconfig WHERE id=1");
$cfg_sth->execute();
$cfg = $cfg_sth->fetch(PDO::FETCH_ASSOC);
$pm25_threshold = ($cfg && is_numeric($cfg['alert_pm25_threshold'])) ? floatval($cfg['alert_pm25_threshold']) : 35.5;
$smoke_aqi_threshold = ($cfg && is_numeric($cfg['alert_smoke_aqi_threshold'])) ? intval($cfg['alert_smoke_aqi_threshold']) : 150;

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

// Build PHP data arrays
$pm25_data = []; $pm10_data = []; $pm1_data = [];
$pm25_aqi_data = []; $pm10_aqi_data = [];

foreach ($result as $r) {
    $ts = $r['datetime'];

    $pm25 = is_numeric($r['air_pm2_5_raw']) ? floatval($r['air_pm2_5_raw']) : (is_numeric($r['air_pm2_5']) ? floatval($r['air_pm2_5']) : null);
    $pm10 = is_numeric($r['air_pm10_raw']) ? floatval($r['air_pm10_raw']) : (is_numeric($r['air_pm10']) ? floatval($r['air_pm10']) : null);
    $pm1  = is_numeric($r['air_pm1_raw'])  ? floatval($r['air_pm1_raw'])  : (is_numeric($r['air_pm1'])  ? floatval($r['air_pm1'])  : null);

    $pm25_data[]     = "[$ts," . ($pm25 !== null ? $pm25 : "null") . "]";
    $pm10_data[]     = "[$ts," . ($pm10 !== null ? $pm10 : "null") . "]";
    $pm1_data[]      = "[$ts," . ($pm1  !== null ? $pm1  : "null") . "]";

    $aqi25 = is_numeric($r['air_pm2_5_aqi']) ? intval($r['air_pm2_5_aqi']) : (is_numeric($r['air_aqi']) ? intval($r['air_aqi']) : null);
    $aqi10 = is_numeric($r['air_pm10_aqi']) ? intval($r['air_pm10_aqi']) : null;
    $pm25_aqi_data[] = "[$ts," . ($aqi25 !== null ? $aqi25 : "null") . "]";
    $pm10_aqi_data[] = "[$ts," . ($aqi10 !== null ? $aqi10 : "null") . "]";
}

$o3_aqi_data = []; $no2_aqi_data = [];
foreach ($epa_result as $r) {
    $ts = $r['datetime'];
    $o3  = is_numeric($r['o3_aqi'])  ? intval($r['o3_aqi'])  : null;
    $no2 = is_numeric($r['no2_aqi']) ? intval($r['no2_aqi']) : null;
    $o3_aqi_data[]  = "[$ts," . ($o3  !== null ? $o3  : "null") . "]";
    $no2_aqi_data[] = "[$ts," . ($no2 !== null ? $no2 : "null") . "]";
}

$period_js = htmlspecialchars($period, ENT_QUOTES);
?>

<script>
$(function () {

    // Shared crosshair sync between the two charts
    var charts = [];

    function syncCrosshair(e, thisChart) {
        charts.forEach(function(chart) {
            if (chart !== thisChart && chart.xAxis && chart.xAxis[0]) {
                var xAxis = chart.xAxis[0],
                    point, event;

                event = chart.pointer.normalize(e);
                var x = xAxis.toValue(event.chartX || 0, true);
                chart.series.forEach(function(s) {
                    if (s.visible && s.points && s.points.length) {
                        point = s.searchPoint({ chartX: event.chartX, chartY: event.chartY }, true);
                    }
                });

                if (point) {
                    point.onMouseOver();
                    chart.xAxis[0].drawCrosshair(event, point);
                }
            }
        });
    }

    var commonXAxis = {
        type: 'datetime',
        crosshair: { width: 1, color: '#ccc' },
        dateTimeLabelFormats: {
            second: '%m-%d<br/>%H:%M:%S',
            minute: '%m-%d<br/>%H:%M',
            hour: '%m-%d<br/>%H:%M',
            day: '<br/>%m-%d',
            week: '<br/>%m-%d',
            month: '%Y-%m',
            year: '%Y'
        },
        events: {
            afterSetExtremes: function(e) {
                var triggerChart = this.chart;
                charts.forEach(function(c) {
                    if (c !== triggerChart && c.xAxis && c.xAxis[0]) {
                        c.xAxis[0].setExtremes(e.min, e.max, true, false);
                    }
                });
            }
        }
    };

    var commonTooltip = {
        shared: true,
        formatter: function () {
            var s = '<b>' + Highcharts.dateFormat('%m/%d %H:%M', this.x) + '</b>';
            this.points.forEach(function (point) {
                s += '<br/><span style="color:' + point.series.color + '">●</span> ' + point.series.name + ': <b>' + Highcharts.numberFormat(point.y, 1) + '</b>';
            });
            return s;
        }
    };

    var commonExporting = function(chartName) {
        var menuItems = ['viewFullscreen', 'printChart', 'separator', 'downloadPNG', 'downloadJPEG', 'downloadPDF', 'downloadSVG'];
        <?php if (empty($is_fullscreen)) { ?>
        menuItems.push('separator', {
            text: 'Enlarge Chart',
            onclick: function () {
                centeredPopup('/pages/fullscreen/air.php?chart=line&period=<?php echo $period_js; ?>', 'HiveControl', '1200', '500', 'yes');
                return false;
            }
        });
        <?php } ?>
        return {
            buttons: {
                contextButton: { menuItems: menuItems }
            }
        };
    };

    // Chart 1: Particulate Matter Concentrations
    var pmChart = Highcharts.chart('pm-chart', {
        chart: {
            zoomType: 'x',
            height: 300,
            marginRight: 10
        },
        title: { text: '' },
        xAxis: commonXAxis,
        yAxis: {
            title: { text: 'Concentration (ug/m3)' },
            minRange: 10,
            floor: 0,
            plotLines: [{
                value: <?php echo $pm25_threshold; ?>,
                color: '#FF7E00',
                width: 1,
                dashStyle: 'Dash',
                zIndex: 3,
                label: {
                    text: 'USG Threshold',
                    style: { fontSize: '10px', color: '#FF7E00' },
                    align: 'right'
                }
            }]
        },
        plotOptions: {
            line: {
                marker: { enabled: true, radius: 1.5, symbol: 'circle' },
                lineWidth: 2
            }
        },
        tooltip: commonTooltip,
        exporting: commonExporting('pm'),
        series: [{
            name: 'PM2.5',
            data: [<?php echo implode(',', $pm25_data); ?>],
            color: '<?php echo isset($color_pm2_5) ? $color_pm2_5 : "#FF6347"; ?>'
        }, {
            name: 'PM10',
            data: [<?php echo implode(',', $pm10_data); ?>],
            color: '<?php echo isset($color_pm10) ? $color_pm10 : "#FF8C00"; ?>'
        }, {
            name: 'PM1',
            data: [<?php echo implode(',', $pm1_data); ?>],
            color: '<?php echo isset($color_pm1) ? $color_pm1 : "#32CD32"; ?>',
            visible: false
        }]
    });
    charts.push(pmChart);

    // Chart 2: AQI Index with EPA color bands
    var aqiSeries = [{
        name: 'PM2.5 AQI',
        data: [<?php echo implode(',', $pm25_aqi_data); ?>],
        color: '<?php echo isset($color_pm2_5_aqi) ? $color_pm2_5_aqi : "#CC3333"; ?>'
    }, {
        name: 'PM10 AQI',
        data: [<?php echo implode(',', $pm10_aqi_data); ?>],
        color: '<?php echo isset($color_pm10_aqi) ? $color_pm10_aqi : "#CC6600"; ?>',
        visible: false
    }];

    <?php if (!empty($epa_result)) { ?>
    aqiSeries.push({
        name: 'O3 AQI (EPA)',
        data: [<?php echo implode(',', $o3_aqi_data); ?>],
        color: '<?php echo isset($color_o3) ? $color_o3 : "#9370DB"; ?>',
        dashStyle: 'ShortDot'
    });
    aqiSeries.push({
        name: 'NO2 AQI (EPA)',
        data: [<?php echo implode(',', $no2_aqi_data); ?>],
        color: '<?php echo isset($color_no2) ? $color_no2 : "#20B2AA"; ?>',
        dashStyle: 'ShortDot',
        visible: false
    });
    <?php } ?>

    var aqiChart = Highcharts.chart('aqi-chart', {
        chart: {
            zoomType: 'x',
            height: 300,
            marginRight: 10
        },
        title: { text: '' },
        xAxis: commonXAxis,
        yAxis: {
            title: { text: 'AQI' },
            min: 0,
            max: 300,
            plotBands: [
                { from: 0,   to: 50,  color: 'rgba(0,228,0,0.15)',     label: { text: 'Good',      align: 'right', x: -5, style: { fontSize: '11px', color: '#00A000' } } },
                { from: 50,  to: 100, color: 'rgba(255,255,0,0.12)',    label: { text: 'Moderate',  align: 'right', x: -5, style: { fontSize: '11px', color: '#B0B000' } } },
                { from: 100, to: 150, color: 'rgba(255,126,0,0.12)',    label: { text: 'USG',       align: 'right', x: -5, style: { fontSize: '11px', color: '#DD6600' } } },
                { from: 150, to: 200, color: 'rgba(255,0,0,0.10)',      label: { text: 'Unhealthy', align: 'right', x: -5, style: { fontSize: '11px', color: '#DD0000' } } },
                { from: 200, to: 300, color: 'rgba(143,63,151,0.10)',   label: { text: 'Very Unhealthy', align: 'right', x: -5, style: { fontSize: '10px', color: '#8F3F97' } } }
            ],
            plotLines: [{
                value: <?php echo $smoke_aqi_threshold; ?>,
                color: '#FF0000',
                width: 2,
                dashStyle: 'Dash',
                zIndex: 3,
                label: {
                    text: 'Smoke Threshold',
                    style: { fontSize: '10px', color: '#FF0000' },
                    align: 'left'
                }
            }, {
                value: 50,
                color: 'rgba(0,0,0,0.08)',
                width: 1,
                dashStyle: 'Dot'
            }, {
                value: 100,
                color: 'rgba(0,0,0,0.08)',
                width: 1,
                dashStyle: 'Dot'
            }]
        },
        plotOptions: {
            line: {
                marker: { enabled: true, radius: 1.5, symbol: 'circle' },
                lineWidth: 2
            }
        },
        tooltip: {
            shared: true,
            formatter: function () {
                var s = '<b>' + Highcharts.dateFormat('%m/%d %H:%M', this.x) + '</b>';
                this.points.forEach(function (point) {
                    s += '<br/><span style="color:' + point.series.color + '">●</span> ' + point.series.name + ': <b>' + point.y + '</b>';
                });
                return s;
            }
        },
        exporting: commonExporting('aqi'),
        series: aqiSeries
    });
    charts.push(aqiChart);

    // Sync crosshairs on mouse move
    ['pm-chart', 'aqi-chart'].forEach(function(id) {
        var el = document.getElementById(id);
        if (el) {
            ['mousemove', 'touchmove', 'touchstart'].forEach(function(eventType) {
                el.addEventListener(eventType, function(e) {
                    var chart = Highcharts.charts.filter(function(c) { return c && c.renderTo && c.renderTo.id === id; })[0];
                    if (chart) {
                        syncCrosshair(e, chart);
                    }
                });
            });
        }
    });
});
</script>
