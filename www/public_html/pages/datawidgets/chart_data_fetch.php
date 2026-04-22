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
        $sth = $conn->prepare("SELECT round((hiveweight * 0.453592),2) as hiveweight, round((hiverawweight * 0.453592),2) as hiverawweight, hivetempc AS hivetempf, hiveHum, weather_tempc as weather_tempf, weather_humidity, precip_1hr_metric as precip_1hr_in, solarradiation, lux, in_count, out_count, wind_kph as wind, pressure_mb as pressure, COALESCE(air_pm2_5_raw, air_pm2_5) as air_pm2_5_raw, COALESCE(air_pm10_raw, air_pm10) as air_pm10_raw, COALESCE(air_pm1_raw, air_pm1) as air_pm1_raw, COALESCE(air_pm2_5_aqi, air_aqi) as air_pm2_5_aqi, air_pm10_aqi, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
    } else {
        $sth = $conn->prepare("SELECT hiveweight, hiverawweight, hivetempf, hiveHum, weather_tempf, weather_humidity, precip_1hr_in, solarradiation, lux, in_count, out_count, wind_mph as wind, pressure_in as pressure, COALESCE(air_pm2_5_raw, air_pm2_5) as air_pm2_5_raw, COALESCE(air_pm10_raw, air_pm10) as air_pm10_raw, COALESCE(air_pm1_raw, air_pm1) as air_pm1_raw, COALESCE(air_pm2_5_aqi, air_aqi) as air_pm2_5_aqi, air_pm10_aqi, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
    }

    $sth->execute();
    $chart_result = $sth->fetchAll(PDO::FETCH_ASSOC);

    $sth1 = $conn->prepare("SELECT seasongdd AS gdd, strftime('%s',gdddate)*1000 AS datetime FROM gdd WHERE gdddate > datetime('now', 'localtime', '$gdd_sqlperiod') ORDER by datetime ASC");
    $sth1->execute();
    $chart_result_gdd = $sth1->fetchAll(PDO::FETCH_ASSOC);

    $sth3 = $conn->prepare("SELECT pollenlevel, strftime('%s', date)*1000 AS datetime FROM pollen WHERE date >= datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
    $sth3->execute();
    $chart_result_pollen = $sth3->fetchAll(PDO::FETCH_ASSOC);

    $sth_epa = $conn->prepare("SELECT o3_aqi, no2_aqi, pm25_aqi, pm10_aqi, strftime('%s',date)*1000 AS datetime FROM airquality_epa WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
    $sth_epa->execute();
    $chart_result_epa = $sth_epa->fetchAll(PDO::FETCH_ASSOC);

    include_once($_SERVER["DOCUMENT_ROOT"] . "/include/gettheme.php");

    // Pre-build data arrays for dashboard charts (implode pattern)
    $data_hivetemp = []; $data_outtemp = []; $data_hivehum = []; $data_outhum = [];
    $data_netweight = []; $data_grossweight = []; $data_rain = [];
    $data_solar = []; $data_lux = []; $data_flights = []; $data_wind = [];
    $data_pm25 = []; $data_pm10 = []; $data_pm1 = [];
    $data_pm25_aqi = []; $data_pm10_aqi = [];

    // Thin data for large periods: keep every Nth row to cap at ~800 points
    $data_point_count = count($chart_result);
    $thin_step = ($data_point_count > 800) ? intval(ceil($data_point_count / 800)) : 1;
    $idx = 0;

    foreach ($chart_result as $r) {
        $idx++;
        if ($thin_step > 1 && ($idx % $thin_step !== 0) && $idx !== 1 && $idx !== $data_point_count) continue;

        $ts = $r['datetime'];
        $round = ($chart_rounding == "on");

        $v = is_numeric($r['hivetempf']) ? ($round ? ceil($r['hivetempf']) : floatval($r['hivetempf'])) : null;
        $data_hivetemp[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = is_numeric($r['weather_tempf']) ? ($round ? ceil($r['weather_tempf']) : floatval($r['weather_tempf'])) : null;
        $data_outtemp[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = (is_numeric($r['hiveHum']) && $r['hiveHum'] != 0) ? ($round ? ceil($r['hiveHum']) : floatval($r['hiveHum'])) : null;
        $data_hivehum[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = (is_numeric($r['weather_humidity']) && $r['weather_humidity'] != 0) ? ($round ? ceil($r['weather_humidity']) : floatval($r['weather_humidity'])) : null;
        $data_outhum[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = (is_numeric($r['hiveweight']) && $r['hiveweight'] != 0) ? ($round ? ceil($r['hiveweight']) : floatval($r['hiveweight'])) : null;
        $data_netweight[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = (is_numeric($r['hiverawweight']) && $r['hiverawweight'] != 0) ? ($round ? ceil($r['hiverawweight']) : floatval($r['hiverawweight'])) : null;
        $data_grossweight[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = (is_numeric($r['precip_1hr_in']) && $r['precip_1hr_in'] != 0) ? floatval($r['precip_1hr_in']) : null;
        $data_rain[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = (is_numeric($r['solarradiation']) && $r['solarradiation'] != 0) ? floatval($r['solarradiation']) : null;
        $data_solar[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = (is_numeric($r['lux']) && $r['lux'] != 0) ? floatval($r['lux']) : null;
        $data_lux[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = (is_numeric($r['out_count']) && $r['out_count'] != 0) ? floatval($r['out_count']) : null;
        $data_flights[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = (is_numeric($r['wind']) && $r['wind'] != 0) ? floatval($r['wind']) : null;
        $data_wind[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = is_numeric($r['air_pm2_5_raw']) ? floatval($r['air_pm2_5_raw']) : null;
        $data_pm25[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = is_numeric($r['air_pm10_raw']) ? floatval($r['air_pm10_raw']) : null;
        $data_pm10[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = is_numeric($r['air_pm1_raw']) ? floatval($r['air_pm1_raw']) : null;
        $data_pm1[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = is_numeric($r['air_pm2_5_aqi']) ? intval($r['air_pm2_5_aqi']) : null;
        $data_pm25_aqi[] = "[$ts," . ($v !== null ? $v : "null") . "]";

        $v = is_numeric($r['air_pm10_aqi']) ? intval($r['air_pm10_aqi']) : null;
        $data_pm10_aqi[] = "[$ts," . ($v !== null ? $v : "null") . "]";
    }

    $data_epa_o3 = []; $data_epa_no2 = []; $data_epa_pm25 = []; $data_epa_pm10 = [];
    foreach ($chart_result_epa as $r) {
        $ts = $r['datetime'];
        $v = is_numeric($r['o3_aqi']) ? intval($r['o3_aqi']) : null;
        $data_epa_o3[] = "[$ts," . ($v !== null ? $v : "null") . "]";
        $v = is_numeric($r['no2_aqi']) ? intval($r['no2_aqi']) : null;
        $data_epa_no2[] = "[$ts," . ($v !== null ? $v : "null") . "]";
        $v = is_numeric($r['pm25_aqi']) ? intval($r['pm25_aqi']) : null;
        $data_epa_pm25[] = "[$ts," . ($v !== null ? $v : "null") . "]";
        $v = is_numeric($r['pm10_aqi']) ? intval($r['pm10_aqi']) : null;
        $data_epa_pm10[] = "[$ts," . ($v !== null ? $v : "null") . "]";
    }

    $show_markers = ($data_point_count <= 200) ? 'true' : 'false';

    $chart_data_fetched = true;
}

?>
