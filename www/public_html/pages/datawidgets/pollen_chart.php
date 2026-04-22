


<!-- Pollen Chart Code -->

<?php
# Uses $period and $chart variables from the parent page

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

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sth = $conn->prepare("SELECT pollenlevel, pollentypes, strftime('%s', date)*1000 AS datetime FROM pollen WHERE date >= datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);

if (empty($result)) {
    echo '<div class="alert alert-info" style="margin:20px 0"><i class="fa fa-info-circle"></i> <strong>No pollen data available</strong> for the selected time period. Pollen data is collected daily from online sources.</div>';
    return;
}

include($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

echo "
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
            title: {
                text: 'Pollen Index',
                style: {
                    color: '"; echo "$color_pollen"; echo "'
                }
            },
            labels: {
                format: '{value}',
                style: {
                    color: '"; echo "$color_pollen"; echo "'
                }
            },
            min: 0,
            max: 12,
            plotBands: [{
                from: 0,
                to: 2.4,
                color: 'rgba(76, 175, 80, 0.1)',
                label: { text: 'Low', style: { color: '#4CAF50', fontSize: '10px' } }
            }, {
                from: 2.4,
                to: 4.8,
                color: 'rgba(139, 195, 74, 0.1)',
                label: { text: 'Low-Med', style: { color: '#8BC34A', fontSize: '10px' } }
            }, {
                from: 4.8,
                to: 7.2,
                color: 'rgba(255, 193, 7, 0.1)',
                label: { text: 'Medium', style: { color: '#FFC107', fontSize: '10px' } }
            }, {
                from: 7.2,
                to: 9.6,
                color: 'rgba(255, 152, 0, 0.1)',
                label: { text: 'Med-High', style: { color: '#FF9800', fontSize: '10px' } }
            }, {
                from: 9.6,
                to: 12,
                color: 'rgba(244, 67, 54, 0.1)',
                label: { text: 'High', style: { color: '#F44336', fontSize: '10px' } }
            }]
        },
        plotOptions: {
            line: {
                marker: {
                    enabled: true,
                    radius: 3,
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
                            centeredPopup('/pages/fullscreen/pollen.php?chart=line&period="; echo htmlspecialchars($period, ENT_QUOTES); echo "','HiveControl','1200','500','yes');
                            return false;
                        }
                    }]
                }
            }
        },
        series: [
        {
            type: 'area',
            name: 'Pollen Index',
            data: ["; foreach($result as $r){
                if (is_numeric($r['pollenlevel']) && $r['pollenlevel'] != 0) {
                    echo "[".$r['datetime'].", ".floatval($r['pollenlevel'])."], ";
                } else {
                    echo "[".$r['datetime'].", null], ";
                }
            } echo "],
            color: '"; echo "$color_pollen"; echo "',
            fillOpacity: 0.3,
            visible: true
        }
        ]
    });
});
</script>";


} elseif ($chart == 'bar') {
    # placeholder
}

?>
