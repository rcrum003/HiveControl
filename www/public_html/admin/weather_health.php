<?PHP
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
include($_SERVER["DOCUMENT_ROOT"] . "/include/security-init.php");

function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}

$period = isset($_GET['period']) ? test_input($_GET['period']) : '7';
if (!in_array($period, ['1', '7', '30'])) {
    $period = '7';
}

// Get current weather config
$sth = $conn->prepare("SELECT WEATHER_LEVEL, WEATHER_FALLBACK, WEATHER_FALLBACK_2, WX_MAX_STALE_MINUTES FROM hiveconfig");
$sth->execute();
$config = $sth->fetch(PDO::FETCH_ASSOC);

// Provider success rates for the period
$sth2 = $conn->prepare("SELECT provider,
    COUNT(*) as total_calls,
    SUM(success) as successes,
    ROUND(AVG(success) * 100, 1) as success_rate,
    ROUND(AVG(response_ms), 0) as avg_response_ms,
    ROUND(AVG(CASE WHEN success = 1 THEN observation_age_minutes END), 1) as avg_obs_age,
    MAX(timestamp) as last_seen
    FROM weather_health
    WHERE timestamp >= datetime('now', '-' || ? || ' days', 'localtime')
    GROUP BY provider
    ORDER BY total_calls DESC");
$sth2->execute([$period]);
$provider_stats = $sth2->fetchAll(PDO::FETCH_ASSOC);

// Recent failures (last 50)
$sth3 = $conn->prepare("SELECT timestamp, provider, role, error_reason, observation_age_minutes, response_ms
    FROM weather_health
    WHERE success = 0 AND timestamp >= datetime('now', '-' || ? || ' days', 'localtime')
    ORDER BY timestamp DESC LIMIT 50");
$sth3->execute([$period]);
$recent_failures = $sth3->fetchAll(PDO::FETCH_ASSOC);

// Hourly success rate data for chart (last N days)
$sth4 = $conn->prepare("SELECT
    strftime('%Y-%m-%d %H:00', timestamp) as hour,
    provider,
    COUNT(*) as calls,
    SUM(success) as successes,
    ROUND(AVG(response_ms), 0) as avg_ms
    FROM weather_health
    WHERE timestamp >= datetime('now', '-' || ? || ' days', 'localtime')
    GROUP BY hour, provider
    ORDER BY hour ASC");
$sth4->execute([$period]);
$hourly_data = $sth4->fetchAll(PDO::FETCH_ASSOC);

// Build chart series data per provider
$chart_providers = [];
$chart_response = [];
foreach ($hourly_data as $row) {
    $ts = strtotime($row['hour']) * 1000; // JS timestamp
    $rate = $row['calls'] > 0 ? round(($row['successes'] / $row['calls']) * 100, 1) : 0;
    $chart_providers[$row['provider']][] = [$ts, $rate];
    $chart_response[$row['provider']][] = [$ts, (int)$row['avg_ms']];
}

// Role distribution
$sth5 = $conn->prepare("SELECT role,
    COUNT(*) as total,
    SUM(success) as successes
    FROM weather_health
    WHERE success = 1 AND timestamp >= datetime('now', '-' || ? || ' days', 'localtime')
    GROUP BY role");
$sth5->execute([$period]);
$role_stats = $sth5->fetchAll(PDO::FETCH_ASSOC);

$primary_pct = 0;
$fallback_pct = 0;
$total_success = 0;
foreach ($role_stats as $r) {
    $total_success += $r['successes'];
}
foreach ($role_stats as $r) {
    if ($r['role'] === 'primary') {
        $primary_pct = $total_success > 0 ? round(($r['successes'] / $total_success) * 100, 1) : 0;
    } else {
        $fallback_pct += $total_success > 0 ? round(($r['successes'] / $total_success) * 100, 1) : 0;
    }
}

?>
<!DOCTYPE html>
<html lang="en">

    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->
    <div id="wrapper">
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Weather Health Dashboard</h1>
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

            <!-- Config Summary -->
            <div class="row">
                <div class="col-lg-3 col-md-6">
                    <div class="panel panel-primary">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3"><i class="fa fa-cloud fa-5x"></i></div>
                                <div class="col-xs-9 text-right">
                                    <div class="huge"><?PHP echo htmlspecialchars($config['WEATHER_LEVEL'] ?: 'None'); ?></div>
                                    <div>Primary Source</div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-3 col-md-6">
                    <div class="panel panel-green">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3"><i class="fa fa-refresh fa-5x"></i></div>
                                <div class="col-xs-9 text-right">
                                    <div class="huge"><?PHP echo $primary_pct; ?>%</div>
                                    <div>Primary Success</div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-3 col-md-6">
                    <div class="panel panel-yellow">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3"><i class="fa fa-exchange fa-5x"></i></div>
                                <div class="col-xs-9 text-right">
                                    <div class="huge"><?PHP echo $fallback_pct; ?>%</div>
                                    <div>Fallback Used</div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-3 col-md-6">
                    <div class="panel panel-red">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3"><i class="fa fa-clock-o fa-5x"></i></div>
                                <div class="col-xs-9 text-right">
                                    <div class="huge"><?PHP echo htmlspecialchars($config['WX_MAX_STALE_MINUTES'] ?: '120'); ?> min</div>
                                    <div>Stale Threshold</div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>

            <!-- Provider Stats Table -->
            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-heading"><i class="fa fa-bar-chart fa-fw"></i> Provider Statistics (<?PHP echo $period; ?> day<?PHP echo $period !== '1' ? 's' : ''; ?>)</div>
                        <div class="panel-body">
                            <div class="table-responsive">
                                <table class="table table-striped table-bordered table-hover">
                                    <thead>
                                        <tr>
                                            <th>Provider</th>
                                            <th>Total Calls</th>
                                            <th>Successes</th>
                                            <th>Success Rate</th>
                                            <th>Avg Response</th>
                                            <th>Avg Obs Age</th>
                                            <th>Last Seen</th>
                                        </tr>
                                    </thead>
                                    <tbody>
<?PHP
if (empty($provider_stats)) {
    echo '<tr><td colspan="7" class="text-center text-muted">No weather health data recorded yet. Data appears after the first weather fetch with the updated getwx.sh.</td></tr>';
} else {
    foreach ($provider_stats as $stat) {
        $rate = floatval($stat['success_rate']);
        if ($rate >= 95) {
            $badge = 'success';
        } elseif ($rate >= 80) {
            $badge = 'warning';
        } else {
            $badge = 'danger';
        }
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
                        </div>
                    </div>
                </div>
            </div>

            <!-- Charts Row -->
            <div class="row">
                <div class="col-lg-6">
                    <div class="panel panel-default">
                        <div class="panel-heading"><i class="fa fa-line-chart fa-fw"></i> Success Rate Over Time</div>
                        <div class="panel-body">
                            <div id="success-chart" style="height: 300px;"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-6">
                    <div class="panel panel-default">
                        <div class="panel-heading"><i class="fa fa-tachometer fa-fw"></i> Response Time Over Time</div>
                        <div class="panel-body">
                            <div id="response-chart" style="height: 300px;"></div>
                        </div>
                    </div>
                </div>
            </div>

            <!-- Recent Failures -->
            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-heading"><i class="fa fa-exclamation-triangle fa-fw"></i> Recent Failures</div>
                        <div class="panel-body">
                            <div class="table-responsive">
                                <table class="table table-striped table-bordered table-hover" id="failures-table">
                                    <thead>
                                        <tr>
                                            <th>Time</th>
                                            <th>Provider</th>
                                            <th>Role</th>
                                            <th>Error</th>
                                            <th>Obs Age</th>
                                            <th>Response</th>
                                        </tr>
                                    </thead>
                                    <tbody>
<?PHP
if (empty($recent_failures)) {
    echo '<tr><td colspan="6" class="text-center text-muted">No failures recorded in this period.</td></tr>';
} else {
    foreach ($recent_failures as $fail) {
        echo '<tr>';
        echo '<td>' . htmlspecialchars($fail['timestamp']) . '</td>';
        echo '<td>' . htmlspecialchars($fail['provider']) . '</td>';
        echo '<td><span class="label label-default">' . htmlspecialchars($fail['role']) . '</span></td>';
        echo '<td>' . htmlspecialchars($fail['error_reason'] ?: '-') . '</td>';
        echo '<td>' . htmlspecialchars($fail['observation_age_minutes']) . ' min</td>';
        echo '<td>' . htmlspecialchars($fail['response_ms']) . ' ms</td>';
        echo '</tr>';
    }
}
?>
                                    </tbody>
                                </table>
                            </div>
                        </div>
                    </div>
                </div>
            </div>

    </div>
    <!-- /#page-wrapper -->

    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>
    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>
    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>
    <!-- DataTables JavaScript -->
    <script src="../bower_components/datatables/media/js/jquery.dataTables.min.js"></script>
    <script src="../bower_components/datatables-plugins/integration/bootstrap/3/dataTables.bootstrap.min.js"></script>
    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>
    <!-- Highcharts -->
    <script src="/js/highcharts/highcharts.js"></script>
    <script src="/js/highcharts/modules/exporting.js"></script>

    <script>
    $(document).ready(function() {
        if ($('#failures-table tbody tr td').length > 1) {
            $('#failures-table').DataTable({
                responsive: true,
                order: [[0, 'desc']],
                pageLength: 25
            });
        }

        // Success Rate Chart
        Highcharts.chart('success-chart', {
            chart: { type: 'line', zoomType: 'x' },
            title: { text: null },
            xAxis: { type: 'datetime' },
            yAxis: { title: { text: 'Success Rate (%)' }, min: 0, max: 100 },
            tooltip: { shared: true, valueSuffix: '%' },
            legend: { enabled: true },
            series: [
<?PHP
$colors = ['#2196F3', '#4CAF50', '#FF9800', '#9C27B0', '#F44336', '#00BCD4'];
$ci = 0;
foreach ($chart_providers as $prov => $data) {
    $json = json_encode($data);
    echo "{ name: " . json_encode($prov) . ", data: $json, color: '{$colors[$ci % count($colors)]}' },\n";
    $ci++;
}
?>
            ]
        });

        // Response Time Chart
        Highcharts.chart('response-chart', {
            chart: { type: 'line', zoomType: 'x' },
            title: { text: null },
            xAxis: { type: 'datetime' },
            yAxis: { title: { text: 'Response Time (ms)' }, min: 0 },
            tooltip: { shared: true, valueSuffix: ' ms' },
            legend: { enabled: true },
            series: [
<?PHP
$ci = 0;
foreach ($chart_response as $prov => $data) {
    $json = json_encode($data);
    echo "{ name: " . json_encode($prov) . ", data: $json, color: '{$colors[$ci % count($colors)]}' },\n";
    $ci++;
}
?>
            ]
        });
    });
    </script>

</body>
</html>
