



<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart


switch ($period) {
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
$sth = $conn->prepare("SELECT ROUND(AVG(hiveweight), 1) as hiveweight, MAX(hiveweight) as maxhiveweight, MIN(hiveweight) as minhiveweight, ROUND(AVG(hiverawweight), 1) as hiverawweight, MAX(hiverawweight) as maxhiverawweight, MIN(hiverawweight) as minhiverawweight,  strftime('%s',date)*1000 AS datetime, (select hiveweight from allhivedata WHERE date > datetime('now','$sqlperiod') ORDER BY date 
ASC LIMIT 1) as startweight, (select hiveweight from allhivedata WHERE date > datetime('now','$sqlperiod') ORDER BY date DESC LIMIT 1) as endweight FROM allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime')");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);

// Calculate the gainloss difference for the specifified period

$avghiveweight = $result['hiveweight'];
$maxhiveweight = $result['maxhiveweight'];
$minhiveweight = $result['minhiveweight'];
$avghiverawweight = $result['hiverawweight'];
$maxhiverawweight = $result['maxhiverawweight'];
$minhiverawweight = $result['minhiverawweight'];
$startweight = $result['startweight'];
$endweight = $result['endweight'];
$diffweight = round(($endweight - $startweight), 2);


?>


 




