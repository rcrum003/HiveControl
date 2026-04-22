<?php
include_once "dashboard_chart_shared.php";

include_once($_SERVER["DOCUMENT_ROOT"] . "/include/alert_engine.php");
$weight_alerts = get_active_alerts($conn);

$weight_unit = ($SHOW_METRIC == "on") ? "kg" : "lb";
$rain_unit = ($SHOW_METRIC == "on") ? "mm" : "in";

$latest_weight_sth = $conn->prepare("SELECT hiveweight FROM allhivedata WHERE hiveweight > 0 ORDER BY datetime(date) DESC LIMIT 1");
$latest_weight_sth->execute();
$latest_weight_row = $latest_weight_sth->fetch(PDO::FETCH_ASSOC);
$current_weight_val = $latest_weight_row ? floatval($latest_weight_row['hiveweight']) : 0;
if ($SHOW_METRIC == "on" && $current_weight_val > 0) {
    $current_weight_val = round($current_weight_val * 0.453592, 2);
}

$weight_plotlines = [];
foreach ($weight_alerts as $wa) {
    if ($wa['type'] === 'swarm') {
        $weight_plotlines[] = '{ value: ' . $current_weight_val . ', color: "#cc0000", dashStyle: "Dash", width: 2, label: { text: ' . json_encode($wa['title']) . ', style: { color: "#cc0000", fontWeight: "bold", fontSize: "11px" } } }';
    } elseif ($wa['type'] === 'robbing') {
        $weight_plotlines[] = '{ value: ' . $current_weight_val . ', color: "#ff6600", dashStyle: "Dash", width: 2, label: { text: ' . json_encode($wa['title']) . ', style: { color: "#ff6600", fontWeight: "bold", fontSize: "11px" } } }';
    }
}
?>

<script>
$(function () {
    var chart = Highcharts.chart('weightcontainer', {
        chart: { zoomType: 'x', height: 280 },
        title: { text: '' },
        xAxis: hcCommonXAxis,
        yAxis: [{
            gridLineWidth: 1,
            title: { text: 'Weight', style: { color: '<?php echo $color_netweight; ?>' } },
            labels: { format: '{value} <?php echo $weight_unit; ?>', style: { color: '<?php echo $color_netweight; ?>' } },
            minRange: 10,
            showEmpty: false
            <?php if (!empty($weight_plotlines)) echo ', plotLines: [' . implode(',', $weight_plotlines) . ']'; ?>
        }, {
            gridLineWidth: 0,
            title: { text: 'Rain', style: { color: '<?php echo $color_rain; ?>' } },
            labels: { format: '{value} <?php echo $rain_unit; ?>', style: { color: '<?php echo $color_rain; ?>' } },
            showEmpty: false,
            opposite: true
        }],
        plotOptions: {
            <?php if ($chart_smoothing == "on") echo "series: { connectNulls: true },"; ?>
            line: { marker: { enabled: <?php echo $show_markers; ?>, radius: 1.5, symbol: 'circle' }, lineWidth: 2 },
            area: { fillOpacity: 0.3 }
        },
        tooltip: hcCommonTooltip,
        exporting: hcCommonExporting,
        series: [{
            type: 'line', yAxis: 0,
            name: 'Net Weight (<?php echo $weight_unit; ?>)',
            data: [<?php echo implode(',', $data_netweight); ?>],
            color: '<?php echo $color_netweight; ?>',
            visible: <?php echo $trend_netweight; ?>
        }, {
            type: 'line', yAxis: 0,
            name: 'Gross Weight (<?php echo $weight_unit; ?>)',
            data: [<?php echo implode(',', $data_grossweight); ?>],
            color: '<?php echo $color_grossweight; ?>',
            visible: <?php echo $trend_grossweight; ?>
        }, {
            type: 'area', yAxis: 1,
            name: 'Rain (<?php echo $rain_unit; ?>)',
            data: [<?php echo implode(',', $data_rain); ?>],
            color: '<?php echo $color_rain; ?>',
            visible: <?php echo $trend_rain; ?>
        }]
    });
    dashboardCharts.push(chart);
});
</script>
