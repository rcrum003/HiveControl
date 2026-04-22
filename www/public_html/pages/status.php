<?php

include_once($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "/include/sensor_health_check.php");

$health = get_sensor_health($conn);
$sensors = $health['sensors'];

$latest_sth = $conn->prepare("SELECT (strftime('%s','now','localtime') - strftime('%s',date)) AS age FROM allhivedata ORDER BY datetime(date) DESC LIMIT 1");
$latest_sth->execute();
$latest = $latest_sth->fetch(PDO::FETCH_ASSOC);
$AGE = $latest ? intval($latest['age']) : 99999;

if ($AGE <= 600) {
    $freshness = '<span class="label label-success">Live</span>';
} elseif ($AGE <= 1800) {
    $freshness = '<span class="label label-warning">' . round($AGE / 60) . 'm ago</span>';
} else {
    $freshness = '<span class="label label-danger">' . round($AGE / 3600, 1) . 'h ago</span>';
}

$badge_sensors = ['hivetemp' => 'Temp', 'weight' => 'Weight', 'weather' => 'Weather', 'light' => 'Light', 'beecount' => 'Bees', 'air' => 'Air', 'pollen' => 'Pollen', 'gdd' => 'GDD'];
?>
<div style="margin-bottom: 8px;">Data: <?php echo $freshness; ?></div>
<?php
foreach ($badge_sensors as $key => $label) {
    $s = $sensors[$key];
    if (!$s['enabled'] && !$s['always']) continue;
    $cls = status_label_class($s['status']);
    echo '<span class="label ' . $cls . '">' . $label . '</span> ';
}
?>
