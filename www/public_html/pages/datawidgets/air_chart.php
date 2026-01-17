


<!-- Chart Code -->


<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart

#echo "Second Chart is $chart";
#echo "Second Period is "; 

switch ($period) {
    case "today":
        $sqlperiod = "start of day";
        break;

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

if ( $SHOW_METRIC == "on" ) {

$sth = $conn->prepare("SELECT air_pm1, air_pm2_5,weather_tempf, air_aqi, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
} else {

//Show normal
$sth = $conn->prepare("SELECT air_pm1, air_pm2_5,weather_tempf, air_aqi, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
}

$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

echo "
<!-- Chart Code -->


<script>
$(function () {
    $('#container').highcharts({
        chart: {
            
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
           
        yAxis: [{ // Primary yAxis
            labels: {
                style: {
                    color: '#000000'
                }
            },
            title: {
                text: 'AIR ug/m3',
                style: {
                    color: '#000000'
                },
            ceiling: 500,
            floor: 0
            }
            

        }, { // Secondary yAxis
            gridLineWidth: 0,
            title: {
                text: 'Temp',
                style: {
                    color: '"; echo "$color_outtemp"; echo "'
                }
            },
            labels: {
                 format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} echo "',
                style: {
                    color: '"; echo "$color_outtemp"; echo "'
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
            name: 'PM1("; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo ")',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['air_pm1']."]".", ";} echo "], 
            color: '"; echo "$color_pm1"; echo "'
        },
        {
           type: 'line',
           name: 'PM2.5 ("; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo ")',
           data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['air_pm2_5']."]".", ";} echo "],
           color: '"; echo "$color_pm25"; echo "',
           visible: true
        },
        {
            type: 'line',
            name: 'AQI("; if ( $SHOW_METRIC == "on" ) { echo "um";} else {echo "um";} echo ")',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['air_aqi']."]".", ";} echo "], 
            color: '"; echo "$color_aqi"; echo "'
        },
        {
            type: 'line',
            yAxis: 1,
            name: 'Rain ("; if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} echo ")',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['weather_rain']."]".", ";} echo "],
            color: '"; echo "$color_rain"; echo "',
            visible: true
        }
        ]
    });

    $(\"#b\").click(function(){
            chart.yAxis[0].update({
                labels: {
                    enabled: false
                },
                title: {
                    text: null
                }
            });
        });

        Highcharts.getOptions().exporting.buttons.contextButton.menuItems.push({
            text: 'Enlarge Chart',
            onclick: function () {
                centeredPopup('/pages/fullscreen/weight.php?chart=line&period=";echo $period; echo"','HiveControl','1200','500','yes')
                return false;
            }
        });
});
</script>";


} elseif ($chart == 'candle') {
   include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Get Hive Data First

if ( $SHOW_METRIC == "on" ) {

$sth = $conn->prepare("SELECT round((hiveweight * 0.453592),2) as hiveweight, round((hiverawweight * 0.453592),2) as hiverawweight, precip_1hr_metric as precip_1hr_in, wind_kph as wind, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
} else {

//Show normal
$sth = $conn->prepare("SELECT air_pm1, air_pm2_5,weather_tempf,air_aqi,strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
}

$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

echo "
<!-- Chart Code -->


<script>
$(function () {
    $('#container').highcharts({
        chart: {
            
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
           
        yAxis: [{ // Primary yAxis
            labels: {
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "ppm";} else {echo "ppm";} echo "',
                style: {
                    color: '"; echo "$color_pm1"; echo "'
                }
            },
            title: {
                text: 'Weight',
                style: {
                    color: '"; echo "$color_pm1"; echo "'
                },
            ceiling: 500,
            floor: 0
            }
            

        }, { // Secondary yAxis
            gridLineWidth: 0,
            title: {
                text: 'AQI',
                style: {
                    color: '"; echo "$color_aqi"; echo "'
                }
            },
            labels: {
                 format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "mm";} else {echo "°in";} echo "',
                style: {
                    color: '"; echo "$color_aqi"; echo "'
                }
            },
            opposite: true

        }, 
         { // Wind yAxis
            gridLineWidth: 1,
            title: {
                text: 'Wind',
                style: {
                    color: '"; echo "$color_pm25"; echo "'
                }
            },
            labels: {
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "kph";} else {echo "mph";} echo "',
                style: {
                    color: '"; echo "$color_pm25"; echo "'
                }
            },
            showEmpty: false,
            opposite: false

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
            name: 'Air PM1 ("; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo ")',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['air_pm1']."]".", ";} echo "], 
            color: '"; echo "$color_pm1"; echo "'
        },
        {
           type: 'line',
           name: 'Air PM2 ("; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo ")',
           data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['air_pm2_5']."]".", ";} echo "],
           color: '"; echo "$color_pm25"; echo "',
           visible: true
        },
        {
           type: 'line',
           name: 'Air TempF ("; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo ")',
           data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['air_aqi']."]".", ";} echo "],
           color: '"; echo "$color_aqi"; echo "',
           visible: true
        },
        }, 
        {
            type: 'line',
            name: 'Wind',
            yAxis: 2,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['wind']."]".", ";} echo "],
            color: '"; echo "$color_wind"; echo "',
            visible: "; echo "$trend_wind"; echo "
        }
        ]
    });

    $(\"#b\").click(function(){
            chart.yAxis[0].update({
                labels: {
                    enabled: false
                },
                title: {
                    text: null
                }
            });
        });

        Highcharts.getOptions().exporting.buttons.contextButton.menuItems.push({
            text: 'Enlarge Chart',
            onclick: function () {
                centeredPopup('/pages/fullscreen/air.php?chart=line&period=";echo $period; echo"','HiveControl','1200','500','yes')
                return false;
            }
        });
});
</script>";
}



?>


 




