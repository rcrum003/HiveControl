



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

if ( $SHOW_METRIC == "on" ) {
$sth = $conn->prepare("SELECT ROUND(AVG(hivetempc), 1) as avghivetempf, MAX(hivetempc) as maxhivetempf, MIN(hivetempc) as minhivetempf, ROUND(AVG(hiveHum), 1) as avghivehum, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', :p) AND hivetempf IS NOT NULL");
$sth2 = $conn->prepare("SELECT ROUND(AVG(weather_tempc),1) as weather_tempf, MAX(weather_tempc) as maxweather_tempf, MIN(weather_tempc) as minweather_tempf, ROUND(AVG(weather_humidity),1) as weather_humidity, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', :p) AND hivetempc IS NOT NULL");
} else {
$sth = $conn->prepare("SELECT ROUND(AVG(hivetempf), 1) as avghivetempf, MAX(hivetempf) as maxhivetempf, MIN(hivetempf) as minhivetempf, ROUND(AVG(hiveHum), 1) as avghivehum, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', :p) AND hivetempf IS NOT NULL");
$sth2 = $conn->prepare("SELECT ROUND(AVG(weather_tempf),1) as weather_tempf, MAX(weather_tempf) as maxweather_tempf, MIN(weather_tempf) as minweather_tempf, ROUND(AVG(weather_humidity),1) as weather_humidity, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', :p) AND weather_tempf IS NOT NULL");
}

$sth->execute([':p' => $sqlperiod]);
$result = $sth->fetch(PDO::FETCH_ASSOC);

$sth2->execute([':p' => $sqlperiod]);
$result2 = $sth2->fetch(PDO::FETCH_ASSOC);


$avghivetempf = is_numeric($result['avghivetempf']) ? $result['avghivetempf'] : '--';
$avghivehum  = is_numeric($result['avghivehum']) ? $result['avghivehum'] : '--';
$avgweather_tempf = is_numeric($result2['weather_tempf']) ? $result2['weather_tempf'] : '--';
$avgweather_humidity = is_numeric($result2['weather_humidity']) ? $result2['weather_humidity'] : '--';
$maxhivetempf = is_numeric($result['maxhivetempf']) ? $result['maxhivetempf'] : '--';
$minhivetempf = is_numeric($result['minhivetempf']) ? $result['minhivetempf'] : '--';
$maxweathertempf = is_numeric($result2['maxweather_tempf']) ? $result2['maxweather_tempf'] : '--';
$minweathertempf = is_numeric($result2['minweather_tempf']) ? $result2['minweather_tempf'] : '--';

?>


 




