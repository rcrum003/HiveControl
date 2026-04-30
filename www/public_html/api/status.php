<?php

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');
header('Cache-Control: public, max-age=60');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(204);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'GET') {
    http_response_code(405);
    echo json_encode(['status' => 'error', 'message' => 'Method not allowed']);
    exit;
}

// Rate limiting: 60 requests/min per IP
$rate_file = '/tmp/hivecontrol_api_ratelimit.json';
$client_ip = $_SERVER['REMOTE_ADDR'] ?? 'unknown';
$now = time();
$rate_data = [];

$fp = @fopen($rate_file, 'c+');
if ($fp && flock($fp, LOCK_EX)) {
    $contents = stream_get_contents($fp);
    $rate_data = $contents ? json_decode($contents, true) : [];
    if (!is_array($rate_data)) $rate_data = [];

    // Clean entries older than 60 seconds
    foreach ($rate_data as $ip => $entries) {
        $rate_data[$ip] = array_values(array_filter($entries, function($t) use ($now) {
            return ($now - $t) < 60;
        }));
        if (empty($rate_data[$ip])) unset($rate_data[$ip]);
    }

    $client_hits = $rate_data[$client_ip] ?? [];
    if (count($client_hits) >= 60) {
        flock($fp, LOCK_UN);
        fclose($fp);
        http_response_code(429);
        echo json_encode(['status' => 'error', 'message' => 'Rate limit exceeded. Try again in 60 seconds.']);
        exit;
    }

    $client_hits[] = $now;
    $rate_data[$client_ip] = $client_hits;

    fseek($fp, 0);
    ftruncate($fp, 0);
    fwrite($fp, json_encode($rate_data));
    flock($fp, LOCK_UN);
    fclose($fp);
}

