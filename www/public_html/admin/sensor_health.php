<?PHP
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
include($_SERVER["DOCUMENT_ROOT"] . "/include/security-init.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "/include/sensor_health_check.php");

function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}

$period = isset($_GET['period']) ? test_input($_GET['period']) : '1';
if (!in_array($period, ['1', '7', '30'])) {
    $period = '1';
}

$health = get_sensor_health($conn);
$sensors = $health['sensors'];
$config = $health['config'];
$is_metric = ($config['SHOW_METRIC'] === 'on');
$stats = get_sensor_stats($conn, intval($period), $is_metric);
$rh = get_reporting_health($conn, intval($period));
$reporting = $rh['reporting'];
$rh_summary = $rh['summary'];

// Weather provider stats
$provider_stats = [];
$recent_failures = [];
$chart_providers = [];
$chart_response = [];
$primary_pct = 0;
$fallback_pct = 0;

try {
    $sth2 = $conn->prepare("SELECT provider, COUNT(*) as total_calls, SUM(success) as successes, ROUND(AVG(success) * 100, 1) as success_rate, ROUND(AVG(response_ms), 0) as avg_response_ms, ROUND(AVG(CASE WHEN success = 1 THEN observation_age_minutes END), 1) as avg_obs_age, MAX(timestamp) as last_seen FROM weather_health WHERE timestamp >= datetime('now', '-' || ? || ' days', 'localtime') GROUP BY provider ORDER BY total_calls DESC");
    $sth2->execute([$period]);
    $provider_stats = $sth2->fetchAll(PDO::FETCH_ASSOC);

    $sth3 = $conn->prepare("SELECT timestamp, provider, role, error_reason, observation_age_minutes, response_ms FROM weather_health WHERE success = 0 AND timestamp >= datetime('now', '-' || ? || ' days', 'localtime') ORDER BY timestamp DESC LIMIT 50");
    $sth3->execute([$period]);
    $recent_failures = $sth3->fetchAll(PDO::FETCH_ASSOC);

    $sth4 = $conn->prepare("SELECT strftime('%Y-%m-%d %H:00', timestamp) as hour, provider, COUNT(*) as calls, SUM(success) as successes, ROUND(AVG(response_ms), 0) as avg_ms FROM weather_health WHERE timestamp >= datetime('now', '-' || ? || ' days', 'localtime') GROUP BY hour, provider ORDER BY hour ASC");
    $sth4->execute([$period]);
    $hourly_data = $sth4->fetchAll(PDO::FETCH_ASSOC);

    foreach ($hourly_data as $row) {
        $ts = strtotime($row['hour']) * 1000;
        $rate = $row['calls'] > 0 ? round(($row['successes'] / $row['calls']) * 100, 1) : 0;
        $chart_providers[$row['provider']][] = [$ts, $rate];
        $chart_response[$row['provider']][] = [$ts, (int)$row['avg_ms']];
    }

    $sth5 = $conn->prepare("SELECT role, COUNT(*) as total, SUM(success) as successes FROM weather_health WHERE success = 1 AND timestamp >= datetime('now', '-' || ? || ' days', 'localtime') GROUP BY role");
    $sth5->execute([$period]);
    $role_stats = $sth5->fetchAll(PDO::FETCH_ASSOC);
    $total_success = 0;
    foreach ($role_stats as $r) { $total_success += $r['successes']; }
    foreach ($role_stats as $r) {
        if ($r['role'] === 'primary') {
            $primary_pct = $total_success > 0 ? round(($r['successes'] / $total_success) * 100, 1) : 0;
        } else {
            $fallback_pct += $total_success > 0 ? round(($r['successes'] / $total_success) * 100, 1) : 0;
        }
    }
} catch (PDOException $e) {}

$temp_unit = $is_metric ? '°C' : '°F';
$weight_unit = $is_metric ? 'kg' : 'lb';

