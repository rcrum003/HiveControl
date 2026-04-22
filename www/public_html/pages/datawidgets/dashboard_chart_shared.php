<?php include_once "chart_data_fetch.php"; ?>

<script>
var dashboardCharts = [];

var hcCommonXAxis = {
    type: 'datetime',
    crosshair: { width: 1, color: '#ccc' },
    dateTimeLabelFormats: {
        second: '%m-%d<br/>%H:%M:%S',
        minute: '%m-%d<br/>%H:%M',
        hour: '%m-%d<br/>%H:%M',
        day: '<br/>%m-%d',
        week: '<br/>%m-%d',
        month: '%Y-%m',
        year: '%Y'
    },
    events: {
        afterSetExtremes: function(e) {
            var triggerChart = this.chart;
            dashboardCharts.forEach(function(c) {
                if (c !== triggerChart && c.xAxis && c.xAxis[0]) {
                    c.xAxis[0].setExtremes(e.min, e.max, true, false);
                }
            });
        }
    }
};

var hcCommonTooltip = {
    shared: true,
    formatter: function () {
        var s = '<b>' + Highcharts.dateFormat('%m/%d %H:%M', this.x) + '</b>';
        this.points.forEach(function (point) {
            s += '<br/><span style="color:' + point.series.color + '">●</span> ' +
                 point.series.name + ': <b>' + Highcharts.numberFormat(point.y, 1) + '</b>';
        });
        return s;
    }
};

var hcCommonExporting = {
    buttons: {
        contextButton: {
            menuItems: ['viewFullscreen', 'printChart', 'separator', 'downloadPNG', 'downloadJPEG', 'downloadPDF', 'downloadSVG']
        }
    }
};

function syncDashboardCrosshair(e, thisChart) {
    dashboardCharts.forEach(function(chart) {
        if (chart !== thisChart && chart.pointer) {
            var event = chart.pointer.normalize(e);
            var point;
            chart.series.forEach(function(s) {
                if (s.visible && s.points && s.points.length) {
                    var p = s.searchPoint(event, true);
                    if (p) point = p;
                }
            });
            if (point) {
                point.onMouseOver();
                chart.xAxis[0].drawCrosshair(event, point);
            }
        }
    });
}
</script>
