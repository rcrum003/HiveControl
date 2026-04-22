<?php

$period_map = [
    'today' => 'start of day', 'day' => '-1 days',
    'week'  => '-7 days',      'month' => '-1 months',
    'year'  => '-1 years',     'all'   => '-20 years',
];
$sqlperiod = isset($period_map[$period]) ? $period_map[$period] : 'start of day';

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sth = $conn->prepare("SELECT ROUND(AVG(pollenlevel), 1) as avgpollen, MAX(pollenlevel) as maxpollen, MIN(pollenlevel) as minpollen, COUNT(*) as readings FROM pollen WHERE date >= datetime('now', 'localtime', :p) AND pollenlevel > 0");
$sth->execute([':p' => $sqlperiod]);
$result = $sth->fetch(PDO::FETCH_ASSOC);

$avgpollen = $result['avgpollen'];
$maxpollen = $result['maxpollen'];
$minpollen = $result['minpollen'];
$readings = $result['readings'];

$sth2 = $conn->prepare("SELECT pollenlevel, pollentypes, date FROM pollen WHERE date >= datetime('now', 'localtime', :p) ORDER BY datetime(date) DESC LIMIT 1");
$sth2->execute([':p' => $sqlperiod]);
$latest = $sth2->fetch(PDO::FETCH_ASSOC);

$latest_level = $latest ? $latest['pollenlevel'] : 'N/A';
$latest_types = $latest ? $latest['pollentypes'] : '';
$latest_date = $latest ? $latest['date'] : '';

function pollen_severity($level) {
    if ($level <= 2.4) return 'Low';
    if ($level <= 4.8) return 'Low-Medium';
    if ($level <= 7.2) return 'Medium';
    if ($level <= 9.6) return 'Medium-High';
    return 'High';
}

?>
