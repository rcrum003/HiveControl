


<!-- Chart Code -->


<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart

if ( $SHOW_METRIC == "on") {
    # Abstracted this code so we don't have to specify this every freakin time
    $HIVETEMP = "hivetempc"; 
    $WEATHER_TEMP = "weather_tempc";  
    $PRECIP_1HR = "precip_1hr_metric";
}
else {
    $HIVETEMP = "hivetempf"; 
    $WEATHER_TEMP = "weather_tempf";  
    $PRECIP_1HR = "precip_1hr_in";
}


switch ($period) {
    case "today":
        $sqlperiod = "start of day";
        $sth = $conn->prepare("SELECT $HIVETEMP as hivetemp, hiveHum, $WEATHER_TEMP as weather_temp, weather_humidity, $PRECIP_1HR as precip_1hr, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
        break;
    case "day":
        $sqlperiod = "-1 days";
        $sth = $conn->prepare("SELECT $HIVETEMP as hivetemp, hiveHum, $WEATHER_TEMP as weather_temp, weather_humidity, $PRECIP_1HR as precip_1hr, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");        
        break;
    case "week":
        $sqlperiod = "-7 days";
        $sth = $conn->prepare("SELECT $HIVETEMP as hivetemp, hiveHum, $WEATHER_TEMP as weather_temp, weather_humidity, $PRECIP_1HR as precip_1hr, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");        
        break;
    case "month":
        $sqlperiod = "-1 months";
        $sth = $conn->prepare("SELECT $HIVETEMP as hivetemp, hiveHum, $WEATHER_TEMP as weather_temp, weather_humidity, $PRECIP_1HR as precip_1hr, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");        
        break;
    case "year":
        $sqlperiod =  "-1 years";
        $sth = $conn->prepare("SELECT ROUND(AVG($HIVETEMP),2) as hivetemp, ROUND(AVG(hiveHum),2) as hiveHum, ROUND(AVG($WEATHER_TEMP),2) as weather_temp, ROUND(AVG(weather_humidity),2) as weather_humidity, ROUND(AVG($PRECIP_1HR),2) as precip_1hr, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') GROUP BY strftime('%Y-%m-%d-%HH', date) ORDER by datetime ASC");
        break;
    case "all":
        $sqlperiod =  "-20 years";
        $sth = $conn->prepare("SELECT $HIVETEMP as hivetemp, hiveHum, $WEATHER_TEMP as weather_temp, weather_humidity, $PRECIP_1HR as precip_1hr, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
        break;
    }


if ($chart == 'line') {
    # Echo back the Javascript code
 
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Get Hive Data First

$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);


include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");


echo "
<!-- Chart Code -->


<script>
$(function () {
    $('#tempcontainer').highcharts({
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
                format: '{value}"; if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} echo "',
                style: {
                    color: '"; echo "$color_hivetemp"; echo "'
                }
            },
            title: {
                text: 'Temperature',
                style: {
                    color: '"; echo "$color_hivetemp"; echo "'
                }
            },
            showEmpty: false
        },
        { // Humidity yAxis
            gridLineWidth: 0,
            ceiling: 100,
            floor: 0,
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
            showEmpty: false,
            opposite: false

        }, 
        { // Rain yAxis
            gridLineWidth: 0,
            title: {
                text: 'Rain',
                style: {
                    color: '"; echo "$color_rain"; echo "'
                }
            },
            labels: {
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "mm";} else {echo "in";} echo "',
                style: {
                    color: '"; echo "$color_rain"; echo "'
                }
            },
            showEmpty: false,
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
            name: 'Hive Temp ("; if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} echo ")',
            yAxis: 0,
            data: [";
            foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['hivetemp']) && $r['hivetemp'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['hivetemp'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            }
            echo "],
            color: '"; echo "$color_hivetemp"; echo "',
            visible: "; echo "$trend_hivetemp"; echo "
        },
        {
            type: 'line',
            name: 'Outside Temp ("; if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} echo ")',
            yAxis: 0,
            data: [";
            foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['weather_temp']) && $r['weather_temp'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['weather_temp'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            }
            echo "],
            visible: "; echo "$trend_outtemp"; echo ",
            color: '"; echo "$color_outtemp"; echo "'
        },
        {
            type: 'line',
            name: 'Hive Humidty (%)',
            yAxis: 1,
            data: [";
            foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['hiveHum']) && $r['hiveHum'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['hiveHum'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            }
            echo "],
            color: '"; echo "$color_hivehum"; echo "',
            visible: "; echo "$trend_hivehum"; echo "
        },
        {
            type: 'line',
            name: 'Outside Humidty (%)',
            yAxis: 1,
            data: [";
            foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['weather_humidity']) && $r['weather_humidity'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['weather_humidity'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            }
            echo "],
            color: '"; echo "$color_outhum"; echo "',
            visible: "; echo "$trend_outhum"; echo "
        },
        {
            type: 'area',
            yAxis: 2,
            name: 'Rain ("; if ( $SHOW_METRIC == "on" ) { echo "mm";} else {echo "in";} echo ")',
            data: [";
            foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['precip_1hr']) && $r['precip_1hr'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['precip_1hr'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            }
            echo "],
            color: '"; echo "$color_rain"; echo "',
            visible: "; echo "$trend_rain"; echo "
        }
        ]
        });

        Highcharts.getOptions().exporting.buttons.contextButton.menuItems.push({
            text: 'Enlarge Chart',
            onclick: function () {
                centeredPopup('/pages/fullscreen/temp.php?chart=line&period=";echo $period; echo"','HiveControl','1200','500','yes')
                return false;
            }
        });
    
});
</script>";


} elseif ($chart == 'area') {

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Get Hive Data First

$sqlperiod =  "-1 years";
$sth = $conn->prepare("SELECT MIN($HIVETEMP) as minhivetemp, MAX($HIVETEMP) as maxhivetemp, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') GROUP BY strftime('%Y-%m-%d', date) ORDER by datetime ASC");

$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);


include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");


echo "
<!-- Chart Code -->


<script>
$(function () {
    $('#tempcontainer').highcharts({
        chart: {
            type: 'arearange',
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
                format: '{value}"; if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} echo "',
                style: {
                    color: '"; echo "$color_hivetemp"; echo "'
                }
            },
            title: {
                text: 'Temperature',
                style: {
                    color: '"; echo "$color_hivetemp"; echo "'
                }
            },
            showEmpty: false
        },
        { // Humidity yAxis
            gridLineWidth: 0,
            ceiling: 100,
            floor: 0,
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
            showEmpty: false,
            opposite: false

        }, 
        { // Rain yAxis
            gridLineWidth: 0,
            title: {
                text: 'Rain',
                style: {
                    color: '"; echo "$color_rain"; echo "'
                }
            },
            labels: {
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "mm";} else {echo "in";} echo "',
                style: {
                    color: '"; echo "$color_rain"; echo "'
                }
            },
            showEmpty: false,
            opposite: true

        }],
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
            type: 'arearange',
            name: 'Hive Temp ("; if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} echo ")',
            yAxis: 0,
            data: [";
            foreach($result as $r){
                // Validate numeric values
                $minval = (is_numeric($r['minhivetemp']) && $r['minhivetemp'] != 0) ? floatval($r['minhivetemp']) : null;
                $maxval = (is_numeric($r['maxhivetemp']) && $r['maxhivetemp'] != 0) ? floatval($r['maxhivetemp']) : null;
                if ($minval !== null && $maxval !== null) {
                    echo "[".$r['datetime'].", ".$minval.", ".$maxval."], ";
                } else {
                    echo "[".$r['datetime'].", null, null], ";
                }
            }
            echo "],
            color: '"; echo "$color_hivetemp"; echo "',
            visible: "; echo "$trend_hivetemp"; echo "
        }
        ]
        });

        Highcharts.getOptions().exporting.buttons.contextButton.menuItems.push({
            text: 'Enlarge Chart',
            onclick: function () {
                centeredPopup('/pages/fullscreen/temp.php?chart=line&period=";echo $period; echo"','HiveControl','1200','500','yes')
                return false;
            }
        });
    
});
</script>";
}

?>


 




