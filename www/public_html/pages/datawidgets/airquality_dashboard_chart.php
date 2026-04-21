<?php
include "chart_data_fetch.php";

echo "
<script>
$(function () {
    var chart = Highcharts.chart('aqcontainer', {
        chart: {
            zoomType: 'x',
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
                text: 'Concentration (ug/m3)',
                style: {
                    color: '#000000'
                }
            },
            labels: {
                format: '{value}',
                style: {
                    color: '#000000'
                }
            },
            minRange: 10,
            floor: 0
        }, {
            gridLineWidth: 0,
            title: {
                text: 'AQI',
                style: {
                    color: '#FF6347'
                }
            },
            labels: {
                format: '{value}',
                style: {
                    color: '#FF6347'
                }
            },
            plotBands: [
                { from: 0, to: 50, color: 'rgba(0,228,0,0.06)', label: { text: 'Good', style: { fontSize: '9px', color: '#00E400' } } },
                { from: 51, to: 100, color: 'rgba(255,255,0,0.06)', label: { text: 'Moderate', style: { fontSize: '9px', color: '#CCCC00' } } },
                { from: 101, to: 150, color: 'rgba(255,126,0,0.06)', label: { text: 'USG', style: { fontSize: '9px', color: '#FF7E00' } } },
                { from: 151, to: 500, color: 'rgba(255,0,0,0.06)' }
            ],
            min: 0,
            max: 300,
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
            name: 'PM2.5 (ug/m3)',
            yAxis: 0,
            data: ["; foreach($chart_result as $r){
                $val = is_numeric($r['air_pm2_5_raw']) ? floatval($r['air_pm2_5_raw']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '"; echo $color_pm2_5; echo "',
            visible: "; echo $trend_pm2_5; echo "
        },
        {
            type: 'line',
            name: 'PM10 (ug/m3)',
            yAxis: 0,
            data: ["; foreach($chart_result as $r){
                $val = is_numeric($r['air_pm10_raw']) ? floatval($r['air_pm10_raw']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '"; echo $color_pm10; echo "',
            visible: "; echo $trend_pm10; echo "
        },
        {
            type: 'line',
            name: 'PM1 (ug/m3)',
            yAxis: 0,
            data: ["; foreach($chart_result as $r){
                $val = is_numeric($r['air_pm1_raw']) ? floatval($r['air_pm1_raw']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '"; echo $color_pm1; echo "',
            visible: "; echo $trend_pm1; echo "
        },
        {
            type: 'line',
            yAxis: 1,
            name: 'PM2.5 AQI',
            data: ["; foreach($chart_result as $r){
                $val = is_numeric($r['air_pm2_5_aqi']) ? intval($r['air_pm2_5_aqi']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '"; echo $color_pm2_5_aqi; echo "',
            dashStyle: 'Dash',
            visible: "; echo $trend_pm2_5_aqi; echo "
        },
        {
            type: 'line',
            yAxis: 1,
            name: 'PM10 AQI',
            data: ["; foreach($chart_result as $r){
                $val = is_numeric($r['air_pm10_aqi']) ? intval($r['air_pm10_aqi']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '"; echo $color_pm10_aqi; echo "',
            dashStyle: 'Dash',
            visible: "; echo $trend_pm10_aqi; echo "
        }";

if (!empty($chart_result_epa)) {
echo ",
        {
            type: 'line',
            yAxis: 1,
            name: 'PM2.5 AQI (EPA)',
            data: ["; foreach($chart_result_epa as $r){
                $val = is_numeric($r['pm25_aqi']) ? intval($r['pm25_aqi']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#CC3333',
            dashStyle: 'ShortDot'
        },
        {
            type: 'line',
            yAxis: 1,
            name: 'PM10 AQI (EPA)',
            data: ["; foreach($chart_result_epa as $r){
                $val = is_numeric($r['pm10_aqi']) ? intval($r['pm10_aqi']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '#CC6600',
            dashStyle: 'ShortDot',
            visible: false
        },
        {
            type: 'line',
            yAxis: 1,
            name: 'O3 AQI (EPA)',
            data: ["; foreach($chart_result_epa as $r){
                $val = is_numeric($r['o3_aqi']) ? intval($r['o3_aqi']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '"; echo $color_o3; echo "',
            dashStyle: 'ShortDot'
        },
        {
            type: 'line',
            yAxis: 1,
            name: 'NO2 AQI (EPA)',
            data: ["; foreach($chart_result_epa as $r){
                $val = is_numeric($r['no2_aqi']) ? intval($r['no2_aqi']) : null;
                echo "[".$r['datetime'].", ".($val !== null ? $val : "null")."], ";
            } echo "],
            color: '"; echo $color_no2; echo "',
            dashStyle: 'ShortDot',
            visible: false
        }";
}

echo "
        ]
    });
});
</script>";
?>
