



<!-- Chart Code -->


<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart

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
    # Echo back the Javascript code

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Get Hive Data First

$sth = $conn->prepare("SELECT air_pm1, air_pm2_5, weather_tempf, air_aqi, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");

$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

echo "
<!-- Chart Code -->
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
                style: {
                    color: '#000000'
                }
            },
            title: {
                text: 'AIR ug/m3',
                style: {
                    color: '#000000'
                }
            },
            minRange: 50,
            ceiling: 500,
            floor: 0
        }, {
            gridLineWidth: 0,
            title: {
                text: 'Temp',
                style: {
                    color: '"; echo "$color_outtemp"; echo "'
                }
            },
            labels: {
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} echo "',
                style: {
                    color: '"; echo "$color_outtemp"; echo "'
                }
            },
            minRange: 20,
            opposite: true
        }],
        plotOptions: {
            line: {
                marker: {
                    enabled: true,
                    radius: 2,
                    symbol: 'circle'
                },
                dataLabels: {
                    enabled: false
                },
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
                            centeredPopup('/pages/fullscreen/weight.php?chart=line&period="; echo htmlspecialchars($period, ENT_QUOTES); echo "','HiveControl','1200','500','yes');
                            return false;
                        }
                    }]
                }
            }
        },
        series: [{
            type: 'line',
            name: 'PM1 (ug/m3)',
            data: ["; foreach($result as $r){
                if (is_numeric($r['air_pm1']) && $r['air_pm1'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['air_pm1'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_pm1"; echo "'
        },
        {
           type: 'line',
           name: 'PM2.5 (ug/m3)',
           data: ["; foreach($result as $r){
                if (is_numeric($r['air_pm2_5']) && $r['air_pm2_5'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['air_pm2_5'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
           color: '"; echo "$color_pm25"; echo "',
           visible: true
        },
        {
            type: 'line',
            name: 'AQI',
            data: ["; foreach($result as $r){
                if (is_numeric($r['air_aqi']) && $r['air_aqi'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['air_aqi'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_aqi"; echo "'
        },
        {
            type: 'line',
            yAxis: 1,
            name: 'Temp ("; if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} echo ")',
            data: ["; foreach($result as $r){
                if (is_numeric($r['weather_tempf']) && $r['weather_tempf'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['weather_tempf'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_outtemp"; echo "',
            visible: true
        }
        ]
    });
});
</script>";


} elseif ($chart == 'candle') {
   include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Get Hive Data First

if ( $SHOW_METRIC == "on" ) {

$sth = $conn->prepare("SELECT round((hiveweight * 0.453592),2) as hiveweight, round((hiverawweight * 0.453592),2) as hiverawweight, precip_1hr_metric as precip_1hr_in, wind_kph as wind, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
} else {

//Show normal
$sth = $conn->prepare("SELECT air_pm1, air_pm2_5,weather_tempf,air_aqi,strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
}

$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

echo "
<!-- Chart Code -->
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
                format: '{value} ppm',
                style: {
                    color: '"; echo "$color_pm1"; echo "'
                }
            },
            title: {
                text: 'Air Quality',
                style: {
                    color: '"; echo "$color_pm1"; echo "'
                }
            },
            minRange: 50,
            ceiling: 500,
            floor: 0
        }, {
            gridLineWidth: 0,
            title: {
                text: 'AQI',
                style: {
                    color: '"; echo "$color_aqi"; echo "'
                }
            },
            labels: {
                format: '{value}',
                style: {
                    color: '"; echo "$color_aqi"; echo "'
                }
            },
            opposite: true
        },
        {
            gridLineWidth: 1,
            title: {
                text: 'Wind',
                style: {
                    color: '"; echo "$color_pm25"; echo "'
                }
            },
            labels: {
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "kph";} else {echo "mph";} echo "',
                style: {
                    color: '"; echo "$color_pm25"; echo "'
                }
            },
            showEmpty: false,
            opposite: false
        }],
        plotOptions: {
            line: {
                marker: {
                    enabled: true,
                    radius: 2,
                    symbol: 'circle'
                },
                dataLabels: {
                    enabled: false
                },
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
            name: 'Air PM1 (ug/m3)',
            data: ["; foreach($result as $r){
                if (is_numeric($r['air_pm1']) && $r['air_pm1'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['air_pm1'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_pm1"; echo "'
        },
        {
           type: 'line',
           name: 'Air PM2.5 (ug/m3)',
           data: ["; foreach($result as $r){
                if (is_numeric($r['air_pm2_5']) && $r['air_pm2_5'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['air_pm2_5'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
           color: '"; echo "$color_pm25"; echo "',
           visible: true
        },
        {
           type: 'line',
           name: 'AQI',
           data: ["; foreach($result as $r){
                if (is_numeric($r['air_aqi']) && $r['air_aqi'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['air_aqi'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
           color: '"; echo "$color_aqi"; echo "',
           visible: true
        }
        ]
    });
});
</script>";
}



?>







