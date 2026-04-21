<?php
include "chart_data_fetch.php";

echo "
<script>
$(function () {
    var chart = Highcharts.chart('climatecontainer', {
        chart: {
            zoomType: 'xy',
            alignTicks: false
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
            gridLineWidth: 1,
            labels: {
                format: '{value} "; if ($SHOW_METRIC == "on") { echo "°C"; } else { echo "°F"; } echo "',
                style: {
                    color: '"; echo $color_hivetemp; echo "'
                }
            },
            title: {
                text: 'Temperature',
                style: {
                    color: '"; echo $color_hivetemp; echo "'
                }
            },
            minRange: 20,
            plotBands: [{
                from: "; echo ($SHOW_METRIC == "on") ? "38" : "100"; echo ",
                to: "; echo ($SHOW_METRIC == "on") ? "50" : "120"; echo ",
                color: 'rgba(255, 0, 0, 0.05)',
                label: {
                    text: 'High Temp Zone',
                    style: { color: '#999', fontSize: '10px' }
                }
            }]
        }, {
            gridLineWidth: 0,
            ceiling: 100,
            floor: 0,
            title: {
                text: 'Humidity',
                style: {
                    color: '"; echo $color_hivehum; echo "'
                }
            },
            labels: {
                format: '{value} %',
                style: {
                    color: '"; echo $color_hivehum; echo "'
                }
            },
            minRange: 20,
            opposite: true
        }],
        plotOptions: {";
        if ($chart_smoothing == "on") {
            echo "series: { connectNulls: true },";
        }
        echo "
            line: {
                marker: {
                    enabled: true,
                    radius: 2,
                    symbol: 'circle'
                },
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
                    menuItems: ['viewFullscreen', 'printChart', 'separator', 'downloadPNG', 'downloadJPEG', 'downloadPDF', 'downloadSVG']
                }
            }
        },
        series: [{
            type: 'line',
            name: 'Hive Temp ("; if ($SHOW_METRIC == "on") { echo "°C"; } else { echo "°F"; } echo ")',
            yAxis: 0,
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['hivetempf'])) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['hivetempf'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['hivetempf'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_hivetemp; echo "',
            visible: "; echo $trend_hivetemp; echo "
        },
        {
            type: 'line',
            name: 'Outside Temp ("; if ($SHOW_METRIC == "on") { echo "°C"; } else { echo "°F"; } echo ")',
            yAxis: 0,
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['weather_tempf'])) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['weather_tempf'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['weather_tempf'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_outtemp; echo "',
            visible: "; echo $trend_outtemp; echo "
        },
        {
            type: 'line',
            name: 'Hive Humidity (%)',
            yAxis: 1,
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['hiveHum']) && $r['hiveHum'] != 0) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['hiveHum'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['hiveHum'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_hivehum; echo "',
            visible: "; echo $trend_hivehum; echo "
        },
        {
            type: 'line',
            name: 'Outside Humidity (%)',
            yAxis: 1,
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['weather_humidity']) && $r['weather_humidity'] != 0) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['weather_humidity'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['weather_humidity'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_outhum; echo "',
            visible: "; echo $trend_outhum; echo "
        }
        ]
    });
});
</script>";
?>
