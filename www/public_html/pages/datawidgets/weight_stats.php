



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

if ( $SHOW_METRIC == "on" ) {

$sth = $conn->prepare("SELECT ROUND(AVG(hiveweight * 0.453592), 1) as hiveweight, ROUND(MAX(hiveweight * 0.453592), 2) as maxhiveweight, ROUND(MIN(hiveweight * 0.453592), 2) as minhiveweight, ROUND(AVG(hiverawweight * 0.453592), 1) as hiverawweight, ROUND(MAX(hiverawweight * 0.453592), 2) as maxhiverawweight, ROUND(MIN(hiverawweight * 0.453592), 2) as minhiverawweight,  strftime('%s',date)*1000 AS datetime, (select ROUND((hiveweight * 0.453592), 2) as hiveweight from allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime') ORDER BY date 
ASC LIMIT 1) as startweight, (select ROUND((hiveweight * 0.453592), 2) as hiveweight from allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime') ORDER BY date DESC LIMIT 1) as endweight FROM allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime')");

} else {
$sth = $conn->prepare("SELECT ROUND(AVG(hiveweight), 1) as hiveweight, MAX(hiveweight) as maxhiveweight, MIN(hiveweight) as minhiveweight, ROUND(AVG(hiverawweight), 1) as hiverawweight, MAX(hiverawweight) as maxhiverawweight, MIN(hiverawweight) as minhiverawweight,  strftime('%s',date)*1000 AS datetime, (select hiveweight from allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime') ORDER BY date 
ASC LIMIT 1) as startweight, (select hiveweight from allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime') ORDER BY date DESC LIMIT 1) as endweight FROM allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime')");
}




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


 




