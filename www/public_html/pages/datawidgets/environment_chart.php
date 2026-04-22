<?php
include_once "dashboard_chart_shared.php";
?>

<script>
$(function () {
    var chart = Highcharts.chart('envcontainer', {
        chart: { zoomType: 'x', height: 220 },
        title: { text: '' },
        xAxis: hcCommonXAxis,
        yAxis: [{
            gridLineWidth: 1,
            title: { text: 'Solar (W/m²)', style: { color: '<?php echo $color_solarradiation; ?>' } },
            labels: { format: '{value} W/m²', style: { color: '<?php echo $color_solarradiation; ?>' } },
            minRange: 100, floor: 0, showEmpty: false
        }, {
            gridLineWidth: 0,
            title: { text: 'Lux', style: { color: '<?php echo $color_lux; ?>' } },
            labels: { format: '{value} lx', style: { color: '<?php echo $color_lux; ?>' } },
            minRange: 100, showEmpty: false, opposite: true
        }],
        plotOptions: {
            <?php if ($chart_smoothing == "on") echo "series: { connectNulls: true },"; ?>
            line: { marker: { enabled: <?php echo $show_markers; ?>, radius: 1.5, symbol: 'circle' }, lineWidth: 2 }
        },
        tooltip: hcCommonTooltip,
        exporting: hcCommonExporting,
        series: [{
            name: 'Solar (W/m²)', yAxis: 0,
            data: [<?php echo implode(',', $data_solar); ?>],
            color: '<?php echo $color_solarradiation; ?>',
            visible: <?php echo $trend_solarradiation; ?>
        }, {
            name: 'Lux (lx)', yAxis: 1,
            data: [<?php echo implode(',', $data_lux); ?>],
            color: '<?php echo $color_lux; ?>',
            visible: false
        }]
    });
    dashboardCharts.push(chart);
});
</script>
