



<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart


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

    # Echo back the Javascript code
 
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Get Hive Data First
$sth = $conn->prepare("SELECT ROUND(AVG(solarradiation), 1) as solarradiation, MAX(solarradiation) as maxsolarradiation, MIN(solarradiation) as minsolarradiation, ROUND(AVG(lux), 1) as lux, MAX(lux) as maxlux, MIN(lux) as minlux,  strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime')");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);

// Calculate the gainloss difference for the specifified period

$avgsolarradiation = $result['solarradiation'];
$maxsolarradiation = $result['maxsolarradiation'];
$minsolarradiation = $result['minsolarradiation'];
$avglux = $result['lux'];
$maxlux = $result['maxlux'];
$minlux = $result['minlux'];



?>


 