// Helper to render a sensor section
function render_sensor_section($key, $sensor, $stats_data, $rh_sum, $stats_rows, $chart_id) {
    if (!$sensor['enabled'] && !$sensor['always']) return;
    $lbl = status_label_class($sensor['status']);
    $uptime = isset($rh_sum[$key]) ? $rh_sum[$key]['uptime_pct'] : 0;
    $gaps = isset($rh_sum[$key]) ? $rh_sum[$key]['gap_count'] : 0;
    $total_p = isset($rh_sum[$key]) ? $rh_sum[$key]['total_periods'] : 0;
    $uptime_cls = $uptime >= 95 ? 'success' : ($uptime >= 75 ? 'warning' : 'danger');
    ?>
            <div class="row" id="section-<?PHP echo $key; ?>">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa <?PHP echo $sensor['icon']; ?> fa-fw"></i> <strong><?PHP echo htmlspecialchars($sensor['name']); ?></strong>
                            <span class="label <?PHP echo $lbl; ?> pull-right" style="margin-left:5px"><?PHP echo htmlspecialchars($sensor['status_label']); ?></span>
                            <?PHP if ($sensor['last_reading']) { ?>
                            <small class="pull-right text-muted" style="margin-right:10px">Last: <?PHP echo htmlspecialchars($sensor['last_reading']); ?> (<?PHP echo format_age($sensor['age_seconds']); ?>)</small>
                            <?PHP } ?>
                        </div>
                        <div class="panel-body">
                            <div class="row">
                                <!-- Reporting Health Chart -->
                                <div class="col-lg-8 col-md-7">
                                    <div id="<?PHP echo $chart_id; ?>" style="height:180px;"></div>
                                </div>
                                <!-- Stats -->
                                <div class="col-lg-4 col-md-5">
                                    <table class="table table-condensed table-striped" style="margin-bottom:5px">
                                        <tr><td><strong>Uptime</strong></td><td><span class="label label-<?PHP echo $uptime_cls; ?>"><?PHP echo $uptime; ?>%</span></td></tr>
                                        <tr><td>Periods Tracked</td><td><?PHP echo $total_p; ?></td></tr>
                                        <tr><td>Gaps (no data)</td><td><?PHP echo $gaps > 0 ? '<span class="text-danger">' . $gaps . '</span>' : '<span class="text-success">0</span>'; ?></td></tr>
                                        <?PHP foreach ($stats_rows as $label => $val) {
                                            echo '<tr><td>' . $label . '</td><td>' . $val . '</td></tr>';
                                        } ?>
                                    </table>
                                </div>
                            </div>
                        </div>
                        <div class="panel-footer">
                            <a href="<?PHP echo htmlspecialchars($sensor['detail_url']); ?>">View Data <i class="fa fa-arrow-circle-right"></i></a>
                        </div>
                    </div>
                </div>
            </div>
    <?PHP
}
?>
<!DOCTYPE html>
<html lang="en">

    <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>

    <div id="wrapper">
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Sensor Health Dashboard</h1>
                </div>
            </div>

            <!-- Period Selector -->
            <div class="row">
                <div class="col-lg-12">
                    <div class="btn-group" style="margin-bottom: 15px;">
                        <a href="?period=1" class="btn btn-default <?PHP echo $period === '1' ? 'active' : ''; ?>">24 Hours</a>
                        <a href="?period=7" class="btn btn-default <?PHP echo $period === '7' ? 'active' : ''; ?>">7 Days</a>
                        <a href="?period=30" class="btn btn-default <?PHP echo $period === '30' ? 'active' : ''; ?>">30 Days</a>
                    </div>
                </div>
            </div>

            <!-- Summary Cards -->
            <div class="row">
<?PHP
foreach ($sensors as $key => $s) {
    $panel_class = status_panel_class($s['status']);
    $label_class = status_label_class($s['status']);
    $age_str = ($s['enabled'] || $s['always']) && $s['last_reading'] ? format_age($s['age_seconds']) : '';
    $vals = '';
    foreach ($s['values'] as $k => $v) {
        if ($k === 'Source' || $k === 'Types') continue;
        $vals .= htmlspecialchars($v) . ' ';
    }
    $gray_style = ($s['status'] === 'gray') ? ' style="background-color:#d5d5d5; border-color:#bbb;"' : '';
    $gray_head = ($s['status'] === 'gray') ? ' style="background-color:#aaa; border-color:#999; color:#fff;"' : '';
    echo '
                <div class="col-lg-3 col-md-6">
                    <div class="panel ' . $panel_class . '"' . $gray_style . '>
                        <div class="panel-heading"' . $gray_head . '>
                            <div class="row">
                                <div class="col-xs-3"><i class="fa ' . $s['icon'] . ' fa-4x"></i></div>
                                <div class="col-xs-9 text-right">
                                    <div style="font-size:20px; font-weight:bold;">' . trim($vals) . '</div>
                                    <div>' . htmlspecialchars($s['name']) . '</div>
                                    <div><span class="label ' . $label_class . '">' . htmlspecialchars($s['status_label']) . '</span> <small>' . $age_str . '</small></div>
                                </div>
                            </div>
                        </div>
                        <a href="#section-' . $key . '">
                            <div class="panel-footer">
                                <span class="pull-left">View Details</span>
                                <span class="pull-right"><i class="fa fa-arrow-circle-right"></i></span>
                                <div class="clearfix"></div>
                            </div>
                        </a>
                    </div>
                </div>';
}
?>
            </div>

            <hr>
            <h3><i class="fa fa-heartbeat"></i> Sensor Reporting Details</h3>

            <!-- Hive Temp Section -->
