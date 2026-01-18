



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

// Get the Data First
// No metric conversion for ug/m3

$sth = $conn->prepare("SELECT ROUND(AVG(air_pm2_5), 2) as avgair_pm2_5, MAX(air_pm2_5) as maxair_pm2_5, MIN(air_pm2_5) as minair_pm2_5, strftime('%s',date)*1000 AS datetime, (select air_pm2_5 from allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER BY date 
ASC LIMIT 1) as startair, (select air_pm2_5 from allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER BY date DESC LIMIT 1) as endair FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod')");


$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);

// Calculate the gainloss difference for the specifified period

$avgair_pm2_5 = $result['avgair_pm2_5'];
$maxair_pm2_5 = $result['maxair_pm2_5'];
$minair_pm2_5 = $result['minair_pm2_5'];
$startair = $result['startair'];
$endair = $result['endair'];
$diffair = round(($endair - $startair), 2);


?>


 




