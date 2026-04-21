<?php

switch ($period) {
    case "today":
        $sqlperiod = "start of day";
        break;
    case "day":
        $sqlperiod = "-1 days";
        break;
    case "week":
        $sqlperiod = "-7 days";
        break;
    case "month":
        $sqlperiod = "-1 months";
        break;
    case "year":
        $sqlperiod =  "-1 years";
        break;
    case "all":
        $sqlperiod =  "-20 years";
        break;
    }

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sth = $conn->prepare("SELECT ROUND(AVG(pollenlevel), 1) as avgpollen, MAX(pollenlevel) as maxpollen, MIN(pollenlevel) as minpollen, COUNT(*) as readings FROM pollen WHERE date >= datetime('now', 'localtime', '$sqlperiod') AND pollenlevel > 0");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);

$avgpollen = $result['avgpollen'];
$maxpollen = $result['maxpollen'];
$minpollen = $result['minpollen'];
$readings = $result['readings'];

$sth2 = $conn->prepare("SELECT pollenlevel, pollentypes, date FROM pollen WHERE date >= datetime('now', 'localtime', '$sqlperiod') ORDER BY datetime(date) DESC LIMIT 1");
$sth2->execute();
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
