



<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart


$period_map = [
    'today' => 'start of day', 'day' => '-1 days',
    'week'  => '-7 days',      'month' => '-1 months',
    'year'  => '-1 years',     'all'   => '-20 years',
];
$sqlperiod = isset($period_map[$period]) ? $period_map[$period] : 'start of day';

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sth = $conn->prepare("SELECT ROUND(AVG(air_pm2_5), 2) as avgair_pm2_5, MAX(air_pm2_5) as maxair_pm2_5, MIN(air_pm2_5) as minair_pm2_5, ROUND(AVG(air_pm2_5_raw), 2) as avgair_pm2_5_raw, ROUND(AVG(air_pm10_raw), 2) as avgair_pm10_raw, MAX(air_pm10_raw) as maxair_pm10_raw, MIN(air_pm10_raw) as minair_pm10_raw, ROUND(AVG(air_pm2_5_aqi), 0) as avgair_pm2_5_aqi, MAX(air_pm2_5_aqi) as maxair_pm2_5_aqi, strftime('%s',date)*1000 AS datetime, (select air_pm2_5 from allhivedata WHERE date > datetime('now', 'localtime', :p1) ORDER BY date ASC LIMIT 1) as startair, (select air_pm2_5 from allhivedata WHERE date > datetime('now', 'localtime', :p2) ORDER BY date DESC LIMIT 1) as endair FROM allhivedata WHERE date > datetime('now', 'localtime', :p3)");
$sth->execute([':p1' => $sqlperiod, ':p2' => $sqlperiod, ':p3' => $sqlperiod]);
$result = $sth->fetch(PDO::FETCH_ASSOC);

$avgair_pm2_5 = is_numeric($result['avgair_pm2_5']) ? $result['avgair_pm2_5'] : '--';
$maxair_pm2_5 = is_numeric($result['maxair_pm2_5']) ? $result['maxair_pm2_5'] : '--';
$minair_pm2_5 = is_numeric($result['minair_pm2_5']) ? $result['minair_pm2_5'] : '--';
$startair = is_numeric($result['startair']) ? $result['startair'] : null;
$endair = is_numeric($result['endair']) ? $result['endair'] : null;
$diffair = ($startair !== null && $endair !== null) ? round(($endair - $startair), 2) : '--';

$avgair_pm2_5_raw = is_numeric($result['avgair_pm2_5_raw']) ? $result['avgair_pm2_5_raw'] : '--';
$avgair_pm10_raw = is_numeric($result['avgair_pm10_raw']) ? $result['avgair_pm10_raw'] : '--';
$maxair_pm10_raw = is_numeric($result['maxair_pm10_raw']) ? $result['maxair_pm10_raw'] : '--';
$minair_pm10_raw = is_numeric($result['minair_pm10_raw']) ? $result['minair_pm10_raw'] : '--';
$avgair_pm2_5_aqi = is_numeric($result['avgair_pm2_5_aqi']) ? $result['avgair_pm2_5_aqi'] : '--';
$maxair_pm2_5_aqi = is_numeric($result['maxair_pm2_5_aqi']) ? $result['maxair_pm2_5_aqi'] : '--';

$avg_o3_aqi = null; $max_o3_aqi = null; $avg_no2_aqi = null; $max_no2_aqi = null;
try {
    $epa_sth = $conn->prepare("SELECT ROUND(AVG(o3_aqi), 0) as avg_o3_aqi, MAX(o3_aqi) as max_o3_aqi, ROUND(AVG(no2_aqi), 0) as avg_no2_aqi, MAX(no2_aqi) as max_no2_aqi FROM airquality_epa WHERE date > datetime('now', 'localtime', :p)");
    $epa_sth->execute([':p' => $sqlperiod]);
    $epa_result = $epa_sth->fetch(PDO::FETCH_ASSOC);
    $avg_o3_aqi = $epa_result['avg_o3_aqi'];
    $max_o3_aqi = $epa_result['max_o3_aqi'];
    $avg_no2_aqi = $epa_result['avg_no2_aqi'];
    $max_no2_aqi = $epa_result['max_no2_aqi'];
} catch (PDOException $e) {
    // airquality_epa table may not exist on pre-2.16 databases
}


?>


 




