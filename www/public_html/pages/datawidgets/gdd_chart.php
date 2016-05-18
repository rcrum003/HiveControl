


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

// Get GDD Data First
$sth = $conn->prepare("SELECT seasongdd AS gdd, strftime('%s',calcdate)*1000 AS datetime FROM gdd WHERE calcdate > datetime('now','$sqlperiod', 'localtime') ORDER by datetime ASC");
$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

// Get Pollen Data
$sth3 = $conn->prepare("SELECT pollenlevel, strftime('%s', date)*1000 AS datetime FROM pollen WHERE date > datetime('now','$sqlperiod', 'localtime') ORDER by datetime ASC");
$sth3->execute();
$result3 = $sth3->fetchAll(PDO::FETCH_ASSOC);

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

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
           
    yAxis: [
        { //GDD yAxis
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
        series: [
        {
            type: 'area',
            name: 'GDD',
            yAxis: 0,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['gdd']."]".", ";} echo "],
            color: '"; echo "$color_gdd"; echo "',
            visible: true
        },
         {
            type: 'area',
            name: 'Pollen',
            yAxis: 1,
            data: ["; foreach($result3 as $r){echo "[".$r['datetime'].", ".$r['pollenlevel']."]".", ";} echo "],
            color: '"; echo "$color_pollen"; echo "',
            visible: "; echo "$trend_pollen"; echo "
        }
        ]
    });
        Highcharts.getOptions().exporting.buttons.contextButton.menuItems.push({
            text: 'Enlarge Chart',
            onclick: function () {
                centeredPopup('/pages/fullscreen/gdd.php?chart=line&period=";echo $period; echo"','HiveControl','1200','500','yes')
                return false;
            }
        });

});
</script>";


} elseif ($chart == 'bar') {
    # code...
}



?>


 




