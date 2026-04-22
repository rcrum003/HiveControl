<?php
include_once "dashboard_chart_shared.php";

$wind_unit = ($SHOW_METRIC == "on") ? "kph" : "mph";
$high_wind = ($SHOW_METRIC == "on") ? 32 : 20;
?>

<script>
$(function () {
    var chart = Highcharts.chart('foragingcontainer', {
        chart: { zoomType: 'x', height: 220 },
        title: { text: '' },
        xAxis: hcCommonXAxis,
        yAxis: [{
            gridLineWidth: 1,
            title: { text: 'Flights', style: { color: '<?php echo $color_beecount_out; ?>' } },
            labels: { format: '{value}', style: { color: '<?php echo $color_beecount_out; ?>' } },
            minRange: 20, floor: 0, showEmpty: false
        }, {
            gridLineWidth: 0,
            title: { text: 'Wind (<?php echo $wind_unit; ?>)', style: { color: '<?php echo $color_wind; ?>' } },
            labels: { format: '{value} <?php echo $wind_unit; ?>', style: { color: '<?php echo $color_wind; ?>' } },
            floor: 0, showEmpty: false, opposite: true,
            plotBands: [{
                from: <?php echo $high_wind; ?>, to: 200,
                color: 'rgba(255,0,0,0.06)',
                label: { text: 'High Wind', align: 'right', x: -5, style: { fontSize: '10px', color: '#CC0000' } }
            }]
        }],
        plotOptions: {
            <?php if ($chart_smoothing == "on") echo "series: { connectNulls: true },"; ?>
            line: { marker: { enabled: <?php echo $show_markers; ?>, radius: 1.5, symbol: 'circle' }, lineWidth: 2 }
        },
        tooltip: hcCommonTooltip,
        exporting: hcCommonExporting,
        series: [{
            name: 'Flight Activity', yAxis: 0,
            data: [<?php echo implode(',', $data_flights); ?>],
            color: '<?php echo $color_beecount_out; ?>',
            visible: <?php echo $trend_beecount_out; ?>
        }, {
            name: 'Wind (<?php echo $wind_unit; ?>)', yAxis: 1,
            data: [<?php echo implode(',', $data_wind); ?>],
            color: '<?php echo $color_wind; ?>',
            visible: <?php echo $trend_wind; ?>
        }]
    });
    dashboardCharts.push(chart);
});
</script>