try {
    include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
    include_once($_SERVER["DOCUMENT_ROOT"] . "/include/alert_engine.php");

    // Query 1: Hive configuration
    $cfg_sth = $conn->prepare("SELECT HIVENAME, HIVEID, CITY, STATE, COUNTRY, LATITUDE, LONGITUDE, TIMEZONE, SHOW_METRIC, HCVersion FROM hiveconfig WHERE id=1");
    $cfg_sth->execute();
    $config = $cfg_sth->fetch(PDO::FETCH_ASSOC);

    if (!$config) {
        http_response_code(500);
        echo json_encode(['status' => 'error', 'message' => 'Hive not configured']);
        exit;
    }

    $is_metric = ($config['SHOW_METRIC'] === 'on');

    // Query 2: Latest sensor reading
    $latest_sth = $conn->prepare("SELECT *, (strftime('%s','now','localtime') - strftime('%s',date)) AS age_seconds FROM allhivedata ORDER BY datetime(date) DESC LIMIT 1");
    $latest_sth->execute();
    $latest = $latest_sth->fetch(PDO::FETCH_ASSOC);

    if (!$latest) {
        echo json_encode([
            'status' => 'ok',
            'generated_at' => date('c'),
            'hive' => build_hive_info($config, $is_metric),
            'latest_reading' => null,
            'trends' => null,
            'alarms' => [],
            'alarm_count' => 0
        ], JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
        exit;
    }

    // Query 3: 24-hour weight trend
    $weight_trend_sth = $conn->prepare("SELECT hiveweight FROM allhivedata WHERE date > datetime('now', 'localtime', '-1 days') AND hiveweight > 0 ORDER BY datetime(date) ASC LIMIT 1");
    $weight_trend_sth->execute();
    $weight_past = $weight_trend_sth->fetch(PDO::FETCH_ASSOC);

    // Query 4: 24-hour temperature trend
    $temp_trend_sth = $conn->prepare("SELECT hivetempf, hivetempc FROM allhivedata WHERE date > datetime('now', 'localtime', '-1 days') AND (hivetempf > 0 OR hivetempc > 0) ORDER BY datetime(date) ASC LIMIT 1");
    $temp_trend_sth->execute();
    $temp_past_24h = $temp_trend_sth->fetch(PDO::FETCH_ASSOC);

    // Query 5: 1-hour temperature trend
    $temp_1h_sth = $conn->prepare("SELECT hivetempf, hivetempc FROM allhivedata WHERE date <= datetime('now', 'localtime', '-1 hours') ORDER BY datetime(date) DESC LIMIT 1");
    $temp_1h_sth->execute();
    $temp_past_1h = $temp_1h_sth->fetch(PDO::FETCH_ASSOC);

    // Alarms
    $alarms = get_active_alerts($conn);
    $alarms_clean = array_map(function($a) {
        unset($a['icon']);
        return $a;
    }, $alarms);

    // Build response
    $current_temp = $is_metric ? safe_float($latest['hivetempc']) : safe_float($latest['hivetempf']);
    $current_weight_raw = safe_float($latest['hiveweight']);

    $response = [
        'status' => 'ok',
        'generated_at' => date('c'),
        'hive' => build_hive_info($config, $is_metric),
        'latest_reading' => build_latest_reading($latest, $is_metric),
        'trends' => build_trends($latest, $weight_past, $temp_past_24h, $temp_past_1h, $is_metric),
        'alarms' => array_values($alarms_clean),
        'alarm_count' => count($alarms_clean)
    ];

    echo json_encode($response, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);

} catch (Exception $e) {
    error_log("HiveControl API error: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['status' => 'error', 'message' => 'Internal server error']);
}


function safe_float($val) {
    return (is_numeric($val) && floatval($val) != 0) ? floatval($val) : null;
}

function build_hive_info($config, $is_metric) {
    return [
        'name' => $config['HIVENAME'] ?? '',
        'id' => $config['HIVEID'] ?? '',
        'location' => [
            'city' => $config['CITY'] ?? '',
            'state' => $config['STATE'] ?? '',
            'country' => $config['COUNTRY'] ?? '',
            'latitude' => $config['LATITUDE'] ?? '',
            'longitude' => $config['LONGITUDE'] ?? ''
        ],
        'timezone' => $config['TIMEZONE'] ?? '',
        'version' => $config['HCVersion'] ?? '',
        'units' => $is_metric ? 'metric' : 'imperial'
    ];
}

function build_latest_reading($row, $is_metric) {
    if ($is_metric) {
        $temp = safe_float($row['hivetempc']);
        $temp_unit = 'C';
        $wx_temp = safe_float($row['weather_tempc']);
        $wx_dew = safe_float($row['weather_dewc'] ?? null);
        $weight_raw = safe_float($row['hiveweight']);
        $weight = $weight_raw !== null ? round($weight_raw * 0.453592, 2) : null;
        $weight_unit = 'kg';
        $raw_weight_raw = safe_float($row['hiverawweight'] ?? null);
        $raw_weight = $raw_weight_raw !== null ? round($raw_weight_raw * 0.453592, 2) : null;
        $wind = safe_float($row['wind_kph'] ?? null);
        $wind_gust = safe_float($row['wind_gust_kph'] ?? null);
        $wind_unit = 'kph';
        $pressure = safe_float($row['pressure_mb'] ?? null);
        $pressure_unit = 'mb';
        $precip = safe_float($row['precip_1hr_metric'] ?? null);
    } else {
        $temp = safe_float($row['hivetempf']);
        $temp_unit = 'F';
        $wx_temp = safe_float($row['weather_tempf']);
        $wx_dew = safe_float($row['weather_dewf'] ?? null);
        $weight = safe_float($row['hiveweight']);
        $weight_unit = 'lb';
        $raw_weight = safe_float($row['hiverawweight'] ?? null);
        $wind = safe_float($row['wind_mph'] ?? null);
        $wind_gust = safe_float($row['wind_gust_mph'] ?? null);
        $wind_unit = 'mph';
        $pressure = safe_float($row['pressure_in'] ?? null);
        $pressure_unit = 'in';
        $precip = safe_float($row['precip_1hr_in'] ?? null);
    }

    return [
        'timestamp' => $row['date'] ?? null,
        'age_seconds' => intval($row['age_seconds'] ?? 0),
        'hive' => [
            'temperature' => $temp,
            'temperature_unit' => $temp_unit,
            'humidity' => safe_float($row['hiveHum']),
            'weight' => $weight,
            'weight_unit' => $weight_unit,
            'raw_weight' => $raw_weight,
            'lux' => safe_float($row['lux'] ?? null)
        ],
        'weather' => [
            'temperature' => $wx_temp,
            'temperature_unit' => $temp_unit,
            'humidity' => safe_float($row['weather_humidity']),
            'dew_point' => $wx_dew,
            'wind_speed' => $wind,
            'wind_speed_unit' => $wind_unit,
            'wind_direction' => $row['wind_dir'] ?? null,
            'wind_gust' => $wind_gust,
            'pressure' => $pressure,
            'pressure_unit' => $pressure_unit,
            'pressure_trend' => $row['pressure_trend'] ?? null,
            'precipitation_1hr' => $precip,
            'solar_radiation' => safe_float($row['solarradiation'] ?? null),
            'uv' => safe_float($row['UV'] ?? null)
        ],
        'air_quality' => [
            'pm2_5' => safe_float($row['air_pm2_5_raw'] ?? null),
            'pm2_5_aqi' => safe_float($row['air_pm2_5_aqi'] ?? null),
            'pm10' => safe_float($row['air_pm10_raw'] ?? null),
            'pm10_aqi' => safe_float($row['air_pm10_aqi'] ?? null),
            'pm1' => safe_float($row['air_pm1_raw'] ?? null)
        ]
    ];
}

function build_trends($latest, $weight_past, $temp_past_24h, $temp_past_1h, $is_metric) {
    $trends = [];

    // Weight 24h trend
    $current_weight = safe_float($latest['hiveweight']);
    $past_weight = $weight_past ? safe_float($weight_past['hiveweight']) : null;

    if ($current_weight !== null && $past_weight !== null) {
        if ($is_metric) {
            $start_val = round($past_weight * 0.453592, 2);
            $end_val = round($current_weight * 0.453592, 2);
            $change = round($end_val - $start_val, 2);
            $unit = 'kg';
        } else {
            $start_val = round($past_weight, 2);
            $end_val = round($current_weight, 2);
            $change = round($end_val - $start_val, 2);
            $unit = 'lb';
        }
        $trends['weight_24h'] = [
            'direction' => ($change > 0.1) ? 'rising' : (($change < -0.1) ? 'falling' : 'stable'),
            'change' => $change,
            'unit' => $unit,
            'start_value' => $start_val,
            'end_value' => $end_val
        ];
    } else {
        $trends['weight_24h'] = null;
    }

    // Temperature 24h trend
    $current_temp = $is_metric ? safe_float($latest['hivetempc']) : safe_float($latest['hivetempf']);
    $past_temp_24 = null;
    if ($temp_past_24h) {
        $past_temp_24 = $is_metric ? safe_float($temp_past_24h['hivetempc']) : safe_float($temp_past_24h['hivetempf']);
    }

    if ($current_temp !== null && $past_temp_24 !== null) {
        $temp_unit = $is_metric ? 'C' : 'F';
        $change = round($current_temp - $past_temp_24, 1);
        $trends['temperature_24h'] = [
            'direction' => ($change > 1.0) ? 'rising' : (($change < -1.0) ? 'falling' : 'stable'),
            'change' => $change,
            'unit' => $temp_unit,
            'start_value' => round($past_temp_24, 1),
            'end_value' => round($current_temp, 1)
        ];
    } else {
        $trends['temperature_24h'] = null;
    }

    // Temperature 1h trend
    $past_temp_1 = null;
    if ($temp_past_1h) {
        $past_temp_1 = $is_metric ? safe_float($temp_past_1h['hivetempc']) : safe_float($temp_past_1h['hivetempf']);
    }

    if ($current_temp !== null && $past_temp_1 !== null) {
        $temp_unit = $is_metric ? 'C' : 'F';
        $change = round($current_temp - $past_temp_1, 1);
        $trends['temperature_1h'] = [
            'direction' => ($change > 1.0) ? 'rising' : (($change < -1.0) ? 'falling' : 'stable'),
            'change' => $change,
            'unit' => $temp_unit
        ];
    } else {
        $trends['temperature_1h'] = null;
    }

    return $trends;
}

?>
