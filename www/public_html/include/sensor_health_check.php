<?php

function get_sensor_health($conn, $period_days = 1) {
    $config_sth = $conn->prepare("SELECT ENABLE_HIVE_TEMP_CHK, ENABLE_HIVE_WEIGHT_CHK, ENABLE_LUX, ENABLE_BEECOUNTER, ENABLE_AIR, WEATHER_LEVEL, WEATHER_FALLBACK, WEATHER_FALLBACK_2, WX_MAX_STALE_MINUTES, SHOW_METRIC FROM hiveconfig WHERE id=1");
    $config_sth->execute();
    $config = $config_sth->fetch(PDO::FETCH_ASSOC);

    $is_metric = ($config['SHOW_METRIC'] === 'on');
    $wx_stale_min = is_numeric($config['WX_MAX_STALE_MINUTES']) ? intval($config['WX_MAX_STALE_MINUTES']) : 120;

    $enable_airnow = 'no';
    try {
        $an_sth = $conn->prepare("SELECT ENABLE_AIRNOW FROM hiveconfig WHERE id=1");
        $an_sth->execute();
        $an_row = $an_sth->fetch(PDO::FETCH_ASSOC);
        $enable_airnow = $an_row['ENABLE_AIRNOW'] ?? 'no';
    } catch (PDOException $e) {}

    $latest_sth = $conn->prepare("SELECT *, (strftime('%s','now','localtime') - strftime('%s',date)) AS age_seconds FROM allhivedata ORDER BY datetime(date) DESC LIMIT 1");
    $latest_sth->execute();
    $latest = $latest_sth->fetch(PDO::FETCH_ASSOC);

    $gdd_sth = $conn->prepare("SELECT *, (strftime('%s','now','localtime') - strftime('%s',gdddate)) AS age_seconds FROM gdd ORDER BY datetime(gdddate) DESC LIMIT 1");
    $gdd_sth->execute();
    $gdd_latest = $gdd_sth->fetch(PDO::FETCH_ASSOC);

    $pollen_sth = $conn->prepare("SELECT *, (strftime('%s','now','localtime') - strftime('%s',date)) AS age_seconds FROM pollen ORDER BY datetime(date) DESC LIMIT 1");
    $pollen_sth->execute();
    $pollen_latest = $pollen_sth->fetch(PDO::FETCH_ASSOC);

    $epa_latest = null;
    try {
        $epa_sth = $conn->prepare("SELECT *, (strftime('%s','now','localtime') - strftime('%s',date)) AS age_seconds FROM airquality_epa ORDER BY datetime(date) DESC LIMIT 1");
        $epa_sth->execute();
        $epa_latest = $epa_sth->fetch(PDO::FETCH_ASSOC);
    } catch (PDOException $e) {}

    $sensors = [];
    $age = $latest ? intval($latest['age_seconds']) : 99999;

    // Hive Temp/Humidity
    $sensors['hivetemp'] = build_sensor(
        'Hive Temp / Humidity',
        $config['ENABLE_HIVE_TEMP_CHK'],
        $latest,
        $age,
        1200,
        'fa-thermometer-half',
        '/pages/temp.php?chart=line&period=week',
        function($row) use ($is_metric) {
            $temp_col = $is_metric ? 'hivetempc' : 'hivetempf';
            $temp = is_numeric($row[$temp_col] ?? null) ? floatval($row[$temp_col]) : null;
            $hum = is_numeric($row['hiveHum'] ?? null) ? floatval($row['hiveHum']) : null;
            $unit = $is_metric ? '°C' : '°F';
            return [
                'values' => ['Temp' => ($temp !== null ? $temp . $unit : '--'), 'Humidity' => ($hum !== null ? $hum . '%' : '--')],
                'has_data' => ($temp !== null && $temp != 0),
                'in_range' => ($temp === null || ($is_metric ? ($temp >= -45 && $temp <= 65) : ($temp >= -50 && $temp <= 150)))
            ];
        }
    );

    // Weight
    $sensors['weight'] = build_sensor(
        'Hive Weight',
        $config['ENABLE_HIVE_WEIGHT_CHK'],
        $latest,
        $age,
        1200,
        'fa-balance-scale',
        '/pages/weight.php?chart=line&period=week',
        function($row) use ($is_metric) {
            $wt = is_numeric($row['hiveweight'] ?? null) ? floatval($row['hiveweight']) : null;
            $raw = is_numeric($row['hiverawweight'] ?? null) ? floatval($row['hiverawweight']) : null;
            if ($is_metric && $wt !== null) $wt = round($wt * 0.453592, 2);
            if ($is_metric && $raw !== null) $raw = round($raw * 0.453592, 2);
            $unit = $is_metric ? ' kg' : ' lb';
            return [
                'values' => ['Net' => ($wt !== null && $wt != 0 ? $wt . $unit : '--'), 'Gross' => ($raw !== null && $raw != 0 ? $raw . $unit : '--')],
                'has_data' => ($wt !== null && $wt != 0) || ($raw !== null && $raw != 0),
                'in_range' => true
            ];
        }
    );

    // Weather
    $wx_enabled = !empty($config['WEATHER_LEVEL']) && $config['WEATHER_LEVEL'] !== 'none';
    $sensors['weather'] = build_sensor(
        'Weather Station',
        $wx_enabled ? 'yes' : 'no',
        $latest,
        $age,
        $wx_stale_min * 60,
        'fa-cloud',
        '/admin/sensor_health.php',
        function($row) use ($is_metric, $config) {
            $temp_col = $is_metric ? 'weather_tempc' : 'weather_tempf';
            $temp = is_numeric($row[$temp_col] ?? null) ? floatval($row[$temp_col]) : null;
            $hum = is_numeric($row['weather_humidity'] ?? null) ? floatval($row['weather_humidity']) : null;
            $unit = $is_metric ? '°C' : '°F';
            return [
                'values' => ['Temp' => ($temp !== null ? $temp . $unit : '--'), 'Humidity' => ($hum !== null ? $hum . '%' : '--'), 'Source' => htmlspecialchars($config['WEATHER_LEVEL'])],
                'has_data' => ($temp !== null && $temp != 0),
                'in_range' => ($temp === null || ($is_metric ? ($temp >= -50 && $temp <= 60) : ($temp >= -70 && $temp <= 150)))
            ];
        }
    );

    // Light/Solar
    $sensors['light'] = build_sensor(
        'Light / Solar',
        $config['ENABLE_LUX'],
        $latest,
        $age,
        1200,
        'fa-sun-o',
        '/pages/light.php?chart=line&period=week',
        function($row) {
            $solar = is_numeric($row['solarradiation'] ?? null) ? floatval($row['solarradiation']) : null;
            $lux = is_numeric($row['lux'] ?? null) ? floatval($row['lux']) : null;
            return [
                'values' => ['Solar' => ($solar !== null ? $solar . ' W/m²' : '--'), 'Lux' => ($lux !== null ? $lux . ' lx' : '--')],
                'has_data' => ($solar !== null && $solar != 0) || ($lux !== null && $lux != 0),
                'in_range' => ($lux === null || ($lux >= 0 && $lux <= 200000))
            ];
        }
    );

    // Bee Counter
    $sensors['beecount'] = build_sensor(
        'Bee Counter',
        $config['ENABLE_BEECOUNTER'],
        $latest,
        $age,
        1200,
        'fa-bug',
        '/pages/beecount.php?chart=line&period=week',
        function($row) {
            $in = is_numeric($row['IN_COUNT'] ?? null) ? intval($row['IN_COUNT']) : null;
            $out = is_numeric($row['OUT_COUNT'] ?? null) ? intval($row['OUT_COUNT']) : null;
            return [
                'values' => ['In' => ($in !== null ? $in : '--'), 'Out' => ($out !== null ? $out : '--')],
                'has_data' => ($in !== null && $in != 0) || ($out !== null && $out != 0),
                'in_range' => true
            ];
        }
    );

    // Air Quality (PurpleAir / Local)
    $sensors['air'] = build_sensor(
        'Air Quality',
        $config['ENABLE_AIR'],
        $latest,
        $age,
        1200,
        'fa-industry',
        '/pages/air.php?chart=line&period=week',
        function($row) {
            $pm25 = is_numeric($row['air_pm2_5'] ?? null) ? floatval($row['air_pm2_5']) : (is_numeric($row['air_pm2_5_raw'] ?? null) ? floatval($row['air_pm2_5_raw']) : null);
            $aqi = is_numeric($row['air_pm2_5_aqi'] ?? null) ? intval($row['air_pm2_5_aqi']) : (is_numeric($row['air_aqi'] ?? null) ? intval($row['air_aqi']) : null);
            return [
                'values' => ['PM2.5' => ($pm25 !== null ? $pm25 . ' µg/m³' : '--'), 'AQI' => ($aqi !== null ? $aqi : '--')],
                'has_data' => ($pm25 !== null),
                'in_range' => ($pm25 === null || ($pm25 >= 0 && $pm25 <= 1000))
            ];
        }
    );

    // Air Quality (EPA / AirNow)
    $sensors['epa'] = build_sensor(
        'EPA AirNow',
        $enable_airnow,
        $epa_latest,
        $epa_latest ? intval($epa_latest['age_seconds']) : 99999,
        5400,
        'fa-leaf',
        '/pages/air.php?chart=line&period=week',
        function($row) {
            $o3 = is_numeric($row['o3_aqi'] ?? null) ? intval($row['o3_aqi']) : null;
            $no2 = is_numeric($row['no2_aqi'] ?? null) ? intval($row['no2_aqi']) : null;
            return [
                'values' => ['O3 AQI' => ($o3 !== null ? $o3 : '--'), 'NO2 AQI' => ($no2 !== null ? $no2 : '--')],
                'has_data' => ($o3 !== null || $no2 !== null),
                'in_range' => true
            ];
        }
    );

    // Pollen
    $pollen_age = $pollen_latest ? intval($pollen_latest['age_seconds']) : 99999;
    $sensors['pollen'] = build_sensor(
        'Pollen',
        'always',
        $pollen_latest,
        $pollen_age,
        129600,
        'fa-pagelines',
        '/pages/pollen.php?chart=line&period=week',
        function($row) {
            $level = is_numeric($row['pollenlevel'] ?? null) ? floatval($row['pollenlevel']) : null;
            $types = $row['pollentypes'] ?? '';
            return [
                'values' => ['Level' => ($level !== null ? $level . '/12' : '--'), 'Types' => ($types ?: '--')],
                'has_data' => ($level !== null),
                'in_range' => ($level === null || ($level >= 0 && $level <= 12))
            ];
        }
    );

    // GDD
    $gdd_age = $gdd_latest ? intval($gdd_latest['age_seconds']) : 99999;
    $sensors['gdd'] = build_sensor(
        'Growing Degree Days',
        'always',
        $gdd_latest,
        $gdd_age,
        129600,
        'fa-tree',
        '/pages/gdd.php?chart=line&period=week',
        function($row) {
            $day = is_numeric($row['daygdd'] ?? null) ? floatval($row['daygdd']) : null;
            $season = is_numeric($row['seasongdd'] ?? null) ? floatval($row['seasongdd']) : null;
            return [
                'values' => ['Today' => ($day !== null ? $day : '--'), 'Season' => ($season !== null ? $season : '--')],
                'has_data' => ($day !== null || $season !== null),
                'in_range' => ($season === null || $season <= 5000)
            ];
        }
    );

    return ['sensors' => $sensors, 'config' => $config];
}

