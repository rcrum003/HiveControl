<?php
include_once "dashboard_chart_shared.php";

$temp_unit = ($SHOW_METRIC == "on") ? "°C" : "°F";
$brood_low = ($SHOW_METRIC == "on") ? 34 : 93;
$brood_high = ($SHOW_METRIC == "on") ? 36 : 97;
$danger_from = ($SHOW_METRIC == "on") ? 38 : 100;
$danger_to = ($SHOW_METRIC == "on") ? 50 : 120;
?>

<script>
$(function () {
    var chart = Highcharts.chart('climatecontainer', {
        chart: { zoomType: 'x', height: 280 },
        title: { text: '' },
        xAxis: hcCommonXAxis,
        yAxis: [{
            gridLineWidth: 1,
            labels: { format: '{value} <?php echo $temp_unit; ?>', style: { color: '<?php echo $color_hivetemp; ?>' } },
            title: { text: 'Temperature', style: { color: '<?php echo $color_hivetemp; ?>' } },
            minRange: 20,
            plotBands: [{
                from: <?php echo $brood_low; ?>, to: <?php echo $brood_high; ?>,
                color: 'rgba(0,180,0,0.08)',
                label: { text: 'Brood Zone', align: 'right', x: -5, style: { fontSize: '10px', color: '#00A000' } }
            }, {
                from: <?php echo $danger_from; ?>, to: <?php echo $danger_to; ?>,
                color: 'rgba(255,0,0,0.06)',
                label: { text: 'Danger', align: 'right', x: -5, style: { fontSize: '10px', color: '#CC0000' } }
            }]
        }, {
            gridLineWidth: 0,
            ceiling: 100, floor: 0,
            title: { text: 'Humidity', style: { color: '<?php echo $color_hivehum; ?>' } },
            labels: { format: '{value} %', style: { color: '<?php echo $color_hivehum; ?>' } },
            opposite: true
        }],
        plotOptions: {
            <?php if ($chart_smoothing == "on") echo "series: { connectNulls: true },"; ?>
            line: { marker: { enabled: <?php echo $show_markers; ?>, radius: 1.5, symbol: 'circle' }, lineWidth: 2 }
        },
        tooltip: hcCommonTooltip,
        exporting: hcCommonExporting,
        series: [{
            name: 'Hive Temp (<?php echo $temp_unit; ?>)', yAxis: 0,
            data: [<?php echo implode(',', $data_hivetemp); ?>],
            color: '<?php echo $color_hivetemp; ?>',
            visible: <?php echo $trend_hivetemp; ?>
        }, {
            name: 'Outside Temp (<?php echo $temp_unit; ?>)', yAxis: 0,
            data: [<?php echo implode(',', $data_outtemp); ?>],
            color: '<?php echo $color_outtemp; ?>',
            visible: <?php echo $trend_outtemp; ?>
        }, {
            name: 'Hive Humidity (%)', yAxis: 1,
            data: [<?php echo implode(',', $data_hivehum); ?>],
            color: '<?php echo $color_hivehum; ?>',
            visible: <?php echo $trend_hivehum; ?>
        }, {
            name: 'Outside Humidity (%)', yAxis: 1,
            data: [<?php echo implode(',', $data_outhum); ?>],
            color: '<?php echo $color_outhum; ?>',
            visible: <?php echo $trend_outhum; ?>
        }]
    });
    dashboardCharts.push(chart);
});
</script>
