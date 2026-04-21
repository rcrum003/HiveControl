<?php

switch ($period) {
    case "today":
        $sqlperiod = "start of day";
        $gdd_sqlperiod = "-7 days";
        break;
    case "day":
        $sqlperiod = "-24 hours";
        $gdd_sqlperiod = "-7 days";
        break;
    case "week":
        $sqlperiod = "-7 days";
        $gdd_sqlperiod = "-7 days";
        break;
    case "month":
        $sqlperiod = "-1 months";
        $gdd_sqlperiod = "-1 months";
        break;
    case "year":
        $sqlperiod = "-1 years";
        $gdd_sqlperiod = "-1 years";
        break;
    case "all":
        $sqlperiod = "-20 years";
        $gdd_sqlperiod = "-20 years";
        break;
    default:
        $sqlperiod = "-24 hours";
        $gdd_sqlperiod = "-7 days";
        break;
}

if (!isset($chart_data_fetched)) {
    include_once($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

    if ($SHOW_METRIC == "on") {
        $sth = $conn->prepare("SELECT round((hiveweight * 0.453592),2) as hiveweight, round((hiverawweight * 0.453592),2) as hiverawweight, hivetempc AS hivetempf, hiveHum, weather_tempc as weather_tempf, weather_humidity, precip_1hr_metric as precip_1hr_in, solarradiation, lux, in_count, out_count, wind_kph as wind, pressure_mb as pressure, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
    } else {
        $sth = $conn->prepare("SELECT hiveweight, hiverawweight, hivetempf, hiveHum, weather_tempf, weather_humidity, precip_1hr_in, solarradiation, lux, in_count, out_count, wind_mph as wind, pressure_in as pressure, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
    }

    $sth->execute();
    $chart_result = $sth->fetchAll(PDO::FETCH_ASSOC);

    $sth1 = $conn->prepare("SELECT seasongdd AS gdd, strftime('%s',gdddate)*1000 AS datetime FROM gdd WHERE gdddate > datetime('now', 'localtime', '$gdd_sqlperiod') ORDER by datetime ASC");
    $sth1->execute();
    $chart_result_gdd = $sth1->fetchAll(PDO::FETCH_ASSOC);

    $sth3 = $conn->prepare("SELECT pollenlevel, strftime('%s', date)*1000 AS datetime FROM pollen WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
    $sth3->execute();
    $chart_result_pollen = $sth3->fetchAll(PDO::FETCH_ASSOC);

    include_once($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

    $chart_data_fetched = true;
}

?>
