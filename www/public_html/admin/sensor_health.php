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

// Weather provider stats (from existing weather_health.php)
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

// Sparkline trend data (last N days, thinned to ~100 points per sensor)
$sparkline_period = "-$period days";
$spark_sth = $conn->prepare("SELECT hivetempf, hivetempc, hiveweight, weather_tempf, weather_tempc, solarradiation, lux, OUT_COUNT, COALESCE(air_pm2_5_raw, air_pm2_5) as pm25, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', :p) ORDER BY datetime ASC");
$spark_sth->execute([':p' => $sparkline_period]);
$spark_rows = $spark_sth->fetchAll(PDO::FETCH_ASSOC);

$spark_count = count($spark_rows);
$spark_step = ($spark_count > 100) ? intval(ceil($spark_count / 100)) : 1;

$spark_hivetemp = []; $spark_weight = []; $spark_wxtemp = [];
$spark_solar = []; $spark_flights = []; $spark_pm25 = [];
$idx = 0;
foreach ($spark_rows as $r) {
    $idx++;
    if ($spark_step > 1 && ($idx % $spark_step !== 0) && $idx !== 1 && $idx !== $spark_count) continue;
    $ts = $r['datetime'];
    $tc = $is_metric ? 'hivetempc' : 'hivetempf';
    $wc = $is_metric ? 'weather_tempc' : 'weather_tempf';

    $v = is_numeric($r[$tc]) ? floatval($r[$tc]) : null;
    if ($v !== null && $v != 0) $spark_hivetemp[] = "[$ts,$v]";

    $v = is_numeric($r['hiveweight']) ? floatval($r['hiveweight']) : null;
    if ($is_metric && $v !== null) $v = round($v * 0.453592, 2);
    if ($v !== null && $v != 0) $spark_weight[] = "[$ts,$v]";

    $v = is_numeric($r[$wc]) ? floatval($r[$wc]) : null;
    if ($v !== null && $v != 0) $spark_wxtemp[] = "[$ts,$v]";

    $v = is_numeric($r['solarradiation']) ? floatval($r['solarradiation']) : null;
    if ($v !== null && $v != 0) $spark_solar[] = "[$ts,$v]";

    $v = is_numeric($r['OUT_COUNT']) ? intval($r['OUT_COUNT']) : null;
    if ($v !== null && $v != 0) $spark_flights[] = "[$ts,$v]";

    $v = is_numeric($r['pm25']) ? floatval($r['pm25']) : null;
    if ($v !== null) $spark_pm25[] = "[$ts,$v]";
}

// GDD sparkline
$gdd_spark_sth = $conn->prepare("SELECT seasongdd, strftime('%s',gdddate)*1000 AS datetime FROM gdd WHERE gdddate > datetime('now', 'localtime', :p) ORDER BY datetime ASC");
$gdd_spark_sth->execute([':p' => $sparkline_period]);
$gdd_spark_rows = $gdd_spark_sth->fetchAll(PDO::FETCH_ASSOC);
$spark_gdd = [];
foreach ($gdd_spark_rows as $r) {
    $v = is_numeric($r['seasongdd']) ? floatval($r['seasongdd']) : null;
    if ($v !== null) $spark_gdd[] = "[" . $r['datetime'] . ",$v]";
}

// Pollen sparkline
$pollen_spark_sth = $conn->prepare("SELECT pollenlevel, strftime('%s',date)*1000 AS datetime FROM pollen WHERE date >= datetime('now', 'localtime', :p) ORDER BY datetime ASC");
$pollen_spark_sth->execute([':p' => $sparkline_period]);
$pollen_spark_rows = $pollen_spark_sth->fetchAll(PDO::FETCH_ASSOC);
$spark_pollen = [];
foreach ($pollen_spark_rows as $r) {
    $v = is_numeric($r['pollenlevel']) ? floatval($r['pollenlevel']) : null;
    if ($v !== null) $spark_pollen[] = "[" . $r['datetime'] . ",$v]";
}

