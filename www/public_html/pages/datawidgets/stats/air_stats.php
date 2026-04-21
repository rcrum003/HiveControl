



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

$sth = $conn->prepare("SELECT ROUND(AVG(air_pm2_5), 2) as avgair_pm2_5, MAX(air_pm2_5) as maxair_pm2_5, MIN(air_pm2_5) as minair_pm2_5, ROUND(AVG(air_pm2_5_raw), 2) as avgair_pm2_5_raw, ROUND(AVG(air_pm10_raw), 2) as avgair_pm10_raw, MAX(air_pm10_raw) as maxair_pm10_raw, MIN(air_pm10_raw) as minair_pm10_raw, ROUND(AVG(air_pm2_5_aqi), 0) as avgair_pm2_5_aqi, MAX(air_pm2_5_aqi) as maxair_pm2_5_aqi, strftime('%s',date)*1000 AS datetime, (select air_pm2_5 from allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER BY date ASC LIMIT 1) as startair, (select air_pm2_5 from allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER BY date DESC LIMIT 1) as endair FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod')");

$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);

$avgair_pm2_5 = $result['avgair_pm2_5'];
$maxair_pm2_5 = $result['maxair_pm2_5'];
$minair_pm2_5 = $result['minair_pm2_5'];
$startair = $result['startair'];
$endair = $result['endair'];
$diffair = round(($endair - $startair), 2);

$avgair_pm2_5_raw = $result['avgair_pm2_5_raw'];
$avgair_pm10_raw = $result['avgair_pm10_raw'];
$maxair_pm10_raw = $result['maxair_pm10_raw'];
$minair_pm10_raw = $result['minair_pm10_raw'];
$avgair_pm2_5_aqi = $result['avgair_pm2_5_aqi'];
$maxair_pm2_5_aqi = $result['maxair_pm2_5_aqi'];

// EPA O3/NO2 stats
$epa_sth = $conn->prepare("SELECT ROUND(AVG(o3_aqi), 0) as avg_o3_aqi, MAX(o3_aqi) as max_o3_aqi, ROUND(AVG(no2_aqi), 0) as avg_no2_aqi, MAX(no2_aqi) as max_no2_aqi FROM airquality_epa WHERE date > datetime('now', 'localtime', '$sqlperiod')");
$epa_sth->execute();
$epa_result = $epa_sth->fetch(PDO::FETCH_ASSOC);

$avg_o3_aqi = $epa_result['avg_o3_aqi'];
$max_o3_aqi = $epa_result['max_o3_aqi'];
$avg_no2_aqi = $epa_result['avg_no2_aqi'];
$max_no2_aqi = $epa_result['max_no2_aqi'];


?>


 




