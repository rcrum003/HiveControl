


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

$sth1 = $conn->prepare("SELECT seasongdd AS gdd, strftime('%s',calcdate)*1000 AS datetime FROM gdd WHERE calcdate > datetime('now','$sqlperiod')");
$sth1->execute();
$result1 = $sth1->fetchAll(PDO::FETCH_ASSOC);


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
                    color: '"; echo "$color_hivetemp"; echo "'
                }
            },
            title: {
                text: 'Temperature',
                style: {
                    color: '"; echo "$color_hivetemp"; echo "'
                }
            }
            

        }, { // Rain yAxis
            gridLineWidth: 0,
            title: {
                text: 'Rain',
                style: {
                    color: '"; echo "$color_rain"; echo "'
                }
            },
            labels: {
                format: '{value} in',
                style: {
                    color: '"; echo "$color_rain"; echo "'
                }
            },
            opposite: true

        },
        { // Weight yAxis
            gridLineWidth: 0,
            title: {
                text: 'Weight',
                style: {
                    color: '"; echo "$color_netweight"; echo "'
                }
            },
            labels: {
                format: '{value} lbs',
                style: {
                    color: '"; echo "$color_netweight"; echo "'
                }
            },
            opposite: false

        },
        { // Humidity yAxis
            gridLineWidth: 0,
            title: {
                text: 'Humidity',
                style: {
                    color: '"; echo "$color_hivehum"; echo "'
                }
            },
            labels: {
                format: '{value} %',
                style: {
                    color: '"; echo "$color_hivehum"; echo "'
                }
            },
            opposite: true

        },
        { // Solarradiation yAxis
            gridLineWidth: 0,
            title: {
                text: 'Solar',
                style: {
                    color: '"; echo "$color_solarradiation"; echo "'
                }
            },
            labels: {
                format: '{value} wm/2',
                style: {
                    color: '"; echo "$color_solarradiation"; echo "'
                }
            },
            opposite: true

        },
        { // Lux yAxis
            gridLineWidth: 0,
            title: {
                text: 'Lux',
                style: {
                    color: '"; echo "$color_lux"; echo "'
                }
            },
            labels: {
                format: '{value} lx',
                style: {
                    color: '"; echo "$color_lux"; echo "'
                }
            },
            opposite: true

        },
        { // GDD yAxis
            gridLineWidth: 0,
            title: {
                text: 'GDD',
                style: {
                    color: '"; echo "$color_gdd"; echo "'
                }
            },
            labels: {
                format: '{value} gdd',
                style: {
                    color: '"; echo "$color_gdd"; echo "'
                }
            },
            opposite: false

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
            color: '"; echo "$color_hivetemp"; echo "'
        },
        {
            type: 'line',
            name: 'Outside Temp (°F)',
            yAxis: 0,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['weather_tempf']."]".", ";} echo "],
            visible: false,
            color: '"; echo "$color_outtemp"; echo "'
        },
        {
            type: 'line',
            name: 'Hive Humidty (%)',
            yAxis: 3,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiveHum']."]".", ";} echo "],
            color: '"; echo "$color_hivehum"; echo "',
            visible: true
        },
        {
            type: 'line',
            name: 'Outside Humidty (%)',
            yAxis: 3,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['weather_humidity']."]".", ";} echo "],
            color: '"; echo "$color_outhum"; echo "',
            visible: false
        },
        {
            type: 'column',
            yAxis: 1,
            name: 'Rain',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['precip_1hr_in']."]".", ";} echo "],
            color: '"; echo "$color_rain"; echo "',
            visible: false
        },
        {
            type: 'line',
            yAxis: 2,
            name: 'Hive Weight Net (lbs)',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiveweight']."]".", ";} echo "], 
            color: '"; echo "$color_netweight"; echo "',
            visible: true
        },
        {
           type: 'line',
           yAxis: 2,
           name: 'Hive Weight Gross (lbs)',
           data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiverawweight']."]".", ";} echo "],
           color: '"; echo "$color_grossweight"; echo "',
           visible: false
        },
        {
            type: 'line',
            name: 'Solar (wm/2)',
            yAxis: 4,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['solarradiation']."]".", ";} echo "],
            color: '"; echo "$color_solarradiation"; echo "',
            visible: false
        },
        {
            type: 'line',
            name: 'Lux (lx)',
            yAxis: 5,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['lux']."]".", ";} echo "],
            color: '"; echo "$color_lux"; echo "',
            visible: false
        },
        {
            type: 'line',
            name: 'GDD',
            yAxis: 6,
            data: ["; foreach($result1 as $r){echo "[".$r['datetime'].", ".$r['gdd']."]".", ";} echo "],
            color: '"; echo "$color_gdd"; echo "',
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


 




