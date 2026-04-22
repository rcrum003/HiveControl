



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

$sth = $conn->prepare("SELECT ROUND(AVG(solarradiation), 1) as solarradiation, MAX(solarradiation) as maxsolarradiation, MIN(solarradiation) as minsolarradiation, ROUND(AVG(lux), 1) as lux, MAX(lux) as maxlux, MIN(lux) as minlux,  strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', :p)");
$sth->execute([':p' => $sqlperiod]);
$result = $sth->fetch(PDO::FETCH_ASSOC);

// Calculate the gainloss difference for the specifified period

$avgsolarradiation = $result['solarradiation'];
$maxsolarradiation = $result['maxsolarradiation'];
$minsolarradiation = $result['minsolarradiation'];
$avglux = $result['lux'];
$maxlux = $result['maxlux'];
$minlux = $result['minlux'];



?>


 




