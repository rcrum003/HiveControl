
<script>
$(function () {
    $('#weightgauge-container').highcharts({
        chart: {
            type: 'column'
        },
        legend: {
            enabled: false
        },
        title: {
            text: ''
        },
        xAxis: {
            categories: ['' ]
        },
        yAxis: {
            min: 0,
            max: 200,
            title: {
                text: 'Total Weight (<?PHP if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} ?>)'
            },
            stackLabels: {
                enabled: true,
                style: {
                    fontWeight: 'bold',
                }
            }
        },
        tooltip: {
            enabled: false
        },
        plotOptions: {
            column: {
                stacking: 'normal',
                dataLabels: {
                    enabled: true,
                    color: (Highcharts.theme && Highcharts.theme.dataLabelsColor) || 'white',
                    formatter: function() {return this.series.name + ': ' + this.y + ' <?PHP if ( $SHOW_METRIC == "on" ) { echo "kgs";} else {echo "lbs";} ?>'},
                    inside: true,
                    style: {
                        textShadow: '0 0 3px black'
                    }
                }
            }
        },
        series: [{
            name: 'Stores',
            color: '#e6b800',
            data: [ <?php echo $hiveweight; ?> ]
        }, {
            name: 'Equipment',
            color: '#000000',
            data: [ <?php echo ($rawweight - $hiveweight); ?> ]
        }]
    });
});
</script>