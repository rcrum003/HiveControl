<?php

function get_active_alerts($conn) {
    static $cached = null;
    if ($cached !== null) return $cached;

    $alerts = [];

    $cfg = $conn->prepare("SELECT * FROM hiveconfig WHERE id=1");
    $cfg->execute();
    $config = $cfg->fetch(PDO::FETCH_ASSOC);

    if (!$config || $config['alerts_enabled'] !== 'on') {
        return $alerts;
    }

    $stale_minutes = intval($config['alert_stale_minutes'] ?: 30);
    $high_temp = floatval($config['alert_high_temp'] ?: 100.0);
    $low_temp = floatval($config['alert_low_temp'] ?: 40.0);
    $weight_loss_threshold = floatval($config['alert_weight_loss_threshold'] ?: 1.0);
    $weight_loss_hours = intval($config['alert_weight_loss_hours'] ?: 6);
    $swarm_threshold = floatval($config['alert_swarm_threshold'] ?: 3.0);
    $flow_daily_gain = floatval($config['alert_flow_daily_gain'] ?: 0.5);
    $flow_days = intval($config['alert_flow_days'] ?: 3);
    $is_metric = ($config['SHOW_METRIC'] === 'on');

    // Thresholds are stored in imperial (°F, lb). Convert temp thresholds
    // for comparison against metric DB columns. Weight stays in lbs since
    // allhivedata always stores weight in lbs regardless of display mode.
    if ($is_metric) {
        $high_temp = ($high_temp - 32) * 5.0 / 9.0;
        $low_temp = ($low_temp - 32) * 5.0 / 9.0;
    }

    // Get latest reading + age
    $latest = $conn->prepare("SELECT hivetempf, hivetempc, hiveweight, weather_tempf, weather_tempc, (strftime('%s','now','localtime') - strftime('%s',date)) AS age_seconds, date FROM allhivedata ORDER BY datetime(date) DESC LIMIT 1");
    $latest->execute();
    $current = $latest->fetch(PDO::FETCH_ASSOC);

    if (!$current) {
        $alerts[] = [
            'type' => 'stale_data',
            'severity' => 'info',
            'title' => 'No Data',
            'message' => 'No sensor readings found in the database.',
            'icon' => 'fa-exclamation-circle'
        ];
        return $alerts;
    }

    $age_seconds = intval($current['age_seconds']);
    $current_temp = $is_metric ? floatval($current['hivetempc']) : floatval($current['hivetempf']);
    $current_wx_temp = $is_metric ? floatval($current['weather_tempc']) : floatval($current['weather_tempf']);
    $current_weight = floatval($current['hiveweight']);

    // --- Stale Data Check ---
    if ($age_seconds > ($stale_minutes * 60)) {
        $minutes_ago = round($age_seconds / 60);
        $alerts[] = [
            'type' => 'stale_data',
            'severity' => 'info',
            'title' => 'Stale Data',
            'message' => "Last reading was {$minutes_ago} minutes ago.",
            'icon' => 'fa-clock-o'
        ];
    }

    // --- High Hive Temp ---
    if (is_numeric($current_temp) && $current_temp > 0) {
        $temp_unit = $is_metric ? '°C' : '°F';
        $display_high = $is_metric ? round($high_temp, 1) : $config['alert_high_temp'];
        $display_low = $is_metric ? round($low_temp, 1) : $config['alert_low_temp'];

        if ($current_temp > $high_temp) {
            $alerts[] = [
                'type' => 'high_temp',
                'severity' => 'warning',
                'title' => 'High Hive Temperature',
                'message' => "Hive temp is {$current_temp}{$temp_unit} (threshold: {$display_high}{$temp_unit}).",
                'icon' => 'fa-fire'
            ];
        }

        // --- Low Hive Temp ---
        if ($current_temp < $low_temp && $current_temp > -40) {
            $alerts[] = [
                'type' => 'low_temp',
                'severity' => 'warning',
                'title' => 'Low Hive Temperature',
                'message' => "Hive temp is {$current_temp}{$temp_unit} (threshold: {$display_low}{$temp_unit}).",
                'icon' => 'fa-fire'
            ];
        }
    }

    // --- Rapid Weight Loss (Robbing) ---
    if (is_numeric($current_weight) && $current_weight > 0) {
        $hours_ago = "-{$weight_loss_hours} hours";
        $weight_past = $conn->prepare("SELECT hiveweight FROM allhivedata WHERE date > datetime('now', 'localtime', :hours) AND hiveweight > 0 ORDER BY datetime(date) ASC LIMIT 1");
        $weight_past->execute([':hours' => $hours_ago]);
        $past_row = $weight_past->fetch(PDO::FETCH_ASSOC);

        if ($past_row) {
            $past_weight = floatval($past_row['hiveweight']);
            $weight_diff = $past_weight - $current_weight;
            $weight_unit = $is_metric ? 'kg' : 'lb';

            if ($is_metric) {
                $weight_diff_display = round($weight_diff * 0.453592, 2);
            } else {
                $weight_diff_display = round($weight_diff, 2);
            }

            $warm_enough = $is_metric ? ($current_wx_temp > 21) : ($current_wx_temp > 70);

            if ($weight_diff > $swarm_threshold) {
                $alerts[] = [
                    'type' => 'swarm',
                    'severity' => 'danger',
                    'title' => 'Possible Swarm Event',
                    'message' => "Weight dropped {$weight_diff_display} {$weight_unit} in the last {$weight_loss_hours} hours.",
                    'icon' => 'fa-arrow-down'
                ];
            } elseif ($weight_diff > $weight_loss_threshold && $warm_enough) {
                $alerts[] = [
                    'type' => 'robbing',
                    'severity' => 'danger',
                    'title' => 'Possible Robbing Event',
                    'message' => "Weight dropped {$weight_diff_display} {$weight_unit} in {$weight_loss_hours} hours while temp is {$current_wx_temp}" . ($is_metric ? '°C' : '°F') . ".",
                    'icon' => 'fa-warning'
                ];
            }
        }

        // --- Sudden Weight Drop (single reading interval) ---
        $prev_reading = $conn->prepare("SELECT hiveweight FROM allhivedata WHERE hiveweight > 0 ORDER BY datetime(date) DESC LIMIT 1 OFFSET 1");
        $prev_reading->execute();
        $prev_row = $prev_reading->fetch(PDO::FETCH_ASSOC);

        if ($prev_row) {
            $prev_weight = floatval($prev_row['hiveweight']);
            $sudden_diff = $prev_weight - $current_weight;
            if ($sudden_diff > $swarm_threshold) {
                $already_has_swarm = false;
                foreach ($alerts as $a) {
                    if ($a['type'] === 'swarm') { $already_has_swarm = true; break; }
                }
                if (!$already_has_swarm) {
                    $weight_unit = $is_metric ? 'kg' : 'lb';
                    $diff_display = $is_metric ? round($sudden_diff * 0.453592, 2) : round($sudden_diff, 2);
                    $alerts[] = [
                        'type' => 'swarm',
                        'severity' => 'danger',
                        'title' => 'Possible Swarm Event',
                        'message' => "Weight dropped {$diff_display} {$weight_unit} in a single reading interval.",
                        'icon' => 'fa-arrow-down'
                    ];
                }
            }
        }

        // --- Honey Flow (steady gain) ---
        $flow_query = $conn->prepare("SELECT date(date) as day, MAX(hiveweight) - MIN(hiveweight) as daily_change, MAX(hiveweight) as max_weight, MIN(hiveweight) as min_weight FROM allhivedata WHERE date > datetime('now', 'localtime', :days) AND hiveweight > 0 GROUP BY date(date) ORDER BY day DESC");
        $flow_query->execute([':days' => "-{$flow_days} days"]);
        $daily_rows = $flow_query->fetchAll(PDO::FETCH_ASSOC);

        if (count($daily_rows) >= $flow_days) {
            $consecutive_gain = 0;
            $prev_max = null;
            $daily_sorted = array_reverse($daily_rows);
            foreach ($daily_sorted as $day) {
                if ($prev_max !== null) {
                    $gain = floatval($day['max_weight']) - $prev_max;
                    if ($gain >= $flow_daily_gain) {
                        $consecutive_gain++;
                    } else {
                        $consecutive_gain = 0;
                    }
                }
                $prev_max = floatval($day['max_weight']);
            }

            if ($consecutive_gain >= ($flow_days - 1)) {
                $weight_unit = $is_metric ? 'kg' : 'lb';
                $alerts[] = [
                    'type' => 'honey_flow',
                    'severity' => 'success',
                    'title' => 'Honey Flow Detected',
                    'message' => "Weight has been increasing for {$flow_days}+ consecutive days.",
                    'icon' => 'fa-arrow-up'
                ];
            }
        }
    }

    // --- Air Quality: High PM2.5 ---
    $pm25_threshold = floatval(isset($config['alert_pm25_threshold']) ? $config['alert_pm25_threshold'] : 35.5);
    $air_latest = $conn->prepare("SELECT air_pm2_5_raw, air_pm2_5_aqi, air_pm2_5 FROM allhivedata WHERE (air_pm2_5_raw IS NOT NULL OR air_pm2_5 IS NOT NULL) ORDER BY datetime(date) DESC LIMIT 1");
    $air_latest->execute();
    $air_row = $air_latest->fetch(PDO::FETCH_ASSOC);

    if ($air_row) {
        $pm25_val = is_numeric($air_row['air_pm2_5_raw']) ? floatval($air_row['air_pm2_5_raw']) : (is_numeric($air_row['air_pm2_5']) ? floatval($air_row['air_pm2_5']) : null);
        $pm25_aqi = is_numeric($air_row['air_pm2_5_aqi']) ? intval($air_row['air_pm2_5_aqi']) : null;

        if ($pm25_val !== null && $pm25_val > $pm25_threshold) {
            $alerts[] = [
                'type' => 'high_pm25',
                'severity' => 'warning',
                'title' => 'High PM2.5',
                'message' => "PM2.5 is {$pm25_val} ug/m3 (threshold: {$pm25_threshold})." . ($pm25_aqi !== null ? " AQI: {$pm25_aqi}." : ""),
                'icon' => 'fa-cloud'
            ];
        }

        // --- Wildfire Smoke Detection (AQI-based) ---
        $smoke_threshold = intval(isset($config['alert_smoke_aqi_threshold']) ? $config['alert_smoke_aqi_threshold'] : 150);
        if ($pm25_aqi !== null && $pm25_aqi >= $smoke_threshold) {
            $alerts[] = [
                'type' => 'smoke',
                'severity' => 'danger',
                'title' => 'Possible Wildfire Smoke',
                'message' => "PM2.5 AQI is {$pm25_aqi} (smoke threshold: {$smoke_threshold}). Consider restricting hive entrance.",
                'icon' => 'fa-fire'
            ];
        }
    }

    // --- Air Quality: High O3 AQI (EPA) ---
    $o3_threshold = intval(isset($config['alert_o3_threshold']) ? $config['alert_o3_threshold'] : 100);
    try {
        $o3_latest = $conn->prepare("SELECT o3_aqi FROM airquality_epa ORDER BY datetime(date) DESC LIMIT 1");
        $o3_latest->execute();
        $o3_row = $o3_latest->fetch(PDO::FETCH_ASSOC);

        if ($o3_row && is_numeric($o3_row['o3_aqi'])) {
            $o3_aqi = intval($o3_row['o3_aqi']);
            if ($o3_aqi > $o3_threshold) {
                $alerts[] = [
                    'type' => 'high_o3',
                    'severity' => 'warning',
                    'title' => 'High Ozone',
                    'message' => "O3 AQI is {$o3_aqi} (threshold: {$o3_threshold}). Ozone degrades floral scents and impairs bee navigation.",
                    'icon' => 'fa-cloud'
                ];
            }
        }
    } catch (PDOException $e) {
        // airquality_epa table may not exist on pre-2.16 databases
    }

    $cached = $alerts;
    return $alerts;
}

function get_page_alerts($conn, $page_type) {
    $all = get_active_alerts($conn);
    $page_map = [
        'temp'    => ['high_temp', 'low_temp'],
        'weight'  => ['swarm', 'robbing', 'honey_flow'],
        'air'     => ['high_pm25', 'smoke', 'high_o3'],
        'beecount'=> ['swarm', 'robbing'],
    ];
    $types = isset($page_map[$page_type]) ? $page_map[$page_type] : [];
    return array_values(array_filter($all, function($a) use ($types) {
        return in_array($a['type'], $types);
    }));
}

?>
