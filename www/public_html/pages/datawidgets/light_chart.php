


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
$sth = $conn->prepare("SELECT solarradiation, lux, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

$has_valid = false;
foreach ($result as $r) {
    if ((is_numeric($r['solarradiation'] ?? null) && $r['solarradiation'] != 0) || (is_numeric($r['lux'] ?? null) && $r['lux'] != 0)) {
        $has_valid = true;
        break;
    }
}
if (!$has_valid) {
    echo '<script>document.getElementById("lightcontainer").parentElement.innerHTML = \'<div class="alert alert-info" style="margin:0"><i class="fa fa-info-circle"></i> <strong>No light data available</strong> for the selected time period. Data will appear here once the light sensor begins recording.</div>\';</script>';
    return;
}

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

echo "
<!-- Chart Code -->
<script>
$(function () {
    Highcharts.chart('lightcontainer', {
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
        yAxis: [
        {
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
            minRange: 100,
            opposite: false
        },
        {
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
            minRange: 100,
            opposite: true
        }
        ],
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
                            centeredPopup('/pages/fullscreen/light.php?chart=line&period="; echo htmlspecialchars($period, ENT_QUOTES); echo "','HiveControl','1200','500','yes');
                            return false;
                        }
                    }]
                }
            }
        },
        series: [
        {
            type: 'line',
            name: 'Solar (wm/2)',
            yAxis: 0,
            data: ["; foreach($result as $r){
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
            yAxis: 1,
            data: ["; foreach($result as $r){
                if (is_numeric($r['lux']) && $r['lux'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['lux'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_lux"; echo "',
            visible: "; echo "$trend_lux"; echo "
        }
        ]
    });
});
</script>";


} elseif ($chart == 'bar') {
    # code...
}



?>


 




