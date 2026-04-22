



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
$sth = $conn->prepare("SELECT ROUND(AVG(hiveweight * 0.453592), 1) as hiveweight, ROUND(MAX(hiveweight * 0.453592), 2) as maxhiveweight, ROUND(MIN(hiveweight * 0.453592), 2) as minhiveweight, ROUND(AVG(hiverawweight * 0.453592), 1) as hiverawweight, ROUND(MAX(hiverawweight * 0.453592), 2) as maxhiverawweight, ROUND(MIN(hiverawweight * 0.453592), 2) as minhiverawweight, strftime('%s',date)*1000 AS datetime, (select ROUND((hiveweight * 0.453592), 2) from allhivedata WHERE date > datetime('now', 'localtime', :p1) ORDER BY date ASC LIMIT 1) as startweight, (select ROUND((hiveweight * 0.453592), 2) from allhivedata WHERE date > datetime('now', 'localtime', :p2) ORDER BY date DESC LIMIT 1) as endweight FROM allhivedata WHERE date > datetime('now', 'localtime', :p3)");
} else {
$sth = $conn->prepare("SELECT ROUND(AVG(hiveweight), 1) as hiveweight, MAX(hiveweight) as maxhiveweight, MIN(hiveweight) as minhiveweight, ROUND(AVG(hiverawweight), 1) as hiverawweight, MAX(hiverawweight) as maxhiverawweight, MIN(hiverawweight) as minhiverawweight, strftime('%s',date)*1000 AS datetime, (select hiveweight from allhivedata WHERE date > datetime('now', 'localtime', :p1) ORDER BY date ASC LIMIT 1) as startweight, (select hiveweight from allhivedata WHERE date > datetime('now', 'localtime', :p2) ORDER BY date DESC LIMIT 1) as endweight FROM allhivedata WHERE date > datetime('now', 'localtime', :p3)");
}

$sth->execute([':p1' => $sqlperiod, ':p2' => $sqlperiod, ':p3' => $sqlperiod]);
$result = $sth->fetch(PDO::FETCH_ASSOC);

// Calculate the gainloss difference for the specifified period

$avghiveweight = is_numeric($result['hiveweight']) ? $result['hiveweight'] : '--';
$maxhiveweight = is_numeric($result['maxhiveweight']) ? $result['maxhiveweight'] : '--';
$minhiveweight = is_numeric($result['minhiveweight']) ? $result['minhiveweight'] : '--';
$avghiverawweight = is_numeric($result['hiverawweight']) ? $result['hiverawweight'] : '--';
$maxhiverawweight = is_numeric($result['maxhiverawweight']) ? $result['maxhiverawweight'] : '--';
$minhiverawweight = is_numeric($result['minhiverawweight']) ? $result['minhiverawweight'] : '--';
$startweight = is_numeric($result['startweight']) ? $result['startweight'] : null;
$endweight = is_numeric($result['endweight']) ? $result['endweight'] : null;
$diffweight = ($startweight !== null && $endweight !== null) ? round(($endweight - $startweight), 2) : '--';


?>


 