<?PHP
render_sensor_section('hivetemp', $sensors['hivetemp'], $stats['hivetemp'], $rh_summary, [
    'Avg Temp' => is_numeric($stats['hivetemp']['avg_temp']) ? $stats['hivetemp']['avg_temp'] . $temp_unit : '--',
    'Max / Min' => (is_numeric($stats['hivetemp']['max_temp']) ? $stats['hivetemp']['max_temp'] : '--') . ' / ' . (is_numeric($stats['hivetemp']['min_temp']) ? $stats['hivetemp']['min_temp'] : '--') . $temp_unit,
    'Avg Humidity' => is_numeric($stats['hivetemp']['avg_hum']) ? $stats['hivetemp']['avg_hum'] . '%' : '--',
    'Readings' => intval($stats['hivetemp']['readings']),
], 'chart-hivetemp');
?>

            <!-- Weight Section -->
<?PHP
$diff = $stats['weight']['diff_wt'];
render_sensor_section('weight', $sensors['weight'], $stats['weight'], $rh_summary, [
    'Avg Weight' => is_numeric($stats['weight']['avg_wt']) ? $stats['weight']['avg_wt'] . ' ' . $weight_unit : '--',
    'Max / Min' => (is_numeric($stats['weight']['max_wt']) ? $stats['weight']['max_wt'] : '--') . ' / ' . (is_numeric($stats['weight']['min_wt']) ? $stats['weight']['min_wt'] : '--') . ' ' . $weight_unit,
    'Gain/Loss' => ($diff !== null ? ($diff >= 0 ? '+' : '') . $diff . ' ' . $weight_unit : '--'),
    'Readings' => intval($stats['weight']['readings']),
], 'chart-weight');
?>

            <!-- Weather Section -->