function build_sensor($name, $enabled_flag, $row, $age_seconds, $stale_threshold, $icon, $detail_url, $value_fn) {
    $sensor = [
        'name' => $name,
        'icon' => $icon,
        'detail_url' => $detail_url,
        'enabled' => ($enabled_flag === 'yes' || $enabled_flag === 'always'),
        'always' => ($enabled_flag === 'always'),
        'last_reading' => null,
        'age_seconds' => $age_seconds,
        'values' => [],
        'status' => 'gray',
        'status_label' => 'Disabled'
    ];

    if (!$sensor['enabled'] && !$sensor['always']) {
        return $sensor;
    }

    if (!$row) {
        $sensor['status'] = 'red';
        $sensor['status_label'] = 'No Data';
        return $sensor;
    }

    $sensor['last_reading'] = $row['date'] ?? ($row['gdddate'] ?? null);
    $extracted = $value_fn($row);
    $sensor['values'] = $extracted['values'];

    if (!$extracted['has_data']) {
        $sensor['status'] = 'red';
        $sensor['status_label'] = 'No Data';
    } elseif (!$extracted['in_range']) {
        $sensor['status'] = 'red';
        $sensor['status_label'] = 'Out of Range';
    } elseif ($age_seconds > $stale_threshold) {
        $sensor['status'] = 'yellow';
        $sensor['status_label'] = 'Stale';
    } else {
        $sensor['status'] = 'green';
        $sensor['status_label'] = 'Working';
    }

    return $sensor;
}

