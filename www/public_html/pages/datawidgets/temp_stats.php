



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


if ( $SHOW_METRIC == "on" ) {
$sth = $conn->prepare("SELECT ROUND(AVG(hivetempc), 1) as hivetempf, MAX(hivetempc) as maxhivetempf, MIN(hivetempc) as minhivetempf, ROUND(AVG(hiveHum), 1) as hivehum, ROUND(AVG(weather_tempc),1) as weather_tempf, MAX(weather_tempc) as maxweather_tempf, MIN(weather_tempc) as minweather_tempf, ROUND(AVG(weather_humidity),1) as weather_humidity, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime')");
} else {
$sth = $conn->prepare("SELECT ROUND(AVG(hivetempf), 1) as hivetempf, MAX(hivetempf) as maxhivetempf, MIN(hivetempf) as minhivetempf, ROUND(AVG(hiveHum), 1) as hivehum, ROUND(AVG(weather_tempf),1) as weather_tempf, MAX(weather_tempf) as maxweather_tempf, MIN(weather_tempf) as minweather_tempf, ROUND(AVG(weather_humidity),1) as weather_humidity, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime')");
}

$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);



$avghivetempf = $result['hivetempf'];
$avghivehum  = $result['hivehum'];
$avgweather_tempf = $result['weather_tempf'];
$avgweather_humidity = $result['weather_humidity'];
$maxhivetempf = $result['maxhivetempf'];
$minhivetempf = $result['minhivetempf'];
$maxweathertempf = $result['maxweather_tempf'];
$minweathertempf = $result['minweather_tempf'];

?>


 