// EPA sparkline
$spark_epa_o3 = [];
try {
    $epa_spark_sth = $conn->prepare("SELECT o3_aqi, strftime('%s',date)*1000 AS datetime FROM airquality_epa WHERE date > datetime('now', 'localtime', :p) ORDER BY datetime ASC");
    $epa_spark_sth->execute([':p' => $sparkline_period]);
    $epa_spark_rows = $epa_spark_sth->fetchAll(PDO::FETCH_ASSOC);
    foreach ($epa_spark_rows as $r) {
        $v = is_numeric($r['o3_aqi']) ? intval($r['o3_aqi']) : null;
        if ($v !== null) $spark_epa_o3[] = "[" . $r['datetime'] . ",$v]";
    }
} catch (PDOException $e) {}

$temp_unit = $is_metric ? '°C' : '°F';
$weight_unit = $is_metric ? 'kg' : 'lb';
?>
<!DOCTYPE html>
<html lang="en">

    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->
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
    echo '
                <div class="col-lg-4 col-md-6">
                    <div class="panel ' . $panel_class . '">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3"><i class="fa ' . $s['icon'] . ' fa-4x"></i></div>
                                <div class="col-xs-9 text-right">
                                    <div style="font-size:20px; font-weight:bold;">' . trim($vals) . '</div>
                                    <div>' . htmlspecialchars($s['name']) . '</div>
                                    <div><span class="label ' . $label_class . '">' . htmlspecialchars($s['status_label']) . '</span> <small>' . $age_str . '</small></div>
                                </div>
                            </div>
                        </div>
                        <a href="' . htmlspecialchars($s['detail_url']) . '">
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

            <!-- Per-Sensor Detail Sections -->

            <!-- Hive Temp/Humidity -->
<?PHP if ($sensors['hivetemp']['enabled']) { ?>
            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-thermometer-half fa-fw"></i> Hive Temperature &amp; Humidity
                            <span class="label <?PHP echo status_label_class($sensors['hivetemp']['status']); ?> pull-right"><?PHP echo htmlspecialchars($sensors['hivetemp']['status_label']); ?></span>
                        </div>
                        <div class="panel-body">
                            <div id="spark-hivetemp" style="height:200px;"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats (<?PHP echo $period; ?> day<?PHP echo $period !== '1' ? 's' : ''; ?>)</div>
                        <div class="panel-body">
                            <table class="table table-condensed table-striped">
                                <tr><td>Avg Temp</td><td><?PHP echo is_numeric($stats['hivetemp']['avg_temp']) ? $stats['hivetemp']['avg_temp'] . $temp_unit : '--'; ?></td></tr>
                                <tr><td>Max / Min</td><td><?PHP echo (is_numeric($stats['hivetemp']['max_temp']) ? $stats['hivetemp']['max_temp'] : '--') . ' / ' . (is_numeric($stats['hivetemp']['min_temp']) ? $stats['hivetemp']['min_temp'] : '--') . $temp_unit; ?></td></tr>
                                <tr><td>Avg Humidity</td><td><?PHP echo is_numeric($stats['hivetemp']['avg_hum']) ? $stats['hivetemp']['avg_hum'] . '%' : '--'; ?></td></tr>
                                <tr><td>Max / Min Hum</td><td><?PHP echo (is_numeric($stats['hivetemp']['max_hum']) ? $stats['hivetemp']['max_hum'] : '--') . ' / ' . (is_numeric($stats['hivetemp']['min_hum']) ? $stats['hivetemp']['min_hum'] : '--') . '%'; ?></td></tr>
                                <tr><td>Readings</td><td><?PHP echo intval($stats['hivetemp']['readings']); ?></td></tr>
                            </table>
                        </div>
                    </div>
                </div>
            </div>
<?PHP } ?>

            <!-- Weight -->
