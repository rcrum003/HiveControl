


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


    # Echo back the Javascript code
 
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Get Hive Data First
$sth = $conn->prepare("SELECT hiveweight, hiverawweight, hivetempf, hiveHum, weather_tempf, weather_humidity, precip_1hr_in, solarradiation, lux, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now','$sqlperiod')");
$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

echo "
<!-- Chart Code -->


<script>
$(function () {
    $('#allcontainer').highcharts({
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
           
    yAxis: [{ // Temp yAxis
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
            

        }, { // Rain yAxis
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

        },
        { // Weight yAxis
            gridLineWidth: 0,
            title: {
                text: 'Weight',
                style: {
                    color: Highcharts.getOptions().colors[6]
                }
            },
            labels: {
                format: '{value} lbs',
                style: {
                    color: Highcharts.getOptions().colors[6]
                }
            },
            opposite: false

        },
        { // Humidity yAxis
            gridLineWidth: 0,
            title: {
                text: 'Humidity',
                style: {
                    color: Highcharts.getOptions().colors[0]
                }
            },
            labels: {
                format: '{value} %',
                style: {
                    color: Highcharts.getOptions().colors[0]
                }
            },
            opposite: true

        },
        { // Solarradiation yAxis
            gridLineWidth: 0,
            title: {
                text: 'Solar',
                style: {
                    color: Highcharts.getOptions().colors[8]
                }
            },
            labels: {
                format: '{value} wm/2',
                style: {
                    color: Highcharts.getOptions().colors[8]
                }
            },
            opposite: true

        },
        { // Lux yAxis
            gridLineWidth: 0,
            title: {
                text: 'Lux',
                style: {
                    color: Highcharts.getOptions().colors[8]
                }
            },
            labels: {
                format: '{value} lx',
                style: {
                    color: Highcharts.getOptions().colors[8]
                }
            },
            opposite: true

        }
        ],
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
            yAxis: 0,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hivetempf']."]".", ";} echo "],
            color: Highcharts.getOptions().colors[2]
        },
        {
            type: 'line',
            name: 'Outside Temp (°F)',
            yAxis: 0,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['weather_tempf']."]".", ";} echo "],
            visible: false,
            color: '#808080'
        },
        {
            type: 'line',
            name: 'Hive Humidty (%)',
            yAxis: 3,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiveHum']."]".", ";} echo "],
            color: '#87CEFA',
            visible: true
        },
        {
            type: 'line',
            name: 'Outside Humidty (%)',
            yAxis: 3,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['weather_humidity']."]".", ";} echo "],
            visible: false
        },
        {
            type: 'column',
            yAxis: 1,
            name: 'Rain',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['precip_1hr_in']."]".", ";} echo "],
            visible: false
        },
        {
            type: 'line',
            yAxis: 2,
            name: 'Hive Weight Net (lbs)',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiveweight']."]".", ";} echo "], 
            color: '#FFD700',
            visible: true
        },
        {
           type: 'line',
           yAxis: 2,
           name: 'Hive Weight Gross (lbs)',
           data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiverawweight']."]".", ";} echo "],
           color: '#000000',
           visible: false
        },
        {
            type: 'line',
            name: 'Solar (wm/2)',
            yAxis: 4,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['solarradiation']."]".", ";} echo "],
            color: '#ff0000',
            visible: false
        },
        {
            type: 'line',
            name: 'Lux (lx)',
            yAxis: 5,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['lux']."]".", ";} echo "],
            color: '#ff6666',
            visible: false
        }
        ]
    });
        Highcharts.getOptions().exporting.buttons.contextButton.menuItems.push({
            text: 'Enlarge Chart',
            onclick: function () {
                centeredPopup('/pages/fullscreen/all.php?chart=line&period=";echo $period; echo"','HiveControl','1200','500','yes')
                return false;
            }
        });

});
</script>";



?>


 