<?PHP
$wx_enabled = !empty($config['WEATHER_LEVEL']) && $config['WEATHER_LEVEL'] !== 'none';
if ($wx_enabled) {
    render_sensor_section('weather', $sensors['weather'], $stats['weather'], $rh_summary, [
        'Avg Temp' => is_numeric($stats['weather']['avg_temp']) ? $stats['weather']['avg_temp'] . $temp_unit : '--',
        'Max / Min' => (is_numeric($stats['weather']['max_temp']) ? $stats['weather']['max_temp'] : '--') . ' / ' . (is_numeric($stats['weather']['min_temp']) ? $stats['weather']['min_temp'] : '--') . $temp_unit,
        'Avg Humidity' => is_numeric($stats['weather']['avg_hum']) ? $stats['weather']['avg_hum'] . '%' : '--',
        'Provider' => htmlspecialchars($config['WEATHER_LEVEL']),
    ], 'chart-weather');
?>

            <!-- Weather Provider Detail (collapsible) -->
            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-heading" style="cursor:pointer" data-toggle="collapse" data-target="#weather-detail">
                            <i class="fa fa-bar-chart fa-fw"></i> Weather Provider Statistics
                            <span class="pull-right"><i class="fa fa-chevron-down"></i></span>
                        </div>
                        <div id="weather-detail" class="panel-body collapse">
                            <div class="row" style="margin-bottom: 15px;">
                                <div class="col-md-3 col-sm-6">
                                    <div class="panel panel-primary"><div class="panel-heading"><div class="row"><div class="col-xs-3"><i class="fa fa-cloud fa-3x"></i></div><div class="col-xs-9 text-right"><div style="font-size:20px"><?PHP echo htmlspecialchars($config['WEATHER_LEVEL'] ?: 'None'); ?></div><div>Primary</div></div></div></div></div>
                                </div>
                                <div class="col-md-3 col-sm-6">
                                    <div class="panel panel-green"><div class="panel-heading"><div class="row"><div class="col-xs-3"><i class="fa fa-refresh fa-3x"></i></div><div class="col-xs-9 text-right"><div style="font-size:20px"><?PHP echo $primary_pct; ?>%</div><div>Primary Success</div></div></div></div></div>
                                </div>
                                <div class="col-md-3 col-sm-6">
                                    <div class="panel panel-yellow"><div class="panel-heading"><div class="row"><div class="col-xs-3"><i class="fa fa-exchange fa-3x"></i></div><div class="col-xs-9 text-right"><div style="font-size:20px"><?PHP echo $fallback_pct; ?>%</div><div>Fallback Used</div></div></div></div></div>
                                </div>
                                <div class="col-md-3 col-sm-6">
                                    <div class="panel panel-red"><div class="panel-heading"><div class="row"><div class="col-xs-3"><i class="fa fa-clock-o fa-3x"></i></div><div class="col-xs-9 text-right"><div style="font-size:20px"><?PHP echo htmlspecialchars($config['WX_MAX_STALE_MINUTES'] ?: '120'); ?> min</div><div>Stale Threshold</div></div></div></div></div>
                                </div>
                            </div>
                            <div class="table-responsive">
                                <table class="table table-striped table-bordered table-hover">
                                    <thead><tr><th>Provider</th><th>Total Calls</th><th>Successes</th><th>Success Rate</th><th>Avg Response</th><th>Avg Obs Age</th><th>Last Seen</th></tr></thead>
                                    <tbody>
<?PHP
if (empty($provider_stats)) {
    echo '<tr><td colspan="7" class="text-center text-muted">No weather health data recorded yet.</td></tr>';
} else {
    foreach ($provider_stats as $stat) {
        $rate = floatval($stat['success_rate']);
        $badge = $rate >= 95 ? 'success' : ($rate >= 80 ? 'warning' : 'danger');
        echo '<tr><td><strong>' . htmlspecialchars($stat['provider']) . '</strong></td><td>' . htmlspecialchars($stat['total_calls']) . '</td><td>' . htmlspecialchars($stat['successes']) . '</td><td><span class="label label-' . $badge . '">' . htmlspecialchars($stat['success_rate']) . '%</span></td><td>' . htmlspecialchars($stat['avg_response_ms']) . ' ms</td><td>' . ($stat['avg_obs_age'] !== null ? htmlspecialchars($stat['avg_obs_age']) . ' min' : '-') . '</td><td>' . htmlspecialchars($stat['last_seen']) . '</td></tr>';
    }
}
?>
                                    </tbody>
                                </table>
                            </div>
                            <div class="row" style="margin-top:15px;">
                                <div class="col-lg-6"><div id="wx-success-chart" style="height:250px;"></div></div>
                                <div class="col-lg-6"><div id="wx-response-chart" style="height:250px;"></div></div>
                            </div>
<?PHP if (!empty($recent_failures)) { ?>
                            <h4 style="margin-top:20px;"><i class="fa fa-exclamation-triangle"></i> Recent Failures</h4>
                            <div class="table-responsive">
                                <table class="table table-striped table-bordered table-hover" id="failures-table">
                                    <thead><tr><th>Time</th><th>Provider</th><th>Role</th><th>Error</th><th>Obs Age</th><th>Response</th></tr></thead>
                                    <tbody>
<?PHP foreach ($recent_failures as $fail) {
    echo '<tr><td>' . htmlspecialchars($fail['timestamp']) . '</td><td>' . htmlspecialchars($fail['provider']) . '</td><td><span class="label label-default">' . htmlspecialchars($fail['role']) . '</span></td><td>' . htmlspecialchars($fail['error_reason'] ?: '-') . '</td><td>' . htmlspecialchars($fail['observation_age_minutes']) . ' min</td><td>' . htmlspecialchars($fail['response_ms']) . ' ms</td></tr>';
} ?>
                                    </tbody>
                                </table>
                            </div>
<?PHP } ?>
                        </div>
                    </div>
                </div>
            </div>
<?PHP } ?>

            <!-- Light Section -->
