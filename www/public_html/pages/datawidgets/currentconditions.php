<?php

// Get Hive Data First
$alldatasth = $conn->prepare("SELECT hiveweight, hiverawweight, hivetempf, hiveHum, weather_tempf, weather_humidity, precip_1hr_in, solarradiation, lux, date AS datetime FROM allhivedata ORDER BY datetime(\"date\") DESC LIMIT 1");
$alldatasth->execute();
$alldata = $alldatasth->fetch(PDO::FETCH_ASSOC);

$sth1 = $conn->prepare("SELECT seasongdd, daygdd FROM gdd ORDER BY datetime(\"calcdate\") DESC LIMIT 1;");
$sth1->execute();
$gdddata = $sth1->fetch(PDO::FETCH_ASSOC);

$hivetempf = ceil($alldata['hivetempf']);
$hivehumi = ceil($alldata['hiveHum']);
$hiveweight = ceil($alldata['hiveweight']);
$wxtempf = ceil($alldata['weather_tempf']);
$wxhumi = ceil($alldata['weather_humidity']);
$rawweight = ceil($alldata['hiverawweight']);
$datetime = ceil($alldata['datetime']);
$daygdd = ceil($gdddata['daygdd']);
$seasongdd = ceil($gdddata['seasongdd']);

#Get some trends
$trendsth = $conn->prepare("SELECT hiveweight, hiverawweight, hivetempf, hiveHum, weather_tempf, weather_humidity, precip_1hr_in, solarradiation, lux, date AS datetime FROM allhivedata ORDER BY datetime(\"date\") DESC LIMIT 288;");
$trendsth->execute();
$trenddata = $trendsth->fetch(PDO::FETCH_ASSOC);

$pasthivetempf = ceil($trenddata['hivetempf']);
$pasthivehumi = ceil($trenddata['hiveHum']);
$pasthiveweight = ceil($trenddata['hiveweight']);
$pastwxtempf = ceil($trenddata['weather_tempf']);
$pastwxhumi = ceil($trenddata['weather_humidity']);
$pastrawweight = ceil($trenddata['hiverawweight']);

$changeweight = ($pasthiveweight - $hiveweight);  


?>