
<!-- Air Quality Chart -->

<?php
switch ($period) {
    case "today":
        $sqlperiod = "start of day";
        break;
    case "day":
        $sqlperiod = "-1 days";
        break;
    case "week":
        $sqlperiod = "-7 days";
        break;
    case "month":
        $sqlperiod = "-1 months";
        break;
    case "year":
        $sqlperiod =  "-1 years";
        break;
    case "all":
        $sqlperiod =  "-20 years";
        break;
    }


if ($chart == 'line') {

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sth = $conn->prepare("SELECT air_pm1, air_pm2_5, air_pm10, air_pm2_5_raw, air_pm10_raw, air_pm1_raw, air_pm2_5_aqi, air_pm10_aqi, air_aqi, air_pressure, weather_tempf, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");

$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

// EPA data from airquality_epa table
$epa_sth = $conn->prepare("SELECT o3_aqi, no2_aqi, pm25_aqi, strftime('%s',date)*1000 AS datetime FROM airquality_epa WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
$epa_sth->execute();
$epa_result = $epa_sth->fetchAll(PDO::FETCH_ASSOC);

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

echo "
<script>
$(function () {
    var chart = Highcharts.chart('container', {
        chart: {
            zoomType: 'x'
        },
        title: {
            text: ''
        },
        xAxis: {
            type: 'datetime',
            dateTimeLabelFormats: {
                second: '%m-%d<br/>%H:%M:%S',
                minute: '%m-%d<br/>%H:%M',
                hour: '%m-%d<br/>%H:%M',
                day: '<br/>%m-%d',
                week: '<br/>%m-%d',
                month: '%Y-%m',
                year: '%Y'
            }
        },
        yAxis: [{
            labels: {
                style: { color: '#000000' }
            },
            title: {
                text: 'Concentration (ug/m3)',
                style: { color: '#000000' }
            },
            minRange: 20,
            floor: 0
        }, {
            gridLineWidth: 0,
            title: {
                text: 'AQI',
                style: { color: '#FF6347' }
            },
            labels: {
                format: '{value}',
                style: { color: '#FF6347' }
            },
            plotBands: [
                { from: 0, to: 50, color: 'rgba(0,228,0,0.08)', label: { text: 'Good', style: { fontSize: '9px', color: '#00E400' } } },
                { from: 51, to: 100, color: 'rgba(255,255,0,0.08)', label: { text: 'Moderate', style: { fontSize: '9px', color: '#CCCC00' } } },
                { from: 101, to: 150, color: 'rgba(255,126,0,0.08)', label: { text: 'USG', style: { fontSize: '9px', color: '#FF7E00' } } },
                { from: 151, to: 200, color: 'rgba(255,0,0,0.08)', label: { text: 'Unhealthy', style: { fontSize: '9px', color: '#FF0000' } } },
                { from: 201, to: 300, color: 'rgba(143,63,151,0.08)' },
                { from: 301, to: 500, color: 'rgba(126,0,35,0.08)' }
            ],
            min: 0,
            max: 300,
            opposite: true
        }],
        plotOptions: {
            line: {
                marker: { enabled: true, radius: 2, symbol: 'circle' },
                dataLabels: { enabled: false },
                enableMouseTracking: true
            }
        },
        tooltip: {
            formatter: function () {
                var s = '<b>' + Highcharts.dateFormat('%m/%d %H:%M', this.x) + '</b>';
                this.points.forEach(function (point) {
                    s += '<br/>' + point.series.name + ': ' + point.y;
                });
                return s;
            },
            shared: true
        },
        exporting: {
            buttons: {
                contextButton: {
                    menuItems: ['viewFullscreen', 'printChart', 'separator', 'downloadPNG', 'downloadJPEG', 'downloadPDF', 'downloadSVG', 'separator', {
                        text: 'Enlarge Chart',
                        onclick: function () {
                            centeredPopup('/pages/fullscreen/air.php?chart=line&period="; echo htmlspecialchars($period, ENT_QUOTES); echo "','HiveControl','1200','500','yes');
                            return false;
                        }
                    }]
                }
            }
        },
        series: [{
            type: 'line',
            name: 'PM2.5 (ug/m3)',
            data: ["; foreach($result as $r){
                $val = is_numeric($r['air_pm2_5_raw']) ? floatval($r['air_pm2_5_raw']) : (is_numeric($r['air_pm2_5']) ? floatval($r['air_pm2_5']) : null);
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#FF6347'
        },
        {
            type: 'line',
            name: 'PM10 (ug/m3)',
            data: ["; foreach($result as $r){
                $val = is_numeric($r['air_pm10_raw']) ? floatval($r['air_pm10_raw']) : (is_numeric($r['air_pm10']) ? floatval($r['air_pm10']) : null);
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#FF8C00'
        },
        {
            type: 'line',
            name: 'PM1 (ug/m3)',
            data: ["; foreach($result as $r){
                $val = is_numeric($r['air_pm1_raw']) ? floatval($r['air_pm1_raw']) : (is_numeric($r['air_pm1']) ? floatval($r['air_pm1']) : null);
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#32CD32',
            visible: false
        },
        {
            type: 'line',
            yAxis: 1,
            name: 'PM2.5 AQI',
            data: ["; foreach($result as $r){
                $val = is_numeric($r['air_pm2_5_aqi']) ? intval($r['air_pm2_5_aqi']) : (is_numeric($r['air_aqi']) ? intval($r['air_aqi']) : null);
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#CC3333',
            dashStyle: 'Dash'
        },
        {
            type: 'line',
            yAxis: 1,
            name: 'PM10 AQI',
            data: ["; foreach($result as $r){
                $val = is_numeric($r['air_pm10_aqi']) ? intval($r['air_pm10_aqi']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#CC6600',
            dashStyle: 'Dash',
            visible: false
        }";

// Add O3 and NO2 AQI series from EPA data if available
if (!empty($epa_result)) {
echo ",
        {
            type: 'line',
            yAxis: 1,
            name: 'O3 AQI (EPA)',
            data: ["; foreach($epa_result as $r){
                $val = is_numeric($r['o3_aqi']) ? intval($r['o3_aqi']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#9370DB',
            dashStyle: 'ShortDot'
        },
        {
            type: 'line',
            yAxis: 1,
            name: 'NO2 AQI (EPA)',
            data: ["; foreach($epa_result as $r){
                $val = is_numeric($r['no2_aqi']) ? intval($r['no2_aqi']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#20B2AA',
            dashStyle: 'ShortDot',
            visible: false
        }";
}

echo "
        ]
    });
});
</script>";


} elseif ($chart == 'candle') {
   include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sth = $conn->prepare("SELECT air_pm1, air_pm2_5, air_pm10, air_pm2_5_raw, air_pm10_raw, air_pm2_5_aqi, air_pm10_aqi, air_aqi, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");

$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

echo "
<script>
$(function () {
    var chart = Highcharts.chart('container', {
        chart: {
            zoomType: 'x'
        },
        title: {
            text: ''
        },
        xAxis: {
            type: 'datetime',
            dateTimeLabelFormats: {
                second: '%m-%d<br/>%H:%M:%S',
                minute: '%m-%d<br/>%H:%M',
                hour: '%m-%d<br/>%H:%M',
                day: '<br/>%m-%d',
                week: '<br/>%m-%d',
                month: '%Y-%m',
                year: '%Y'
            }
        },
        yAxis: [{
            labels: {
                format: '{value} ug/m3',
                style: { color: '#FF6347' }
            },
            title: {
                text: 'Concentration',
                style: { color: '#FF6347' }
            },
            minRange: 20,
            floor: 0
        }, {
            gridLineWidth: 0,
            title: {
                text: 'AQI',
                style: { color: '#CC3333' }
            },
            labels: {
                format: '{value}',
                style: { color: '#CC3333' }
            },
            opposite: true
        }],
        plotOptions: {
            line: {
                marker: { enabled: true, radius: 2, symbol: 'circle' },
                dataLabels: { enabled: false },
                enableMouseTracking: true
            }
        },
        tooltip: {
            formatter: function () {
                var s = '<b>' + Highcharts.dateFormat('%m/%d %H:%M', this.x) + '</b>';
                this.points.forEach(function (point) {
                    s += '<br/>' + point.series.name + ': ' + point.y;
                });
                return s;
            },
            shared: true
        },
        exporting: {
            buttons: {
                contextButton: {
                    menuItems: ['viewFullscreen', 'printChart', 'separator', 'downloadPNG', 'downloadJPEG', 'downloadPDF', 'downloadSVG', 'separator', {
                        text: 'Enlarge Chart',
                        onclick: function () {
                            centeredPopup('/pages/fullscreen/air.php?chart=line&period="; echo htmlspecialchars($period, ENT_QUOTES); echo "','HiveControl','1200','500','yes');
                            return false;
                        }
                    }]
                }
            }
        },
        series: [{
            type: 'line',
            name: 'PM2.5 (ug/m3)',
            data: ["; foreach($result as $r){
                $val = is_numeric($r['air_pm2_5_raw']) ? floatval($r['air_pm2_5_raw']) : (is_numeric($r['air_pm2_5']) ? floatval($r['air_pm2_5']) : null);
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#FF6347'
        },
        {
            type: 'line',
            name: 'PM10 (ug/m3)',
            data: ["; foreach($result as $r){
                $val = is_numeric($r['air_pm10_raw']) ? floatval($r['air_pm10_raw']) : (is_numeric($r['air_pm10']) ? floatval($r['air_pm10']) : null);
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#FF8C00'
        },
        {
            type: 'line',
            name: 'PM1 (ug/m3)',
            data: ["; foreach($result as $r){
                $val = is_numeric($r['air_pm1_raw']) ? floatval($r['air_pm1_raw']) : (is_numeric($r['air_pm1']) ? floatval($r['air_pm1']) : null);
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#32CD32',
            visible: false
        },
        {
            type: 'line',
            yAxis: 1,
            name: 'PM2.5 AQI',
            data: ["; foreach($result as $r){
                $val = is_numeric($r['air_pm2_5_aqi']) ? intval($r['air_pm2_5_aqi']) : (is_numeric($r['air_aqi']) ? intval($r['air_aqi']) : null);
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#CC3333',
            dashStyle: 'Dash'
        }
        ]
    });
});
</script>";
}

?>