function format_age($seconds) {
    if ($seconds < 60) return $seconds . 's ago';
    if ($seconds < 3600) return round($seconds / 60) . 'm ago';
    if ($seconds < 86400) return round($seconds / 3600, 1) . 'h ago';
    return round($seconds / 86400, 1) . 'd ago';
}

function status_panel_class($status) {
    switch ($status) {
        case 'green': return 'panel-green';
        case 'yellow': return 'panel-yellow';
        case 'red': return 'panel-red';
        default: return 'panel-default';
    }
}

function status_label_class($status) {
    switch ($status) {
        case 'green': return 'label-success';
        case 'yellow': return 'label-warning';
        case 'red': return 'label-danger';
        default: return 'label-default';
    }
}

function get_sensor_stats($conn, $period_days, $is_metric) {
    $sqlperiod = "-$period_days days";

    $stats = [];

    // Hive Temp/Humidity stats
    $temp_col = $is_metric ? 'hivetempc' : 'hivetempf';
    $wx_temp_col = $is_metric ? 'weather_tempc' : 'weather_tempf';
    $sth = $conn->prepare("SELECT ROUND(AVG($temp_col), 1) as avg_temp, MAX($temp_col) as max_temp, MIN($temp_col) as min_temp, ROUND(AVG(hiveHum), 1) as avg_hum, MAX(hiveHum) as max_hum, MIN(hiveHum) as min_hum, COUNT(*) as readings FROM allhivedata WHERE date > datetime('now', 'localtime', :p) AND $temp_col IS NOT NULL AND $temp_col != '' AND CAST($temp_col AS REAL) != 0");
    $sth->execute([':p' => $sqlperiod]);
    $stats['hivetemp'] = $sth->fetch(PDO::FETCH_ASSOC);

    // Weight stats
    $wt_col = 'hiveweight';
    $raw_col = 'hiverawweight';
    if ($is_metric) {
        $sth = $conn->prepare("SELECT ROUND(AVG($wt_col * 0.453592), 1) as avg_wt, ROUND(MAX($wt_col * 0.453592), 2) as max_wt, ROUND(MIN($wt_col * 0.453592), 2) as min_wt, (SELECT ROUND($wt_col * 0.453592, 2) FROM allhivedata WHERE date > datetime('now', 'localtime', :p1) AND $wt_col IS NOT NULL AND CAST($wt_col AS REAL) != 0 ORDER BY date ASC LIMIT 1) as start_wt, (SELECT ROUND($wt_col * 0.453592, 2) FROM allhivedata WHERE date > datetime('now', 'localtime', :p2) AND $wt_col IS NOT NULL AND CAST($wt_col AS REAL) != 0 ORDER BY date DESC LIMIT 1) as end_wt, COUNT(*) as readings FROM allhivedata WHERE date > datetime('now', 'localtime', :p3) AND $wt_col IS NOT NULL AND CAST($wt_col AS REAL) != 0");
    } else {
        $sth = $conn->prepare("SELECT ROUND(AVG($wt_col), 1) as avg_wt, MAX($wt_col) as max_wt, MIN($wt_col) as min_wt, (SELECT $wt_col FROM allhivedata WHERE date > datetime('now', 'localtime', :p1) AND $wt_col IS NOT NULL AND CAST($wt_col AS REAL) != 0 ORDER BY date ASC LIMIT 1) as start_wt, (SELECT $wt_col FROM allhivedata WHERE date > datetime('now', 'localtime', :p2) AND $wt_col IS NOT NULL AND CAST($wt_col AS REAL) != 0 ORDER BY date DESC LIMIT 1) as end_wt, COUNT(*) as readings FROM allhivedata WHERE date > datetime('now', 'localtime', :p3) AND $wt_col IS NOT NULL AND CAST($wt_col AS REAL) != 0");
    }
    $sth->execute([':p1' => $sqlperiod, ':p2' => $sqlperiod, ':p3' => $sqlperiod]);
    $wt_stats = $sth->fetch(PDO::FETCH_ASSOC);
    $start = is_numeric($wt_stats['start_wt']) ? floatval($wt_stats['start_wt']) : null;
    $end = is_numeric($wt_stats['end_wt']) ? floatval($wt_stats['end_wt']) : null;
    $wt_stats['diff_wt'] = ($start !== null && $end !== null) ? round($end - $start, 2) : null;
    $stats['weight'] = $wt_stats;

    // Weather stats
    $sth = $conn->prepare("SELECT ROUND(AVG($wx_temp_col), 1) as avg_temp, MAX($wx_temp_col) as max_temp, MIN($wx_temp_col) as min_temp, ROUND(AVG(weather_humidity), 1) as avg_hum, COUNT(*) as readings FROM allhivedata WHERE date > datetime('now', 'localtime', :p) AND $wx_temp_col IS NOT NULL AND $wx_temp_col != '' AND CAST($wx_temp_col AS REAL) != 0");
    $sth->execute([':p' => $sqlperiod]);
    $stats['weather'] = $sth->fetch(PDO::FETCH_ASSOC);

    // Light stats
    $sth = $conn->prepare("SELECT ROUND(AVG(solarradiation), 1) as avg_solar, MAX(solarradiation) as max_solar, MIN(solarradiation) as min_solar, ROUND(AVG(lux), 1) as avg_lux, MAX(lux) as max_lux, MIN(lux) as min_lux, COUNT(*) as readings FROM allhivedata WHERE date > datetime('now', 'localtime', :p) AND (solarradiation IS NOT NULL OR lux IS NOT NULL)");
    $sth->execute([':p' => $sqlperiod]);
    $stats['light'] = $sth->fetch(PDO::FETCH_ASSOC);

    // Bee Counter stats
    $sth = $conn->prepare("SELECT SUM(IN_COUNT) as total_in, SUM(OUT_COUNT) as total_out, MAX(IN_COUNT) as max_in, MAX(OUT_COUNT) as max_out, COUNT(*) as readings FROM allhivedata WHERE date > datetime('now', 'localtime', :p) AND (IN_COUNT IS NOT NULL OR OUT_COUNT IS NOT NULL)");
    $sth->execute([':p' => $sqlperiod]);
    $stats['beecount'] = $sth->fetch(PDO::FETCH_ASSOC);

    // Air Quality stats
    $sth = $conn->prepare("SELECT ROUND(AVG(COALESCE(air_pm2_5_raw, air_pm2_5)), 1) as avg_pm25, MAX(COALESCE(air_pm2_5_raw, air_pm2_5)) as max_pm25, MIN(COALESCE(air_pm2_5_raw, air_pm2_5)) as min_pm25, ROUND(AVG(COALESCE(air_pm2_5_aqi, air_aqi)), 0) as avg_aqi, MAX(COALESCE(air_pm2_5_aqi, air_aqi)) as max_aqi, COUNT(*) as readings FROM allhivedata WHERE date > datetime('now', 'localtime', :p) AND (air_pm2_5 IS NOT NULL OR air_pm2_5_raw IS NOT NULL)");
    $sth->execute([':p' => $sqlperiod]);
    $stats['air'] = $sth->fetch(PDO::FETCH_ASSOC);

    // EPA stats
    try {
        $sth = $conn->prepare("SELECT ROUND(AVG(o3_aqi), 0) as avg_o3, MAX(o3_aqi) as max_o3, ROUND(AVG(no2_aqi), 0) as avg_no2, MAX(no2_aqi) as max_no2, COUNT(*) as readings FROM airquality_epa WHERE date > datetime('now', 'localtime', :p)");
        $sth->execute([':p' => $sqlperiod]);
        $stats['epa'] = $sth->fetch(PDO::FETCH_ASSOC);
    } catch (PDOException $e) {
        $stats['epa'] = ['readings' => 0];
    }

    // Pollen stats
    $sth = $conn->prepare("SELECT ROUND(AVG(pollenlevel), 1) as avg_pollen, MAX(pollenlevel) as max_pollen, MIN(pollenlevel) as min_pollen, COUNT(*) as readings FROM pollen WHERE date >= datetime('now', 'localtime', :p) AND pollenlevel > 0");
    $sth->execute([':p' => $sqlperiod]);
    $stats['pollen'] = $sth->fetch(PDO::FETCH_ASSOC);

    // GDD stats
    $sth = $conn->prepare("SELECT ROUND(AVG(daygdd), 1) as avg_gdd, MAX(seasongdd) as max_season, MIN(seasongdd) as min_season, COUNT(*) as readings FROM gdd WHERE gdddate > datetime('now', 'localtime', :p)");
    $sth->execute([':p' => $sqlperiod]);
    $stats['gdd'] = $sth->fetch(PDO::FETCH_ASSOC);

    return $stats;
}
