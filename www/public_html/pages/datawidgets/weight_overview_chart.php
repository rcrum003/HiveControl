<?php
include "chart_data_fetch.php";

// Check for weight-related alerts to add chart annotations
include_once($_SERVER["DOCUMENT_ROOT"] . "/include/alert_engine.php");
$weight_alerts = get_active_alerts($conn);

// Get the latest weight value for plotLine placement
$latest_weight_sth = $conn->prepare("SELECT hiveweight FROM allhivedata WHERE hiveweight > 0 ORDER BY datetime(date) DESC LIMIT 1");
$latest_weight_sth->execute();
$latest_weight_row = $latest_weight_sth->fetch(PDO::FETCH_ASSOC);
$current_weight_val = $latest_weight_row ? floatval($latest_weight_row['hiveweight']) : 0;
if ($SHOW_METRIC == "on" && $current_weight_val > 0) {
    $current_weight_val = round($current_weight_val * 0.453592, 2);
}

echo "
<script>
$(function () {
    var chart = Highcharts.chart('weightcontainer', {
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
                text: 'Weight',
                style: {
                    color: '"; echo $color_netweight; echo "'
                }
            },
            labels: {
                format: '{value} "; if ($SHOW_METRIC == "on") { echo "kg"; } else { echo "lb"; } echo "',
                style: {
                    color: '"; echo $color_netweight; echo "'
                }
            },
            minRange: 10,
            showEmpty: false";
            if (!empty($weight_alerts)) {
                echo ",
            plotLines: [";
                foreach ($weight_alerts as $wa) {
                    if ($wa['type'] === 'robbing' || $wa['type'] === 'swarm') {
                        $line_color = ($wa['type'] === 'swarm') ? '#cc0000' : '#ff6600';
                        $line_label = json_encode($wa['title']);
                        echo "{
                            value: {$current_weight_val},
                            color: '{$line_color}',
                            dashStyle: 'dash',
                            width: 2,
                            label: {
                                text: {$line_label},
                                style: { color: '{$line_color}', fontWeight: 'bold', fontSize: '11px' }
                            }
                        },";
                    }
                }
                echo "]";
            }
            echo "
        }, {
            gridLineWidth: 0,
            title: {
                text: 'Rain',
                style: {
                    color: '"; echo $color_rain; echo "'
                }
            },
            labels: {
                format: '{value} "; if ($SHOW_METRIC == "on") { echo "mm"; } else { echo "in"; } echo "',
                style: {
                    color: '"; echo $color_rain; echo "'
                }
            },
            showEmpty: false,
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
            yAxis: 0,
            name: 'Hive Weight Net ("; if ($SHOW_METRIC == "on") { echo "kg"; } else { echo "lb"; } echo ")',
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['hiveweight']) && $r['hiveweight'] != 0) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['hiveweight'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['hiveweight'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_netweight; echo "',
            visible: "; echo $trend_netweight; echo "
        },
        {
            type: 'line',
            yAxis: 0,
            name: 'Hive Weight Gross ("; if ($SHOW_METRIC == "on") { echo "kg"; } else { echo "lb"; } echo ")',
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['hiverawweight']) && $r['hiverawweight'] != 0) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['hiverawweight'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['hiverawweight'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_grossweight; echo "',
            visible: "; echo $trend_grossweight; echo "
        },
        {
            type: 'area',
            yAxis: 1,
            name: 'Rain ("; if ($SHOW_METRIC == "on") { echo "mm"; } else { echo "in"; } echo ")',
            data: ["; foreach($chart_result as $r){
                if (is_numeric($r['precip_1hr_in']) && $r['precip_1hr_in'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['precip_1hr_in'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo $color_rain; echo "',
            visible: "; echo $trend_rain; echo "
        }
        ]
    });
});
</script>";
?>