<?PHP if ($sensors['weight']['enabled']) { ?>
            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-balance-scale fa-fw"></i> Hive Weight
                            <span class="label <?PHP echo status_label_class($sensors['weight']['status']); ?> pull-right"><?PHP echo htmlspecialchars($sensors['weight']['status_label']); ?></span>
                        </div>
                        <div class="panel-body">
                            <div id="spark-weight" style="height:200px;"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats (<?PHP echo $period; ?> day<?PHP echo $period !== '1' ? 's' : ''; ?>)</div>
                        <div class="panel-body">
                            <table class="table table-condensed table-striped">
                                <tr><td>Avg Weight</td><td><?PHP echo is_numeric($stats['weight']['avg_wt']) ? $stats['weight']['avg_wt'] . ' ' . $weight_unit : '--'; ?></td></tr>
                                <tr><td>Max / Min</td><td><?PHP echo (is_numeric($stats['weight']['max_wt']) ? $stats['weight']['max_wt'] : '--') . ' / ' . (is_numeric($stats['weight']['min_wt']) ? $stats['weight']['min_wt'] : '--') . ' ' . $weight_unit; ?></td></tr>
                                <tr><td>Gain/Loss</td><td><?PHP $d = $stats['weight']['diff_wt']; echo ($d !== null ? ($d >= 0 ? '+' : '') . $d . ' ' . $weight_unit : '--'); ?></td></tr>
                                <tr><td>Readings</td><td><?PHP echo intval($stats['weight']['readings']); ?></td></tr>
                            </table>
                        </div>
                    </div>
                </div>
            </div>
<?PHP } ?>

            <!-- Weather Station -->
<?PHP $wx_enabled = !empty($config['WEATHER_LEVEL']) && $config['WEATHER_LEVEL'] !== 'none';
if ($wx_enabled) { ?>
            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-cloud fa-fw"></i> Weather Station (<?PHP echo htmlspecialchars($config['WEATHER_LEVEL']); ?>)
                            <span class="label <?PHP echo status_label_class($sensors['weather']['status']); ?> pull-right"><?PHP echo htmlspecialchars($sensors['weather']['status_label']); ?></span>
                        </div>
                        <div class="panel-body">
                            <div id="spark-weather" style="height:200px;"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats (<?PHP echo $period; ?> day<?PHP echo $period !== '1' ? 's' : ''; ?>)</div>
                        <div class="panel-body">
                            <table class="table table-condensed table-striped">
                                <tr><td>Avg Temp</td><td><?PHP echo is_numeric($stats['weather']['avg_temp']) ? $stats['weather']['avg_temp'] . $temp_unit : '--'; ?></td></tr>
                                <tr><td>Max / Min</td><td><?PHP echo (is_numeric($stats['weather']['max_temp']) ? $stats['weather']['max_temp'] : '--') . ' / ' . (is_numeric($stats['weather']['min_temp']) ? $stats['weather']['min_temp'] : '--') . $temp_unit; ?></td></tr>
                                <tr><td>Avg Humidity</td><td><?PHP echo is_numeric($stats['weather']['avg_hum']) ? $stats['weather']['avg_hum'] . '%' : '--'; ?></td></tr>
                                <tr><td>Readings</td><td><?PHP echo intval($stats['weather']['readings']); ?></td></tr>
                            </table>
                        </div>
                    </div>
                </div>
            </div>

            <!-- Weather Provider Detail (collapsible) -->
            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-heading" style="cursor:pointer" data-toggle="collapse" data-target="#weather-detail">
                            <i class="fa fa-bar-chart fa-fw"></i> Weather Provider Details
                            <span class="pull-right"><i class="fa fa-chevron-down"></i></span>
                        </div>
                        <div id="weather-detail" class="panel-body collapse">
                            <!-- Summary cards -->
                            <div class="row" style="margin-bottom: 15px;">
                                <div class="col-md-3 col-sm-6">
                                    <div class="panel panel-primary">
                                        <div class="panel-heading"><div class="row"><div class="col-xs-3"><i class="fa fa-cloud fa-3x"></i></div><div class="col-xs-9 text-right"><div style="font-size:20px"><?PHP echo htmlspecialchars($config['WEATHER_LEVEL'] ?: 'None'); ?></div><div>Primary</div></div></div></div>
                                    </div>
                                </div>
                                <div class="col-md-3 col-sm-6">
                                    <div class="panel panel-green">
                                        <div class="panel-heading"><div class="row"><div class="col-xs-3"><i class="fa fa-refresh fa-3x"></i></div><div class="col-xs-9 text-right"><div style="font-size:20px"><?PHP echo $primary_pct; ?>%</div><div>Primary Success</div></div></div></div>
                                    </div>
                                </div>
                                <div class="col-md-3 col-sm-6">
                                    <div class="panel panel-yellow">
                                        <div class="panel-heading"><div class="row"><div class="col-xs-3"><i class="fa fa-exchange fa-3x"></i></div><div class="col-xs-9 text-right"><div style="font-size:20px"><?PHP echo $fallback_pct; ?>%</div><div>Fallback Used</div></div></div></div>
                                    </div>
                                </div>
                                <div class="col-md-3 col-sm-6">
                                    <div class="panel panel-red">
                                        <div class="panel-heading"><div class="row"><div class="col-xs-3"><i class="fa fa-clock-o fa-3x"></i></div><div class="col-xs-9 text-right"><div style="font-size:20px"><?PHP echo htmlspecialchars($config['WX_MAX_STALE_MINUTES'] ?: '120'); ?> min</div><div>Stale Threshold</div></div></div></div>
                                    </div>
                                </div>
                            </div>
                            <!-- Provider Stats Table -->
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
        echo '<tr>';
        echo '<td><strong>' . htmlspecialchars($stat['provider']) . '</strong></td>';
        echo '<td>' . htmlspecialchars($stat['total_calls']) . '</td>';
        echo '<td>' . htmlspecialchars($stat['successes']) . '</td>';
        echo '<td><span class="label label-' . $badge . '">' . htmlspecialchars($stat['success_rate']) . '%</span></td>';
        echo '<td>' . htmlspecialchars($stat['avg_response_ms']) . ' ms</td>';
        echo '<td>' . ($stat['avg_obs_age'] !== null ? htmlspecialchars($stat['avg_obs_age']) . ' min' : '-') . '</td>';
        echo '<td>' . htmlspecialchars($stat['last_seen']) . '</td>';
        echo '</tr>';
    }
}
?>
                                    </tbody>
                                </table>
                            </div>
                            <!-- Charts -->
                            <div class="row" style="margin-top: 15px;">
                                <div class="col-lg-6"><div id="wx-success-chart" style="height: 250px;"></div></div>
                                <div class="col-lg-6"><div id="wx-response-chart" style="height: 250px;"></div></div>
                            </div>
                            <!-- Recent Failures -->
