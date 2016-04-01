


<!-- Chart Code -->


<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart

#echo "Second Chart is $chart";
#echo "Second Period is "; 

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
$sth = $conn->prepare("SELECT hiveweight, hiverawweight, precip_1hr_in, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime')");
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
                format: '{value} lbs',
                style: {
                    color: '"; echo "$color_netweight"; echo "'
                }
            },
            title: {
                text: 'Weight',
                style: {
                    color: '"; echo "$color_netweight"; echo "'
                }
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
                format: '{value} in',
                style: {
                    color: '"; echo "$color_rain"; echo "'
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
            name: 'Hive Weight Net (lbs)',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiveweight']."]".", ";} echo "],
            dashStyle: 'longdash', 
            color: '"; echo "$color_netweight"; echo "'
        },
        {
           type: 'line',
           name: 'Hive Weight Gross (lbs)',
           data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['hiverawweight']."]".", ";} echo "],
           color: '"; echo "$color_grossweight"; echo "',
           dashStyle: 'longdash',
           visible: true
        },
        {
            type: 'column',
            yAxis: 1,
            name: 'Rain',
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['precip_1hr_in']."]".", ";} echo "],
            color: '"; echo "$color_rain"; echo "',
            visible: true
        }
        ]
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


} elseif ($chart == 'bar') {
    # code...
}



?>


 




