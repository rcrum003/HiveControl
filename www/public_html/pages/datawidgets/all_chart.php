


<!-- Chart Code -->


<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart

switch ($period) {
    case "today":
        $sqlperiod = "start of day";
        $groupby="";
        break;
    case "day":
        $sqlperiod = "-24 hours";
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

#Useful when summary by month is needed:
    #select strftime("%Y-%m-%d", date) as observerd, min(hivetempf) as low, max(hivetempf) from allhivedata where hivetempf != "" AND hivetempf != "null" GROUP BY strftime("%Y-%m-%d", date);
    

# Echo back the Javascript code
 
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Get Hive Data First

if ( $SHOW_METRIC == "on" ) {
$sth = $conn->prepare("SELECT round((hiveweight * 0.453592),2) as hiveweight, round((hiverawweight * 0.453592),2) as hiverawweight, hivetempc AS hivetempf, hiveHum, weather_tempc as weather_tempf, weather_humidity, precip_1hr_metric as precip_1hr_in, solarradiation, lux, in_count, out_count, wind_kph as wind, pressure_mb as pressure, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
} else {
$sth = $conn->prepare("SELECT hiveweight, hiverawweight, hivetempf, hiveHum, weather_tempf, weather_humidity, precip_1hr_in, solarradiation, lux, in_count, out_count, wind_mph as wind, pressure_in as pressure, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
}

$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

$sth1 = $conn->prepare("SELECT seasongdd AS gdd, strftime('%s',calcdate)*1000 AS datetime FROM gdd WHERE calcdate > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
$sth1->execute();
$result1 = $sth1->fetchAll(PDO::FETCH_ASSOC);


$sth3 = $conn->prepare("SELECT pollenlevel, strftime('%s', date)*1000 AS datetime FROM pollen WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
$sth3->execute();
$result3 = $sth3->fetchAll(PDO::FETCH_ASSOC);


include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");





echo "
<!-- Chart Code -->


<script>
$(function () {
    $('#allcontainer').highcharts({
        chart: {
            zoomType: 'xy',
            alignTicks: false
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
            gridLineWidth: 0,
            labels: {
                format: '{value}"; if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} echo "',
                style: {
                    color: '"; echo "$color_hivetemp"; echo "'
                },
                padding: 1

            },
            showEmpty: false,
            title: {
                text: 'Temperature',
                style: {
                    color: '"; echo "$color_hivetemp"; echo "'
                }
            }

        }, { // Rain yAxis
            gridLineWidth: 1,
            title: {
                text: 'Rain',
                style: {
                    color: '"; echo "$color_rain"; echo "'
                }
            },
            labels: {
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "mm";} else {echo "°in";} echo "',
                style: {
                    color: '"; echo "$color_rain"; echo "'
                }
            },
            showEmpty: false,
            opposite: true

        },
        { // Weight yAxis
            gridLineWidth: 1,
            title: {
                text: 'Weight',
                style: {
                    color: '"; echo "$color_netweight"; echo "'
                }
            },
            labels: {
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo "',
                style: {
                    color: '"; echo "$color_netweight"; echo "'
                }
            },
            showEmpty: false,
            opposite: false

        },
        { // Humidity yAxis
            gridLineWidth: 1,
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
            opposite: true

        },
        { // Solarradiation yAxis
            gridLineWidth: 1,
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
            showEmpty: false,
            opposite: true

        },
        { // Lux yAxis
            gridLineWidth: 1,
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
            showEmpty: false,
            opposite: true

        },
        { // GDD yAxis
            gridLineWidth: 1,
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
            showEmpty: false,
            opposite: false

        },
         { // Flight yAxis
            gridLineWidth: 1,
            title: {
                text: 'Flights Out',
                style: {
                    color: '"; echo "$color_beecount_out"; echo "'
                }
            },
            labels: {
                format: '{value}',
                style: {
                    color: '"; echo "$color_beecount_out"; echo "'
                }
            },
            showEmpty: false,
            opposite: false

        },
        { // Wind yAxis
            gridLineWidth: 1,
            title: {
                text: 'Wind',
                style: {
                    color: '"; echo "$color_wind"; echo "'
                }
            },
            labels: {
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "kph";} else {echo "mph";} echo "',
                style: {
                    color: '"; echo "$color_wind"; echo "'
                }
            },
            showEmpty: false,
            opposite: false

        },
        { // Pressure yAxis
            gridLineWidth: 1,
            title: {
                text: 'Pressure',
                style: {
                    color: '"; echo "$color_pressure"; echo "'
                }
            },
            labels: {
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "mb";} else {echo "in";} echo "',
                style: {
                    color: '"; echo "$color_pressure"; echo "'
                }
            },
            showEmpty: false,
            opposite: false

        },
        { // Pollen yAxis
            gridLineWidth: 1,
            title: {
                text: 'Pollen',
                style: {
                    color: '"; echo "$color_pollen"; echo "'
                }
            },
            labels: {
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "mb";} else {echo "in";} echo "',
                style: {
                    color: '"; echo "$color_pollen"; echo "'
                }
            },
            showEmpty: false,
            opposite: false

        }
        ],
        plotOptions: {";
             if ( $chart_smoothing == "on" ) { 
                echo "series: {
                    connectNulls:true
                },";
        } echo "
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
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['hivetempf']) && $r['hivetempf'] != 0) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['hivetempf'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['hivetempf'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_hivetemp"; echo "',
            visible: "; echo "$trend_hivetemp"; echo "
        },
        {
            type: 'line',
            name: 'Outside Temp ("; if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} echo ")',
            yAxis: 0,
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['weather_tempf']) && $r['weather_tempf'] != 0) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['weather_tempf'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['weather_tempf'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            visible: "; echo "$trend_outtemp"; echo ",
            color: '"; echo "$color_outtemp"; echo "'
        },
        {
            type: 'line',
            name: 'Hive Humidty (%)',
            yAxis: 3,
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['hiveHum']) && $r['hiveHum'] != 0) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['hiveHum'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['hiveHum'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_hivehum"; echo "',
            visible: "; echo "$trend_hivehum"; echo "
        },
        {
            type: 'line',
            name: 'Outside Humidty (%)',
            yAxis: 3,
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['weather_humidity']) && $r['weather_humidity'] != 0) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['weather_humidity'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['weather_humidity'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_outhum"; echo "',
            visible: "; echo "$trend_outhum"; echo "
        },
        {
            type: 'area',
            yAxis: 1,
            name: 'Rain ("; if ( $SHOW_METRIC == "on" ) { echo "mm";} else {echo "in";} echo ")',
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['precip_1hr_in']) && $r['precip_1hr_in'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['precip_1hr_in'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_rain"; echo "',
            visible: "; echo "$trend_rain"; echo "
        },
        {
            type: 'line',
            yAxis: 2,
            name: 'Hive Weight Net ("; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo ")',
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['hiveweight']) && $r['hiveweight'] != 0) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['hiveweight'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['hiveweight'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_netweight"; echo "',
            visible: "; echo "$trend_netweight"; echo "
        },
        {
           type: 'line',
           yAxis: 2,
           name: 'Hive Weight Gross ("; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo ")',
           data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['hiverawweight']) && $r['hiverawweight'] != 0) {
                    if ($chart_rounding == "on") {
                        echo "[".$r['datetime'].", ".ceil($r['hiverawweight'])."], ";
                    } else {
                        echo "[".$r['datetime'].", ".floatval($r['hiverawweight'])."], ";
                    }
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
           color: '"; echo "$color_grossweight"; echo "',
           visible: "; echo "$trend_grossweight"; echo "
        },
        {
            type: 'line',
            name: 'Solar (wm/2)',
            yAxis: 4,
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['solarradiation']) && $r['solarradiation'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['solarradiation'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_solarradiation"; echo "',
            visible: "; echo "$trend_solarradiation"; echo "
        },
        {
            type: 'line',
            name: 'Lux (lx)',
            yAxis: 5,
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['lux']) && $r['lux'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['lux'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_lux"; echo "',
            visible: "; echo "$trend_lux"; echo "
        },
        {
            type: 'line',
            name: 'GDD',
            yAxis: 6,
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['gdd']) && $r['gdd'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['gdd'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_gdd"; echo "',
            visible: "; echo "$trend_gdd"; echo "
        },
        {
            type: 'line',
            name: 'Flight Activity',
            yAxis: 7,
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['OUT_COUNT']) && $r['OUT_COUNT'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['OUT_COUNT'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_beecount_out"; echo "',
            visible: "; echo "$trend_beecount_out"; echo "
        },        
        {
            type: 'line',
            name: 'Wind',
            yAxis: 8,
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['wind']) && $r['wind'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['wind'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_wind"; echo "',
            visible: "; echo "$trend_wind"; echo "
        },
        {
            type: 'line',
            name: 'Pressure',
            yAxis: 9,
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['pressure']) && $r['pressure'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['pressure'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_pressure"; echo "',
            visible: "; echo "$trend_pressure"; echo "
        },
        {
            type: 'line',
            name: 'Pollen',
            yAxis: 10,
            data: ["; foreach($result as $r){
                // Validate numeric value
                if (is_numeric($r['pollenlevel']) && $r['pollenlevel'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['pollenlevel'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_pollen"; echo "',
            visible: "; echo "$trend_pollen"; echo "
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
                centeredPopup('/pages/fullscreen/all.php?chart=line&period=";echo $period; echo"','HiveControl','1200','500','yes')
                return false;
            }
        });

});
</script>";



?>


 




