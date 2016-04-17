


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
$sth = $conn->prepare("SELECT IN_COUNT, OUT_COUNT, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime')");
$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);


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
           
    yAxis: [{ // In yAxis
            labels: {
                format: '{value}',
                style: {
                    color: '"; echo "$color_beecount_in"; echo "'
                }
            },
            title: {
                text: 'Bee Count In',
                style: {
                    color: '"; echo "$color_beecount_in"; echo "'
                }
            }
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
            name: 'Bees In',
            yAxis: 0,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['IN_COUNT']."]".", ";} echo "],
            color: '"; echo "$color_beecount_in"; echo "',
            visible: "; echo "$trend_beecount_in"; echo "
        },
        {
            type: 'line',
            name: 'Bees Out',
            yAxis: 0,
            data: ["; foreach($result as $r){echo "[".$r['datetime'].", ".$r['OUT_COUNT']."]".", ";} echo "],
            visible: "; echo "$trend_beecount_out"; echo ",
            color: '"; echo "$color_beecount_out"; echo "'
        }
        ]
        });

        Highcharts.getOptions().exporting.buttons.contextButton.menuItems.push({
            text: 'Enlarge Chart',
            onclick: function () {
                centeredPopup('/pages/fullscreen/beecount.php?chart=line&period=";echo $period; echo"','HiveControl','1200','500','yes')
                return false;
            }
        });
    
});
</script>";


} elseif ($chart == 'bar') {
    # code...
}



?>


 