<?PHP
render_sensor_section('light', $sensors['light'], $stats['light'], $rh_summary, [
    'Avg Solar' => is_numeric($stats['light']['avg_solar']) ? $stats['light']['avg_solar'] . ' W/m²' : '--',
    'Max Solar' => is_numeric($stats['light']['max_solar']) ? $stats['light']['max_solar'] . ' W/m²' : '--',
    'Avg Lux' => is_numeric($stats['light']['avg_lux']) ? $stats['light']['avg_lux'] . ' lx' : '--',
    'Readings' => intval($stats['light']['readings']),
], 'chart-light');
?>

            <!-- Bee Counter Section -->
<?PHP
render_sensor_section('beecount', $sensors['beecount'], $stats['beecount'], $rh_summary, [
    'Total In' => is_numeric($stats['beecount']['total_in']) ? number_format($stats['beecount']['total_in']) : '--',
    'Total Out' => is_numeric($stats['beecount']['total_out']) ? number_format($stats['beecount']['total_out']) : '--',
    'Max In' => is_numeric($stats['beecount']['max_in']) ? $stats['beecount']['max_in'] : '--',
    'Readings' => intval($stats['beecount']['readings']),
], 'chart-beecount');
?>

            <!-- Air Quality Section -->
<?PHP
render_sensor_section('air', $sensors['air'], $stats['air'], $rh_summary, [
    'Avg PM2.5' => is_numeric($stats['air']['avg_pm25']) ? $stats['air']['avg_pm25'] . ' µg/m³' : '--',
    'Max PM2.5' => is_numeric($stats['air']['max_pm25']) ? $stats['air']['max_pm25'] . ' µg/m³' : '--',
    'Avg AQI' => is_numeric($stats['air']['avg_aqi']) ? $stats['air']['avg_aqi'] : '--',
    'Readings' => intval($stats['air']['readings']),
], 'chart-air');
?>

            <!-- EPA Section -->
<?PHP
render_sensor_section('epa', $sensors['epa'], $stats['epa'] ?? ['readings' => 0], $rh_summary, [
    'Avg O3 AQI' => is_numeric($stats['epa']['avg_o3'] ?? null) ? $stats['epa']['avg_o3'] : '--',
    'Max O3 AQI' => is_numeric($stats['epa']['max_o3'] ?? null) ? $stats['epa']['max_o3'] : '--',
    'Avg NO2 AQI' => is_numeric($stats['epa']['avg_no2'] ?? null) ? $stats['epa']['avg_no2'] : '--',
    'Readings' => intval($stats['epa']['readings'] ?? 0),
], 'chart-epa');
?>

            <!-- Pollen Section -->
<?PHP
render_sensor_section('pollen', $sensors['pollen'], $stats['pollen'], $rh_summary, [
    'Avg Level' => is_numeric($stats['pollen']['avg_pollen']) ? $stats['pollen']['avg_pollen'] . ' / 12' : '--',
    'Max / Min' => (is_numeric($stats['pollen']['max_pollen']) ? $stats['pollen']['max_pollen'] : '--') . ' / ' . (is_numeric($stats['pollen']['min_pollen']) ? $stats['pollen']['min_pollen'] : '--'),
    'Readings' => intval($stats['pollen']['readings']),
], 'chart-pollen');
?>

            <!-- GDD Section -->
