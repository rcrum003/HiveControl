<?php
include "chart_data_fetch.php";

echo "
<script>
$(function () {
    var chart = Highcharts.chart('envcontainer', {
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
            title: {
                text: 'Solar (W/m²)',
                style: {
                    color: '"; echo $color_solarradiation; echo "'
                }
            },
            labels: {
                format: '{value} W/m²',
                style: {
                    color: '"; echo $color_solarradiation; echo "'
                }
            },
            minRange: 100,
            showEmpty: false
        }, {
            gridLineWidth: 0,
            title: {
                text: 'Flights',
                style: {
                    color: '"; echo $color_beecount_out; echo "'
                }
            },
            labels: {
                format: '{value}',
                style: {
                    color: '"; echo $color_beecount_out; echo "'
                }
            },
            minRange: 20,
            showEmpty: false,
            opposite: true
        }, {
            gridLineWidth: 0,
            title: {
                text: 'Lux',
                style: {
                    color: '"; echo $color_lux; echo "'
                }
            },
            labels: {
                format: '{value} lx',
                style: {
                    color: '"; echo $color_lux; echo "'
                }
            },
            minRange: 100,
            showEmpty: false,
            visible: false
        }, {
            gridLineWidth: 0,
            title: {
                text: 'Wind',
                style: {
                    color: '"; echo $color_wind; echo "'
                }
            },
            labels: {
                format: '{value} "; if ($SHOW_METRIC == "on") { echo "kph"; } else { echo "mph"; } echo "',
                style: {
                    color: '"; echo $color_wind; echo "'
                }
            },
            showEmpty: false
        }, {
            gridLineWidth: 0,
            title: {
                text: 'Pressure',
                style: {
                    color: '"; echo $color_pressure; echo "'
                }
            },
            labels: {
                format: '{value} "; if ($SHOW_METRIC == "on") { echo "mb"; } else { echo "in"; } echo "',
                style: {
                    color: '"; echo $color_pressure; echo "'
                }
            },
            minRange: "; if ($SHOW_METRIC == "on") { echo "10"; } else { echo "1"; } echo ",
            showEmpty: false
        }, {
            gridLineWidth: 0,
            title: {
                text: 'GDD',
                style: {
                    color: '"; echo $color_gdd; echo "'
                }
            },
            labels: {
                format: '{value}',
                style: {
                    color: '"; echo $color_gdd; echo "'
                }
            },
            minRange: 100,
            showEmpty: false
        }, {
            gridLineWidth: 0,
            title: {
                text: 'Pollen',
                style: {
                    color: '"; echo $color_pollen; echo "'
                }
            },
            labels: {
                format: '{value}',
                style: {
                    color: '"; echo $color_pollen; echo "'
                }
            },
            showEmpty: false
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
            name: 'Solar (W/m²)',
            yAxis: 0,
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['solarradiation']) && $r['solarradiation'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['solarradiation'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_solarradiation; echo "',
            visible: "; echo $trend_solarradiation; echo "
        },
        {
            type: 'line',
            name: 'Flight Activity',
            yAxis: 1,
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['out_count']) && $r['out_count'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['out_count'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_beecount_out; echo "',
            visible: "; echo $trend_beecount_out; echo "
        },
        {
            type: 'line',
            name: 'Lux (lx)',
            yAxis: 2,
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['lux']) && $r['lux'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['lux'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_lux; echo "',
            visible: false
        },
        {
            type: 'line',
            name: 'Wind ("; if ($SHOW_METRIC == "on") { echo "kph"; } else { echo "mph"; } echo ")',
            yAxis: 3,
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['wind']) && $r['wind'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['wind'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_wind; echo "',
            visible: false
        },
        {
            type: 'line',
            name: 'Pressure ("; if ($SHOW_METRIC == "on") { echo "mb"; } else { echo "in"; } echo ")',
            yAxis: 4,
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['pressure']) && $r['pressure'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['pressure'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_pressure; echo "',
            visible: false
        },
        {
            type: 'line',
            name: 'GDD',
            yAxis: 5,
            data: ["; foreach($chart_result_gdd as $r){
                if (is_numeric($r['gdd']) && $r['gdd'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['gdd'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_gdd; echo "',
            visible: false
        },
        {
            type: 'line',
            name: 'Pollen',
            yAxis: 6,
            data: ["; foreach($chart_result_pollen as $r){
                if (is_numeric($r['pollenlevel']) && $r['pollenlevel'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['pollenlevel'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_pollen; echo "',
            visible: false
        }
        ]
    });
});
</script>";
?>
