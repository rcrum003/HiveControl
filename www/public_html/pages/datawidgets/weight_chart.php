


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

$sth = $conn->prepare("SELECT round((hiveweight * 0.453592),2) as hiveweight, round((hiverawweight * 0.453592),2) as hiverawweight, precip_1hr_metric as precip_1hr_in, wind_kph as wind, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
} else {

//Show normal
$sth = $conn->prepare("SELECT hiveweight, hiverawweight, precip_1hr_in, wind_mph as wind, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
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
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo "',
                style: {
                    color: '"; echo "$color_netweight"; echo "'
                }
            },
            title: {
                text: 'Weight',
                style: {
                    color: '"; echo "$color_netweight"; echo "'
                },
            ceiling: 500,
            floor: 0
            }
            

        }, { // Secondary yAxis
            gridLineWidth: 0,
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
            opposite: true

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
            name: 'Hive Weight Net ("; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo ")',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiveweight']."]".", ";} echo "], 
            color: '"; echo "$color_netweight"; echo "'
        },
        {
           type: 'line',
           name: 'Hive Weight Gross ("; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo ")',
           data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiverawweight']."]".", ";} echo "],
           color: '"; echo "$color_grossweight"; echo "',
           visible: true
        },
        {
            type: 'area',
            yAxis: 1,
            name: 'Rain ("; if ( $SHOW_METRIC == "on" ) { echo "mm";} else {echo "in";} echo ")',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['precip_1hr_in']."]".", ";} echo "],
            color: '"; echo "$color_rain"; echo "',
            visible: true
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
$sth = $conn->prepare("SELECT hiveweight, hiverawweight, precip_1hr_in, wind_mph as wind, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
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
                format: '{value} "; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo "',
                style: {
                    color: '"; echo "$color_netweight"; echo "'
                }
            },
            title: {
                text: 'Weight',
                style: {
                    color: '"; echo "$color_netweight"; echo "'
                },
            ceiling: 500,
            floor: 0
            }
            

        }, { // Secondary yAxis
            gridLineWidth: 0,
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
            opposite: true

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
            name: 'Hive Weight Net ("; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo ")',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiveweight']."]".", ";} echo "], 
            color: '"; echo "$color_netweight"; echo "'
        },
        {
           type: 'line',
           name: 'Hive Weight Gross ("; if ( $SHOW_METRIC == "on" ) { echo "kg";} else {echo "lb";} echo ")',
           data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiverawweight']."]".", ";} echo "],
           color: '"; echo "$color_grossweight"; echo "',
           visible: true
        },
        {
            type: 'area',
            yAxis: 1,
            name: 'Rain ("; if ( $SHOW_METRIC == "on" ) { echo "mm";} else {echo "in";} echo ")',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['precip_1hr_in']."]".", ";} echo "],
            color: '"; echo "$color_rain"; echo "',
            visible: true
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
                centeredPopup('/pages/fullscreen/weight.php?chart=line&period=";echo $period; echo"','HiveControl','1200','500','yes')
                return false;
            }
        });
});
</script>";
}



?>


 




