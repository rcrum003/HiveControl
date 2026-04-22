<?php
$gauge_stores = is_numeric($hiveweight) ? floatval($hiveweight) : 0;
$gauge_equip = (is_numeric($rawweight) && is_numeric($hiveweight)) ? (floatval($rawweight) - floatval($hiveweight)) : 0;
$gauge_total = $gauge_stores + $gauge_equip;
$gauge_max = ($gauge_total > 0) ? ceil($gauge_total * 1.2 / 10) * 10 : 200;
$weight_label = ($SHOW_METRIC == "on") ? "kg" : "lb";
?>
<script>
$(function () {
    Highcharts.chart('weightgauge-container', {
        chart: { type: 'column' },
        legend: { enabled: false },
        title: { text: '' },
        xAxis: { categories: [''] },
        yAxis: {
            min: 0, max: <?php echo $gauge_max; ?>,
            title: { text: 'Total Weight (<?php echo $weight_label; ?>)' },
            stackLabels: { enabled: true, style: { fontWeight: 'bold' } }
        },
        tooltip: { enabled: false },
        plotOptions: {
            column: {
                stacking: 'normal',
                dataLabels: {
                    enabled: true, color: 'white', inside: true,
                    formatter: function() { return this.series.name + ': ' + Highcharts.numberFormat(this.y, 1) + ' <?php echo $weight_label; ?>'; },
                    style: { textShadow: '0 0 3px black' }
                }
            }
        },
        series: [{
            name: 'Stores', color: '#e6b800',
            data: [<?php echo $gauge_stores; ?>]
        }, {
            name: 'Equipment', color: '#000000',
            data: [<?php echo $gauge_equip; ?>]
        }]
    });
});
</script>