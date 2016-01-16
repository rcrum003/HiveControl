


<!-- Chart Code -->


<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart

switch ($period) {
    case "day":
        $sqlperiod = "-1 days";
        break;
    case "week":
        $sqlperiod = "-7 days";
        break;
    case "month":
        $sqlperiod = "-1 months";
        break;
    case "year":
        $sqlperiod =  "-1 years";
        break;
    case "all":
        $sqlperiod =  "-20 years";
        break;
    }


if ($chart == 'line') {
    # Echo back the Javascript code
 
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Get Hive Data First
$sth = $conn->prepare("SELECT hivetempf, hiveHum, weather_tempf, weather_humidity, precip_1hr_in, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now','$sqlperiod')");
$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

if ( $period  == 'day') {
    // Get Percip by Hour instead of day
 
} else {
    // Get Percip by Day/Hour
}

echo "
<!-- Chart Code -->


<script>
$(function () {
    $('#container').highcharts({
        chart: {
            zoomType: 'xy'
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
           
    yAxis: [{ // Primary yAxis
            labels: {
                format: '{value}°F',
                style: {
                    color: Highcharts.getOptions().colors[2]
                }
            },
            title: {
                text: 'Temperature',
                style: {
                    color: Highcharts.getOptions().colors[2]
                }
            }
            

        }, { // Secondary yAxis
            gridLineWidth: 0,
            title: {
                text: 'Rain',
                style: {
                    color: Highcharts.getOptions().colors[0]
                }
            },
            labels: {
                format: '{value} in',
                style: {
                    color: Highcharts.getOptions().colors[0]
                }
            },
            opposite: true

        }],
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
            type: 'line',
            name: 'Hive Temp (°F)',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hivetempf']."]".", ";} echo "],
            dashStyle: 'longdash', 
            color: '#FFD700',
        },
        {
            type: 'line',
            name: 'Outside Temp (°F)',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['weather_tempf']."]".", ";} echo "],
            visible: true,
            color: '#808080',
        },
        {
            type: 'line',
            name: 'Hive Humidty (%)',
           data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiveHum']."]".", ";} echo "],
            color: '#87CEFA',
            dashStyle: 'longdash',
           visible: false
        },
        {
            type: 'line',
            name: 'Outside Humidty (%)',
           data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['weather_humidity']."]".", ";} echo "],
           visible: false
        },
        {
            type: 'column',
            yAxis: 1,
            name: 'Rain',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['precip_1hr_in']."]".", ";} echo "],
            visible: true
        }
        ]
    });
});
</script>";


} elseif ($chart == 'bar') {
    # code...
}



?>


 




