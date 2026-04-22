<?php

include_once($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sth = $conn->prepare("SELECT (strftime('%s','now','localtime') - strftime('%s',date)) AS age, date, hivetempf, hiveweight, weather_tempf FROM allhivedata ORDER BY datetime(date) DESC LIMIT 1");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);

$AGE = $result ? intval($result['age']) : 99999;
$HIVETEMPF = $result ? $result['hivetempf'] : null;
$HIVEWEIGHT = $result ? $result['hiveweight'] : null;
$WEATHERTEMPF = $result ? $result['weather_tempf'] : null;
$LAST_DATE = $result ? $result['date'] : 'N/A';

$gddsth = $conn->prepare("SELECT (strftime('%s','now','localtime') - strftime('%s',calcdate)) AS age2, daygdd, seasongdd FROM gdd ORDER BY datetime(calcdate) DESC LIMIT 1");
$gddsth->execute();
$gddresult = $gddsth->fetch(PDO::FETCH_ASSOC);

$GDDAGE = $gddresult ? intval($gddresult['age2']) : 99999;
$DAYGDD = $gddresult ? $gddresult['daygdd'] : null;
$SEASONGDD = $gddresult ? $gddresult['seasongdd'] : null;

$temp_ok = ($AGE <= 600 && is_numeric($HIVETEMPF) && $HIVETEMPF <= 150 && $HIVETEMPF >= -50);
$weight_ok = ($AGE <= 600 && is_numeric($HIVEWEIGHT) && $HIVEWEIGHT > 0 && $HIVEWEIGHT <= 500);
$weather_ok = ($AGE <= 600 && is_numeric($WEATHERTEMPF) && $WEATHERTEMPF <= 150 && $WEATHERTEMPF >= -70);
$gdd_ok = ($GDDAGE <= 86400 && is_numeric($DAYGDD) && $DAYGDD >= 0 && $DAYGDD <= 36 && $SEASONGDD <= 5000);

if ($AGE <= 600) {
    $freshness = '<span class="label label-success">Live</span>';
} elseif ($AGE <= 1800) {
    $freshness = '<span class="label label-warning">' . round($AGE / 60) . 'm ago</span>';
} else {
    $freshness = '<span class="label label-danger">' . round($AGE / 3600, 1) . 'h ago</span>';
}
?>
<div style="margin-bottom: 8px;">Data: <?php echo $freshness; ?></div>
<span class="label label-<?php echo $temp_ok ? 'success' : 'danger'; ?>">Temp</span>
<span class="label label-<?php echo $weight_ok ? 'success' : 'danger'; ?>">Weight</span>
<span class="label label-<?php echo $weather_ok ? 'success' : 'danger'; ?>">Weather</span>
<span class="label label-<?php echo $gdd_ok ? 'success' : 'danger'; ?>">GDD</span>
