



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

$sth = $conn->prepare("SELECT sum(IN_COUNT) as total_in, sum(OUT_COUNT) as total_out, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', :p)");
$sth->execute([':p' => $sqlperiod]);
$result = $sth->fetch(PDO::FETCH_ASSOC);


$total_in = $result['total_in'];
$total_out = $result['total_out'];

?>


 




