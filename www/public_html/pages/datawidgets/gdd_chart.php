


<!-- Chart Code -->


<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart

#echo "Second Chart is $chart";
#echo "Second Period is "; 

switch ($period) {
    case "today":
    case "day":
        $sqlperiod = "-7 days";
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
$sth = $conn->prepare("SELECT seasongdd AS gdd, strftime('%s',gdddate)*1000 AS datetime FROM gdd WHERE gdddate > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

$has_valid = false;
foreach ($result as $r) {
    if (is_numeric($r['gdd'] ?? null)) {
        $has_valid = true;
        break;
    }
}
if (!$has_valid) {
    echo '<div class="alert alert-info" style="margin:20px 0"><i class="fa fa-info-circle"></i> <strong>No growing degree day data available</strong> for the selected time period. GDD data is calculated daily from weather observations.</div>';
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
        yAxis: {
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
            minRange: 100
        },
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
                            centeredPopup('/pages/fullscreen/gdd.php?chart=line&period="; echo htmlspecialchars($period, ENT_QUOTES); echo "','HiveControl','1200','500','yes');
                            return false;
                        }
                    }]
                }
            }
        },
        series: [{
            type: 'area',
            name: 'GDD',
            data: ["; foreach($result as $r){
                if (is_numeric($r['gdd']) && $r['gdd'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['gdd'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_gdd"; echo "',
            visible: true
        }]
    });
});
</script>";


} elseif ($chart == 'bar') {
    # code...
}



?>


 




