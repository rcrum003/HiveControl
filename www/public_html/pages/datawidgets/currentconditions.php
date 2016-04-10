<?php

// Get Hive Data First
$alldatasth = $conn->prepare("SELECT hiveweight, hiverawweight, hivetempf, hiveHum, weather_tempf, weather_humidity, precip_1hr_in, solarradiation, lux, date AS datetime FROM allhivedata ORDER BY datetime(\"date\") DESC LIMIT 1");
$alldatasth->execute();
$alldata = $alldatasth->fetch(PDO::FETCH_ASSOC);


$sth1 = $conn->prepare("SELECT seasongdd, daygdd FROM gdd ORDER BY datetime(\"calcdate\") DESC LIMIT 1;");
$sth1->execute();
$gdddata = $sth1->fetch(PDO::FETCH_ASSOC);

$hivetempf = $alldata['hivetempf'];
$hivehumi = $alldata['hiveHum'];
$hiveweight = $alldata['hiveweight'];
$wxtempf = $alldata['weather_tempf'];
$wxhumi = $alldata['weather_humidity'];
$rawweight = $alldata['hiverawweight'];
$datetime = $alldata['datetime'];
$daygdd = $gdddata['daygdd'];
$seasongdd = $gdddata['seasongdd'];

?>