<?PHP
render_sensor_section('gdd', $sensors['gdd'], $stats['gdd'], $rh_summary, [
    'Avg Daily GDD' => is_numeric($stats['gdd']['avg_gdd']) ? $stats['gdd']['avg_gdd'] : '--',
    'Season Total' => is_numeric($stats['gdd']['max_season']) ? $stats['gdd']['max_season'] : '--',
    'Days Tracked' => intval($stats['gdd']['readings']),
], 'chart-gdd');
?>

    </div>

    <script src="../bower_components/jquery/dist/jquery.min.js"></script>
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>
    <script src="../bower_components/datatables/media/js/jquery.dataTables.min.js"></script>
    <script src="../bower_components/datatables-plugins/integration/bootstrap/3/dataTables.bootstrap.min.js"></script>
    <script src="../dist/js/sb-admin-2.js"></script>
    <script src="/js/highcharts/highcharts.js"></script>
    <script src="/js/highcharts/modules/exporting.js"></script>

    <script>
    $(document).ready(function() {
        if ($('#failures-table tbody tr td').length > 1) {
            $('#failures-table').DataTable({ responsive: true, order: [[0, 'desc']], pageLength: 15 });
        }

        function reportingChart(container, title, data) {
            if (!document.getElementById(container) || data.length === 0) return;
            Highcharts.chart(container, {
                chart: { type: 'column', height: 180, marginRight: 10 },
                title: { text: 'Reporting Rate', style: { fontSize: '13px' } },
                xAxis: { type: 'datetime', labels: { style: { fontSize: '10px' } } },
                yAxis: {
                    title: { text: null },
                    min: 0, max: 100,
                    labels: { format: '{value}%', style: { fontSize: '10px' } },
                    plotBands: [
                        { from: 0, to: 50, color: 'rgba(255,0,0,0.05)' },
                        { from: 50, to: 75, color: 'rgba(255,165,0,0.05)' },
                        { from: 75, to: 100, color: 'rgba(0,128,0,0.05)' }
                    ]
                },
                legend: { enabled: false },
                tooltip: { valueSuffix: '%', pointFormat: 'Reporting: <b>{point.y}%</b>' },
                plotOptions: {
                    column: {
                        borderWidth: 0,
                        groupPadding: 0.05,
                        pointPadding: 0,
                        colorByPoint: false,
                        zones: [
                            { value: 50, color: '#d9534f' },
                            { value: 75, color: '#f0ad4e' },
                            { color: '#5cb85c' }
                        ]
                    }
                },
                exporting: { enabled: false },
                series: [{ name: 'Reporting', data: data }]
            });
        }

<?PHP
$chart_map = [
    'hivetemp' => 'chart-hivetemp', 'weight' => 'chart-weight', 'weather' => 'chart-weather',
    'light' => 'chart-light', 'beecount' => 'chart-beecount', 'air' => 'chart-air',
    'epa' => 'chart-epa', 'pollen' => 'chart-pollen', 'gdd' => 'chart-gdd'
];
foreach ($chart_map as $key => $container) {
    $data = isset($reporting[$key]) ? json_encode($reporting[$key]) : '[]';
    $name = $sensors[$key]['name'];
    echo "        reportingChart('$container', " . json_encode($name) . ", $data);\n";
}
?>

        // Weather provider charts (lazy render on expand)
<?PHP if (!empty($chart_providers)) {
    $colors = ['#2196F3', '#4CAF50', '#FF9800', '#9C27B0', '#F44336', '#00BCD4'];
?>
        $('#weather-detail').on('shown.bs.collapse', function() {
            if (!window._wxChartsRendered) {
                window._wxChartsRendered = true;
                Highcharts.chart('wx-success-chart', {
                    chart: { type: 'line', zoomType: 'x' }, title: { text: 'Success Rate' },
                    xAxis: { type: 'datetime' }, yAxis: { title: { text: '%' }, min: 0, max: 100 },
                    tooltip: { shared: true, valueSuffix: '%' },
                    series: [<?PHP $ci = 0; foreach ($chart_providers as $prov => $data) { echo "{ name: " . json_encode($prov) . ", data: " . json_encode($data) . ", color: '{$colors[$ci % count($colors)]}' },"; $ci++; } ?>]
                });
                Highcharts.chart('wx-response-chart', {
                    chart: { type: 'line', zoomType: 'x' }, title: { text: 'Response Time' },
                    xAxis: { type: 'datetime' }, yAxis: { title: { text: 'ms' }, min: 0 },
                    tooltip: { shared: true, valueSuffix: ' ms' },
                    series: [<?PHP $ci = 0; foreach ($chart_response as $prov => $data) { echo "{ name: " . json_encode($prov) . ", data: " . json_encode($data) . ", color: '{$colors[$ci % count($colors)]}' },"; $ci++; } ?>]
                });
            }
        });
<?PHP } ?>

        // Smooth scroll for card links
        $('a[href^="#section-"]').on('click', function(e) {
            e.preventDefault();
            var target = $(this.getAttribute('href'));
            if (target.length) {
                $('html, body').animate({ scrollTop: target.offset().top - 60 }, 400);
            }
        });
    });
    </script>

<?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>
</body>
</html>
