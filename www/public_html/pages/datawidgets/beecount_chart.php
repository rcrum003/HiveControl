


<!-- Chart Code -->


<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart

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

if ( $SHOW_METRIC == "on" ) {

// Get Hive Data First
$sth = $conn->prepare("SELECT IN_COUNT, OUT_COUNT, precip_1hr_metric as precip_1hr_in, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
} else {
//Show normal
$sth = $conn->prepare("SELECT IN_COUNT, OUT_COUNT, precip_1hr_in, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
}

$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

$has_valid = false;
foreach ($result as $r) {
    if ((is_numeric($r['IN_COUNT'] ?? null) && $r['IN_COUNT'] != 0) || (is_numeric($r['OUT_COUNT'] ?? null) && $r['OUT_COUNT'] != 0)) {
        $has_valid = true;
        break;
    }
}
if (!$has_valid) {
    echo '<script>document.getElementById("container").parentElement.innerHTML = \'<div class="alert alert-info" style="margin:0"><i class="fa fa-info-circle"></i> <strong>No bee count data available</strong> for the selected time period. Data will appear here once the bee counter begins recording.</div>\';</script>';
    return;
}

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

echo "
<!-- Chart Code -->
<script>
$(function () {
    Highcharts.chart('container', {
        chart: {
            zoomType: 'xy'
        },
        title: {
            text: ''
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
        yAxis: [{
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
            },
            minRange: 20
        }, {
            gridLineWidth: 1,
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
                marker: {
                    enabled: true,
                    radius: 2,
                    symbol: 'circle'
                },
                dataLabels: {
                    enabled: false
                },
                enableMouseTracking: true
            }
        },
        tooltip: {
            formatter: function () {
                var s = '<b>' + Highcharts.dateFormat('%m/%d %H:%M', this.x) + '</b>';
                this.points.forEach(function (point) {
                    s += '<br/>' + point.series.name + ': ' + point.y;
                });
                return s;
            },
            shared: true
        },
        exporting: {
            buttons: {
                contextButton: {
                    menuItems: ['viewFullscreen', 'printChart', 'separator', 'downloadPNG', 'downloadJPEG', 'downloadPDF', 'downloadSVG', 'separator', {
                        text: 'Enlarge Chart',
                        onclick: function () {
                            centeredPopup('/pages/fullscreen/beecount.php?chart=line&period="; echo htmlspecialchars($period, ENT_QUOTES); echo "','HiveControl','1200','500','yes');
                            return false;
                        }
                    }]
                }
            }
        },
        series: [{
            type: 'line',
            name: 'Bees In',
            yAxis: 0,
            data: ["; foreach($result as $r){
                if (is_numeric($r['IN_COUNT']) && $r['IN_COUNT'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['IN_COUNT'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_beecount_in"; echo "',
            visible: "; echo "$trend_beecount_in"; echo "
        },
        {
            type: 'line',
            name: 'Bees Out',
            yAxis: 0,
            data: ["; foreach($result as $r){
                if (is_numeric($r['OUT_COUNT']) && $r['OUT_COUNT'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['OUT_COUNT'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            visible: "; echo "$trend_beecount_out"; echo ",
            color: '"; echo "$color_beecount_out"; echo "'
        },
        {
            type: 'area',
            yAxis: 1,
            name: 'Rain ("; if ( $SHOW_METRIC == "on" ) { echo "mm";} else {echo "in";} echo ")',
            data: ["; foreach($result as $r){
                if (is_numeric($r['precip_1hr_in']) && $r['precip_1hr_in'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['precip_1hr_in'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_rain"; echo "',
            visible: "; echo "$trend_rain"; echo "
        }
        ]
    });
});
</script>";


} elseif ($chart == 'bar') {
    # code...
}



?>


 