<?PHP if (!empty($recent_failures)) { ?>
                            <h4 style="margin-top: 20px;"><i class="fa fa-exclamation-triangle"></i> Recent Failures</h4>
                            <div class="table-responsive">
                                <table class="table table-striped table-bordered table-hover" id="failures-table">
                                    <thead><tr><th>Time</th><th>Provider</th><th>Role</th><th>Error</th><th>Obs Age</th><th>Response</th></tr></thead>
                                    <tbody>
<?PHP foreach ($recent_failures as $fail) {
    echo '<tr>';
    echo '<td>' . htmlspecialchars($fail['timestamp']) . '</td>';
    echo '<td>' . htmlspecialchars($fail['provider']) . '</td>';
    echo '<td><span class="label label-default">' . htmlspecialchars($fail['role']) . '</span></td>';
    echo '<td>' . htmlspecialchars($fail['error_reason'] ?: '-') . '</td>';
    echo '<td>' . htmlspecialchars($fail['observation_age_minutes']) . ' min</td>';
    echo '<td>' . htmlspecialchars($fail['response_ms']) . ' ms</td>';
    echo '</tr>';
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

            <!-- Light/Solar -->
<?PHP if ($sensors['light']['enabled']) { ?>
            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-sun-o fa-fw"></i> Light / Solar
                            <span class="label <?PHP echo status_label_class($sensors['light']['status']); ?> pull-right"><?PHP echo htmlspecialchars($sensors['light']['status_label']); ?></span>
                        </div>
                        <div class="panel-body">
                            <div id="spark-solar" style="height:200px;"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats (<?PHP echo $period; ?> day<?PHP echo $period !== '1' ? 's' : ''; ?>)</div>
                        <div class="panel-body">
                            <table class="table table-condensed table-striped">
                                <tr><td>Avg Solar</td><td><?PHP echo is_numeric($stats['light']['avg_solar']) ? $stats['light']['avg_solar'] . ' W/m²' : '--'; ?></td></tr>
                                <tr><td>Max / Min Solar</td><td><?PHP echo (is_numeric($stats['light']['max_solar']) ? $stats['light']['max_solar'] : '--') . ' / ' . (is_numeric($stats['light']['min_solar']) ? $stats['light']['min_solar'] : '--') . ' W/m²'; ?></td></tr>
                                <tr><td>Avg Lux</td><td><?PHP echo is_numeric($stats['light']['avg_lux']) ? $stats['light']['avg_lux'] . ' lx' : '--'; ?></td></tr>
                                <tr><td>Max / Min Lux</td><td><?PHP echo (is_numeric($stats['light']['max_lux']) ? $stats['light']['max_lux'] : '--') . ' / ' . (is_numeric($stats['light']['min_lux']) ? $stats['light']['min_lux'] : '--') . ' lx'; ?></td></tr>
                                <tr><td>Readings</td><td><?PHP echo intval($stats['light']['readings']); ?></td></tr>
                            </table>
                        </div>
                    </div>
                </div>
            </div>
<?PHP } ?>

            <!-- Bee Counter -->
<?PHP if ($sensors['beecount']['enabled']) { ?>
            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-bug fa-fw"></i> Bee Counter
                            <span class="label <?PHP echo status_label_class($sensors['beecount']['status']); ?> pull-right"><?PHP echo htmlspecialchars($sensors['beecount']['status_label']); ?></span>
                        </div>
                        <div class="panel-body">
                            <div id="spark-flights" style="height:200px;"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats (<?PHP echo $period; ?> day<?PHP echo $period !== '1' ? 's' : ''; ?>)</div>
                        <div class="panel-body">
                            <table class="table table-condensed table-striped">
                                <tr><td>Total In</td><td><?PHP echo is_numeric($stats['beecount']['total_in']) ? number_format($stats['beecount']['total_in']) : '--'; ?></td></tr>
                                <tr><td>Total Out</td><td><?PHP echo is_numeric($stats['beecount']['total_out']) ? number_format($stats['beecount']['total_out']) : '--'; ?></td></tr>
                                <tr><td>Max In (single)</td><td><?PHP echo is_numeric($stats['beecount']['max_in']) ? $stats['beecount']['max_in'] : '--'; ?></td></tr>
                                <tr><td>Max Out (single)</td><td><?PHP echo is_numeric($stats['beecount']['max_out']) ? $stats['beecount']['max_out'] : '--'; ?></td></tr>
                                <tr><td>Readings</td><td><?PHP echo intval($stats['beecount']['readings']); ?></td></tr>
                            </table>
                        </div>
                    </div>
                </div>
            </div>
<?PHP } ?>

            <!-- Air Quality -->
<?PHP if ($sensors['air']['enabled']) { ?>
            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-industry fa-fw"></i> Air Quality
                            <span class="label <?PHP echo status_label_class($sensors['air']['status']); ?> pull-right"><?PHP echo htmlspecialchars($sensors['air']['status_label']); ?></span>
                        </div>
                        <div class="panel-body">
                            <div id="spark-pm25" style="height:200px;"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats (<?PHP echo $period; ?> day<?PHP echo $period !== '1' ? 's' : ''; ?>)</div>
                        <div class="panel-body">
                            <table class="table table-condensed table-striped">
                                <tr><td>Avg PM2.5</td><td><?PHP echo is_numeric($stats['air']['avg_pm25']) ? $stats['air']['avg_pm25'] . ' µg/m³' : '--'; ?></td></tr>
                                <tr><td>Max / Min PM2.5</td><td><?PHP echo (is_numeric($stats['air']['max_pm25']) ? $stats['air']['max_pm25'] : '--') . ' / ' . (is_numeric($stats['air']['min_pm25']) ? $stats['air']['min_pm25'] : '--') . ' µg/m³'; ?></td></tr>
                                <tr><td>Avg AQI</td><td><?PHP echo is_numeric($stats['air']['avg_aqi']) ? $stats['air']['avg_aqi'] : '--'; ?></td></tr>
                                <tr><td>Max AQI</td><td><?PHP echo is_numeric($stats['air']['max_aqi']) ? $stats['air']['max_aqi'] : '--'; ?></td></tr>
                                <tr><td>Readings</td><td><?PHP echo intval($stats['air']['readings']); ?></td></tr>
                            </table>
                        </div>
                    </div>
                </div>
            </div>
<?PHP } ?>

            <!-- EPA AirNow -->
<?PHP if ($sensors['epa']['enabled']) { ?>
            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-leaf fa-fw"></i> EPA AirNow
                            <span class="label <?PHP echo status_label_class($sensors['epa']['status']); ?> pull-right"><?PHP echo htmlspecialchars($sensors['epa']['status_label']); ?></span>
                        </div>
                        <div class="panel-body">
                            <div id="spark-epa" style="height:200px;"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats (<?PHP echo $period; ?> day<?PHP echo $period !== '1' ? 's' : ''; ?>)</div>
                        <div class="panel-body">
                            <table class="table table-condensed table-striped">
                                <tr><td>Avg O3 AQI</td><td><?PHP echo is_numeric($stats['epa']['avg_o3'] ?? null) ? $stats['epa']['avg_o3'] : '--'; ?></td></tr>
                                <tr><td>Max O3 AQI</td><td><?PHP echo is_numeric($stats['epa']['max_o3'] ?? null) ? $stats['epa']['max_o3'] : '--'; ?></td></tr>
                                <tr><td>Avg NO2 AQI</td><td><?PHP echo is_numeric($stats['epa']['avg_no2'] ?? null) ? $stats['epa']['avg_no2'] : '--'; ?></td></tr>
                                <tr><td>Max NO2 AQI</td><td><?PHP echo is_numeric($stats['epa']['max_no2'] ?? null) ? $stats['epa']['max_no2'] : '--'; ?></td></tr>
                                <tr><td>Readings</td><td><?PHP echo intval($stats['epa']['readings'] ?? 0); ?></td></tr>
                            </table>
                        </div>
                    </div>
                </div>
            </div>
<?PHP } ?>

            <!-- Pollen -->
            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-pagelines fa-fw"></i> Pollen
                            <span class="label <?PHP echo status_label_class($sensors['pollen']['status']); ?> pull-right"><?PHP echo htmlspecialchars($sensors['pollen']['status_label']); ?></span>
                        </div>
                        <div class="panel-body">
                            <div id="spark-pollen" style="height:200px;"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats (<?PHP echo $period; ?> day<?PHP echo $period !== '1' ? 's' : ''; ?>)</div>
                        <div class="panel-body">
                            <table class="table table-condensed table-striped">
                                <tr><td>Avg Level</td><td><?PHP echo is_numeric($stats['pollen']['avg_pollen']) ? $stats['pollen']['avg_pollen'] . ' / 12' : '--'; ?></td></tr>
                                <tr><td>Max / Min</td><td><?PHP echo (is_numeric($stats['pollen']['max_pollen']) ? $stats['pollen']['max_pollen'] : '--') . ' / ' . (is_numeric($stats['pollen']['min_pollen']) ? $stats['pollen']['min_pollen'] : '--'); ?></td></tr>
                                <tr><td>Readings</td><td><?PHP echo intval($stats['pollen']['readings']); ?></td></tr>
                            </table>
                        </div>
                    </div>
                </div>
            </div>

            <!-- GDD -->
            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-tree fa-fw"></i> Growing Degree Days
                            <span class="label <?PHP echo status_label_class($sensors['gdd']['status']); ?> pull-right"><?PHP echo htmlspecialchars($sensors['gdd']['status_label']); ?></span>
                        </div>
                        <div class="panel-body">
                            <div id="spark-gdd" style="height:200px;"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats (<?PHP echo $period; ?> day<?PHP echo $period !== '1' ? 's' : ''; ?>)</div>
                        <div class="panel-body">
                            <table class="table table-condensed table-striped">
                                <tr><td>Avg Daily GDD</td><td><?PHP echo is_numeric($stats['gdd']['avg_gdd']) ? $stats['gdd']['avg_gdd'] : '--'; ?></td></tr>
                                <tr><td>Season Total</td><td><?PHP echo is_numeric($stats['gdd']['max_season']) ? $stats['gdd']['max_season'] : '--'; ?></td></tr>
                                <tr><td>Days Tracked</td><td><?PHP echo intval($stats['gdd']['readings']); ?></td></tr>
                            </table>
                        </div>
                    </div>
                </div>
            </div>

    </div>
    <!-- /#page-wrapper -->

    <!-- jQuery -->
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

        var sparkOpts = function(container, title, data, color, suffix) {
            return {
                chart: { renderTo: container, type: 'line', zoomType: 'x', marginRight: 10 },
                title: { text: null },
                xAxis: { type: 'datetime', labels: { style: { fontSize: '10px' } } },
                yAxis: { title: { text: title }, gridLineWidth: 1 },
                legend: { enabled: false },
                tooltip: { shared: true, valueSuffix: suffix || '' },
                plotOptions: { line: { marker: { enabled: false }, lineWidth: 2 } },
                exporting: { enabled: false },
                series: [{ name: title, data: data, color: color }]
            };
        };

        // Hive Temp sparkline
        var hiveTempData = [<?PHP echo implode(',', $spark_hivetemp); ?>];
        if (hiveTempData.length > 0 && document.getElementById('spark-hivetemp')) {
            new Highcharts.Chart(sparkOpts('spark-hivetemp', 'Hive Temp', hiveTempData, '#FF6347', ' <?PHP echo $temp_unit; ?>'));
        }

        // Weight sparkline
        var weightData = [<?PHP echo implode(',', $spark_weight); ?>];
        if (weightData.length > 0 && document.getElementById('spark-weight')) {
            new Highcharts.Chart(sparkOpts('spark-weight', 'Weight', weightData, '#4682B4', ' <?PHP echo $weight_unit; ?>'));
        }

        // Weather sparkline
        var wxTempData = [<?PHP echo implode(',', $spark_wxtemp); ?>];
        if (wxTempData.length > 0 && document.getElementById('spark-weather')) {
            new Highcharts.Chart(sparkOpts('spark-weather', 'Outside Temp', wxTempData, '#228B22', ' <?PHP echo $temp_unit; ?>'));
        }

        // Solar sparkline
        var solarData = [<?PHP echo implode(',', $spark_solar); ?>];
        if (solarData.length > 0 && document.getElementById('spark-solar')) {
            new Highcharts.Chart(sparkOpts('spark-solar', 'Solar Radiation', solarData, '#FFD700', ' W/m²'));
        }

        // Flights sparkline
        var flightsData = [<?PHP echo implode(',', $spark_flights); ?>];
        if (flightsData.length > 0 && document.getElementById('spark-flights')) {
            new Highcharts.Chart(sparkOpts('spark-flights', 'Flights Out', flightsData, '#8B4513', ''));
        }

        // PM2.5 sparkline
        var pm25Data = [<?PHP echo implode(',', $spark_pm25); ?>];
        if (pm25Data.length > 0 && document.getElementById('spark-pm25')) {
            new Highcharts.Chart(sparkOpts('spark-pm25', 'PM2.5', pm25Data, '#FF6347', ' µg/m³'));
        }

        // EPA sparkline
        var epaData = [<?PHP echo implode(',', $spark_epa_o3); ?>];
        if (epaData.length > 0 && document.getElementById('spark-epa')) {
            new Highcharts.Chart(sparkOpts('spark-epa', 'O3 AQI', epaData, '#9370DB', ''));
        }

        // Pollen sparkline
        var pollenData = [<?PHP echo implode(',', $spark_pollen); ?>];
        if (pollenData.length > 0 && document.getElementById('spark-pollen')) {
            var pollenOpts = sparkOpts('spark-pollen', 'Pollen Level', pollenData, '#4CAF50', ' / 12');
            pollenOpts.series[0].type = 'area';
            pollenOpts.series[0].fillOpacity = 0.3;
            pollenOpts.yAxis.min = 0;
            pollenOpts.yAxis.max = 12;
            new Highcharts.Chart(pollenOpts);
        }

        // GDD sparkline
        var gddData = [<?PHP echo implode(',', $spark_gdd); ?>];
        if (gddData.length > 0 && document.getElementById('spark-gdd')) {
            var gddOpts = sparkOpts('spark-gdd', 'Season GDD', gddData, '#8BC34A', ' GDD');
            gddOpts.series[0].type = 'area';
            gddOpts.series[0].fillOpacity = 0.3;
            new Highcharts.Chart(gddOpts);
        }

        // Weather provider charts (inside collapsible)
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
    });
    </script>

<!-- Footer -->
     <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>
</body>
</html>
