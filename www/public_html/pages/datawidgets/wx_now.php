
<?PHP 


include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sth = $conn->prepare("SELECT hivetempf, hivetempc, hiveHum, hiveweight, strftime('%s',date)*1000 AS datetime, date FROM hivedata WHERE date > datetime('now','-7 days')");
$sth->execute();


$result = $sth->fetchAll(PDO::FETCH_ASSOC);

?>


<!-- Chart Code -->


<script>
$(function () {
    $('#pastweekcontainer').highcharts({
        chart: {
            type: 'line',
            zoomType: 'x'
        },
        title: {
            text: '', 
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

        rangeSelector: {
                allButtonsEnabled: true,
                selected: 2
            },
           
        yAxis: {
            title: {
                text: 'Values'
            }
        },
        plotOptions: {
            line: {
                dataLabels: {
                    enabled: false
                },
                enableMouseTracking: true
            }
        },
        tooltip: {
            formatter: function () {
                var s = '<b>' + Highcharts.dateFormat('%m/%d %H:%M', this.x) + '</b>';

                $.each(this.points, function () {
                    s += '<br/>' + this.series.name + ': ' +
                        this.y;
                });

                return s;
            },
            shared: true

        },
        series: [{
            name: 'Temp (°F)',
            data: [<?php foreach($result as $r){echo "[".$r['datetime'].", ".$r['hivetempf']."]".", ";} ?>],
            dashStyle: 'longdash',
            color: '#5cb85c'
        },
        {
            name: 'Weight (lbs)',
            data: [<?php foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiveweight']."]".", ";} ?>],
            visible: true,
            color: '#e6b800'
        },
        {
            name: 'Humidty (%)',
            color: '#87CEFA',
           data: [<?php foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiveHum']."]".", ";} ?>],
        }
        ]
    });
});
</script>