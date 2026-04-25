<?PHP
// Setup Wizard - Multi-Step Sensor Configuration Guide
// Version 2026041901
// Walks users through configuring each sensor with live testing

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
// SECURITY FIX: Include security-init for CSRF protection and security headers
include($_SERVER["DOCUMENT_ROOT"] . "/include/security-init.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

// Override CSP to allow Leaflet CDN and OpenStreetMap tiles for the location map
header("Content-Security-Policy: default-src 'self'; script-src 'self' 'unsafe-inline' https://unpkg.com; style-src 'self' 'unsafe-inline' https://unpkg.com; img-src 'self' data: https://*.tile.openstreetmap.org; connect-src 'self';");

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}

function test_input_allow_slash($data) {
    $data = trim($data);
    $data = htmlspecialchars($data);
    return $data;
}

$step = isset($_GET['step']) ? max(1, min(8, (int)$_GET['step'])) : 1;
$errors = [];
$saved = false;

// Load current config
$sth = $conn->prepare("SELECT * FROM hiveconfig");
$sth->execute();
$config = $sth->fetch(PDO::FETCH_ASSOC);
if (!$config) { $config = []; }

// Process POST submissions
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $action = isset($_POST['wizard_action']) ? $_POST['wizard_action'] : 'next';
    $post_step = isset($_POST['wizard_step']) ? (int)$_POST['wizard_step'] : 1;

    $test_sensor_type = isset($_POST['test_sensor_type']) ? $_POST['test_sensor_type'] : '';

    if ($action === 'skip') {
        // Skip just advances to next step without saving
        header("Location: setup-wizard.php?step=" . ($post_step + 1));
        exit();
    }

    // Validate and save based on step
    $v = new Valitron\Validator($_POST);
    $update_fields = [];
    $update_values = [];

    switch ($post_step) {
        case 1: // Basic Info
            $v->rule('required', ['HIVENAME', 'CITY', 'STATE'])->message('{field} is required');
            $v->rule('regex', 'HIVENAME', '/^[a-zA-Z0-9_-]+$/')->message('Hive Name can only contain letters, numbers, dashes, and underscores');
            $v->rule('lengthMax', ['HIVENAME', 'CITY', 'STATE'], 40);
            $v->rule('lengthMax', ['LATITUDE', 'LONGITUDE'], 20);
            if ($v->validate()) {
                $update_fields = ['HIVENAME=?', 'CITY=?', 'STATE=?', 'TIMEZONE=?', 'LATITUDE=?', 'LONGITUDE=?'];
                $update_values = [
                    test_input($_POST['HIVENAME']),
                    test_input($_POST['CITY']),
                    test_input($_POST['STATE']),
                    test_input_allow_slash($_POST['TIMEZONE'] ?? ($config['TIMEZONE'] ?? 'America/New_York')),
                    test_input($_POST['LATITUDE'] ?? ($config['LATITUDE'] ?? '')),
                    test_input($_POST['LONGITUDE'] ?? ($config['LONGITUDE'] ?? ''))
                ];
            }
            break;

        case 2: // Temp/Humidity
            $enable = test_input($_POST['ENABLE_HIVE_TEMP_CHK'] ?? 'no');
            $update_fields = ['ENABLE_HIVE_TEMP_CHK=?'];
            $update_values = [$enable];
            if ($enable === 'yes') {
                $v->rule('required', ['TEMPTYPE'])->message('Sensor type is required');
                if ($v->validate()) {
                    $update_fields = array_merge($update_fields, ['TEMPTYPE=?', 'HIVEDEVICE=?', 'HIVE_TEMP_SUB=?', 'HIVE_TEMP_GPIO=?', 'HIVE_TEMP_SLOPE=?', 'HIVE_TEMP_INTERCEPT=?', 'HIVE_HUMIDITY_SLOPE=?', 'HIVE_HUMIDITY_INTERCEPT=?']);
                    $update_values = array_merge($update_values, [
                        test_input($_POST['TEMPTYPE']),
                        test_input($_POST['HIVEDEVICE'] ?? ($config['HIVEDEVICE'] ?? '/dev/hidraw1')),
                        test_input($_POST['HIVE_TEMP_SUB'] ?? ($config['HIVE_TEMP_SUB'] ?? '')),
                        test_input($_POST['HIVE_TEMP_GPIO'] ?? ($config['HIVE_TEMP_GPIO'] ?? '2')),
                        test_input($_POST['HIVE_TEMP_SLOPE'] ?? '1'),
                        test_input($_POST['HIVE_TEMP_INTERCEPT'] ?? '0'),
                        test_input($_POST['HIVE_HUMIDITY_SLOPE'] ?? '1'),
                        test_input($_POST['HIVE_HUMIDITY_INTERCEPT'] ?? '0')
                    ]);
                }
            }
            break;

        case 3: // Weight
            $enable = test_input($_POST['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no');
            $update_fields = ['ENABLE_HIVE_WEIGHT_CHK=?'];
            $update_values = [$enable];
            if ($enable === 'yes') {
                $v->rule('required', ['SCALETYPE'])->message('Scale type is required');
                if ($v->validate()) {
                    $update_fields = array_merge($update_fields, ['SCALETYPE=?', 'HIVE_WEIGHT_GPIO=?', 'HIVE_WEIGHT_SLOPE=?', 'HIVE_WEIGHT_INTERCEPT=?', 'WEIGHT_COMPENSATION_ENABLED=?']);
                    $update_values = array_merge($update_values, [
                        test_input($_POST['SCALETYPE']),
                        test_input($_POST['HIVE_WEIGHT_GPIO'] ?? ($config['HIVE_WEIGHT_GPIO'] ?? '')),
                        test_input($_POST['HIVE_WEIGHT_SLOPE'] ?? '1'),
                        test_input($_POST['HIVE_WEIGHT_INTERCEPT'] ?? '0'),
                        test_input($_POST['WEIGHT_COMPENSATION_ENABLED'] ?? 'no')
                    ]);
                }
            }
            break;

        case 4: // Lux
            $enable = test_input($_POST['ENABLE_LUX'] ?? 'no');
            $update_fields = ['ENABLE_LUX=?'];
            $update_values = [$enable];
            if ($enable === 'yes') {
                $v->rule('required', ['LUX_SOURCE'])->message('Light source is required');
                if ($v->validate()) {
                    $update_fields = array_merge($update_fields, ['LUX_SOURCE=?', 'HIVE_LUX_GPIO=?', 'HIVE_LUX_SLOPE=?', 'HIVE_LUX_INTERCEPT=?']);
                    $update_values = array_merge($update_values, [
                        test_input($_POST['LUX_SOURCE']),
                        test_input($_POST['HIVE_LUX_GPIO'] ?? ($config['HIVE_LUX_GPIO'] ?? '')),
                        test_input($_POST['HIVE_LUX_SLOPE'] ?? '1'),
                        test_input($_POST['HIVE_LUX_INTERCEPT'] ?? '0')
                    ]);
                }
            }
            break;

        case 5: // Weather
            $v->rule('required', ['WEATHER_LEVEL'])->message('Weather source is required');
            if ($v->validate()) {
                $update_fields = ['WEATHER_LEVEL=?', 'KEY=?', 'WXSTATION=?', 'WXTEMPTYPE=?', 'WX_TEMPER_DEVICE=?', 'WX_TEMP_GPIO=?', 'local_wx_type=?', 'local_wx_url=?'];
                $update_values = [
                    test_input($_POST['WEATHER_LEVEL']),
                    test_input($_POST['KEY'] ?? ($config['KEY'] ?? '')),
                    test_input($_POST['WXSTATION'] ?? ($config['WXSTATION'] ?? '')),
                    test_input($_POST['WXTEMPTYPE'] ?? ($config['WXTEMPTYPE'] ?? '')),
                    test_input_allow_slash($_POST['WX_TEMPER_DEVICE'] ?? ($config['WX_TEMPER_DEVICE'] ?? '')),
                    test_input($_POST['WX_TEMP_GPIO'] ?? ($config['WX_TEMP_GPIO'] ?? '3')),
                    test_input($_POST['local_wx_type'] ?? ($config['local_wx_type'] ?? '')),
                    test_input($_POST['local_wx_url'] ?? ($config['local_wx_url'] ?? ''))
                ];
            }
            break;

        case 6: // Camera & Counter
            $update_fields = ['ENABLE_HIVE_CAMERA=?', 'CAMERATYPE=?', 'ENABLE_BEECOUNTER=?', 'COUNTERTYPE=?'];
            $update_values = [
                test_input($_POST['ENABLE_HIVE_CAMERA'] ?? 'no'),
                test_input($_POST['CAMERATYPE'] ?? ($config['CAMERATYPE'] ?? '')),
                test_input($_POST['ENABLE_BEECOUNTER'] ?? 'no'),
                test_input($_POST['COUNTERTYPE'] ?? ($config['COUNTERTYPE'] ?? ''))
            ];
            break;

        case 7: // Air Quality
            $enable = test_input($_POST['ENABLE_AIR'] ?? 'no');
            $enableAirnow = test_input($_POST['ENABLE_AIRNOW'] ?? 'no');
            $update_fields = ['ENABLE_AIR=?', 'ENABLE_AIRNOW=?'];
            $update_values = [$enable, $enableAirnow];
            if ($enable === 'yes') {
                $airType = test_input($_POST['AIR_TYPE'] ?? ($config['AIR_TYPE'] ?? ''));
                $update_fields = array_merge($update_fields, ['AIR_TYPE=?', 'AIR_ID=?', 'AIR_API=?', 'AIR_LOCAL_URL=?']);
                $update_values = array_merge($update_values, [
                    $airType,
                    test_input($_POST['AIR_ID'] ?? ($config['AIR_ID'] ?? '')),
                    test_input($_POST['AIR_API'] ?? ($config['AIR_API'] ?? '')),
                    test_input($_POST['AIR_LOCAL_URL'] ?? ($config['AIR_LOCAL_URL'] ?? ''))
                ]);
            }
            if ($enableAirnow === 'yes') {
                $update_fields = array_merge($update_fields, ['KEY_AIRNOW=?', 'AIRNOW_DISTANCE=?']);
                $update_values = array_merge($update_values, [
                    test_input($_POST['KEY_AIRNOW'] ?? ($config['KEY_AIRNOW'] ?? '')),
                    test_input($_POST['AIRNOW_DISTANCE'] ?? ($config['AIRNOW_DISTANCE'] ?? '25'))
                ]);
            }
            break;
    }

    if (!empty($update_fields) && (empty($v->errors()) || $v->validate())) {
        // Increment version
        $ver = $conn->prepare("SELECT version FROM hiveconfig");
        $ver->execute();
        $version = $ver->fetchColumn() + 1;
        $update_fields[] = 'version=?';
        $update_values[] = $version;

        $sql = "UPDATE hiveconfig SET " . implode(', ', $update_fields);
        $stmt = $conn->prepare($sql);
        $stmt->execute($update_values);

        // Regenerate hiveconfig.inc so sensor scripts use the updated config immediately
        shell_exec("sudo /home/HiveControl/scripts/data/dump_hiveconfig_inc.sh 2>/dev/null");

        // Reload config
        $sth = $conn->prepare("SELECT * FROM hiveconfig");
        $sth->execute();
        $config = $sth->fetch(PDO::FETCH_ASSOC);
        if (!$config) { $config = []; }

        $saved = true;

        if ($action === 'test' && !empty($test_sensor_type)) {
            header("Location: setup-wizard.php?step=" . $post_step . "&saved=1&autotest=" . urlencode($test_sensor_type));
            exit();
        }

        $next_step = $post_step + 1;
        if ($next_step > 8) { $next_step = 8; }

        if ($post_step < 8) {
            header("Location: setup-wizard.php?step=" . $next_step . "&saved=1");
            exit();
        }
    } else {
        $errors = array_values($v->errors());
    }
}

if (isset($_GET['saved'])) { $saved = true; }

// Step definitions for progress bar
$steps = [
    1 => ['label' => 'Basic Info',    'icon' => 'fa-home'],
    2 => ['label' => 'Temp/Humidity', 'icon' => 'fa-fire'],
    3 => ['label' => 'Weight',        'icon' => 'fa-dashboard'],
    4 => ['label' => 'Light',         'icon' => 'fa-sun-o'],
    5 => ['label' => 'Weather',       'icon' => 'fa-cloud'],
    6 => ['label' => 'Camera',        'icon' => 'fa-camera'],
    7 => ['label' => 'Air Quality',   'icon' => 'fa-leaf'],
    8 => ['label' => 'Review',        'icon' => 'fa-check-circle'],
];

// Determine which steps have been configured
function stepConfigured($config, $step) {
    if (empty($config)) return false;
    switch ($step) {
        case 1: return !empty($config['HIVENAME']) && $config['HIVENAME'] !== 'NotSet';
        case 2: return ($config['ENABLE_HIVE_TEMP_CHK'] ?? 'no') === 'yes';
        case 3: return ($config['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no') === 'yes';
        case 4: return ($config['ENABLE_LUX'] ?? 'no') === 'yes';
        case 5: return !empty($config['WEATHER_LEVEL']);
        case 6: return ($config['ENABLE_HIVE_CAMERA'] ?? 'no') === 'yes' || ($config['ENABLE_BEECOUNTER'] ?? 'no') === 'yes';
        case 7: return ($config['ENABLE_AIR'] ?? 'no') === 'yes' || ($config['ENABLE_AIRNOW'] ?? 'no') === 'yes';
        case 8: return false;
        default: return false;
    }
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>HiveControl - Setup Wizard</title>

    <link href="../bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">
    <link href="../bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">
    <link href="../dist/css/sb-admin-2.css" rel="stylesheet">
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" integrity="sha256-p4NxAoJBhIIN+hmNHrzRCf9tD/miZyoHS5obTRR9BMY=" crossorigin="" />

    <style>
        body { background-color: #f8f8f8 !important; background-image: none !important; min-height: 100vh; padding: 20px 0; }
        .wizard-container { max-width: 900px; margin: 20px auto; background: white; border-radius: 4px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); overflow: hidden; border: 1px solid #e7e7e7; }
        .wizard-header { background: #e6b800; color: white; padding: 20px 30px; text-align: center; border-bottom: 1px solid #d4a600; }
        .wizard-header h1 { margin: 0; font-size: 28px; font-weight: bold; }
        .wizard-header p { margin: 8px 0 0; font-size: 14px; opacity: 0.95; }
        .wizard-body { padding: 30px 40px; }

        /* Progress bar */
        .wizard-steps { background: #f9f9f9; padding: 15px 20px; border-bottom: 1px solid #e7e7e7; display: flex; justify-content: center; flex-wrap: wrap; gap: 4px; }
        .wizard-step { display: inline-flex; align-items: center; padding: 6px 12px; border-radius: 4px; font-size: 12px; font-weight: 600; color: #999; background: #eee; text-decoration: none; transition: all 0.2s; }
        .wizard-step:hover { text-decoration: none; color: #666; background: #e0e0e0; }
        .wizard-step.active { background: #e6b800; color: white; }
        .wizard-step.configured { background: #5cb85c; color: white; }
        .wizard-step.configured:hover { background: #4cae4c; color: white; }
        .wizard-step .fa { margin-right: 4px; }

        /* Form */
        .form-group label { font-weight: 600; color: #333; }
        .form-control { height: 40px; border-radius: 4px; border: 1px solid #ccc; font-size: 14px; }
        .form-control:focus { border-color: #e6b800; box-shadow: inset 0 1px 1px rgba(0,0,0,.075), 0 0 8px rgba(230,184,0,.6); }
        .help-text { font-size: 13px; color: #737373; margin-top: 5px; }

        /* Buttons */
        .btn-primary { background-color: #e6b800; border-color: #d4a600; font-weight: 600; }
        .btn-primary:hover, .btn-primary:focus { background-color: #d4a600; border-color: #c29900; }
        .btn-skip { background-color: #999; border-color: #888; color: white; font-weight: 600; }
        .btn-skip:hover { background-color: #888; border-color: #777; color: white; }
        .btn-back { background-color: #996633; border-color: #885a2d; color: white; font-weight: 600; }
        .btn-back:hover, .btn-back:focus { background-color: #885a2d; border-color: #774e26; color: white; }
        .wizard-buttons { margin-top: 30px; display: flex; justify-content: space-between; align-items: center; }
        .wizard-buttons .btn { padding: 10px 25px; font-size: 14px; }

        /* Sensor options */
        .sensor-option { padding: 12px 15px; border: 2px solid #e7e7e7; border-radius: 4px; margin-bottom: 8px; cursor: pointer; transition: all 0.2s; }
        .sensor-option:hover { border-color: #e6b800; background: #fffef5; }
        .sensor-option.selected { border-color: #e6b800; background: #fffef5; }
        .sensor-option input[type="radio"] { margin-right: 8px; }
        .sensor-option label { cursor: pointer; margin-bottom: 0; font-weight: normal; }

        /* Test panel */
        .test-panel { background: #f5f5f5; border: 1px solid #e0e0e0; border-radius: 4px; padding: 20px; margin-top: 20px; }
        .test-panel h4 { margin-top: 0; color: #333; }
        .test-result { font-family: monospace; background: #222; color: #0f0; padding: 15px; border-radius: 4px; margin-top: 10px; min-height: 50px; font-size: 14px; }
        .test-result.waiting { color: #999; }
        .test-result.error { color: #f55; }
        .test-result.success { color: #0f0; }
        .btn-test { background-color: #5cb85c; border-color: #4cae4c; color: white; font-weight: 600; }
        .btn-test:hover { background-color: #4cae4c; border-color: #398439; color: white; }

        /* Enable toggle */
        .enable-toggle { background: #f0f0f0; padding: 15px 20px; border-radius: 4px; margin-bottom: 20px; display: flex; align-items: center; justify-content: space-between; }
        .enable-toggle label { margin-bottom: 0; font-size: 16px; font-weight: 600; }

        /* Section heading */
        .step-title { font-size: 22px; font-weight: 600; color: #333; margin-bottom: 5px; }
        .step-description { color: #737373; margin-bottom: 20px; font-size: 14px; }

        /* Review table */
        .review-table td { padding: 8px 12px; }
        .review-table .label-col { font-weight: 600; color: #555; width: 200px; }
        .review-table .value-col { color: #333; }
        .review-section { margin-bottom: 20px; }
        .review-section h4 { color: #e6b800; border-bottom: 2px solid #e6b800; padding-bottom: 8px; margin-bottom: 10px; }
        .badge-enabled { background: #5cb85c; }
        .badge-disabled { background: #999; }
    </style>
</head>
<body>
    <div class="wizard-container">
        <div class="wizard-header" style="position:relative">
            <a href="/pages/index.php" style="position:absolute;top:12px;right:15px;color:rgba(255,255,255,0.85);font-size:13px;text-decoration:none" title="Return to Dashboard"><i class="fa fa-arrow-left"></i> Exit to Dashboard</a>
            <i class="fa fa-home" style="font-size: 40px;"></i>
            <h1>HiveControl Setup Wizard</h1>
            <p>Configure your sensors step by step</p>
        </div>

        <!-- Progress Steps -->
        <div class="wizard-steps">
            <?php foreach ($steps as $num => $s): ?>
                <a href="setup-wizard.php?step=<?php echo $num; ?>"
                   class="wizard-step <?php echo ($num === $step) ? 'active' : (stepConfigured($config, $num) ? 'configured' : ''); ?>">
                    <i class="fa <?php echo $s['icon']; ?>"></i>
                    <?php echo $s['label']; ?>
                </a>
            <?php endforeach; ?>
        </div>

        <div class="wizard-body">
            <?php if (!empty($errors)): ?>
                <div class="alert alert-danger alert-dismissable">
                    <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>
                    <strong>Please fix the following:</strong>
                    <ul style="margin: 10px 0 0;">
                        <?php foreach ($errors as $error): ?>
                            <li><?php echo is_array($error) ? htmlspecialchars($error[0]) : htmlspecialchars($error); ?></li>
                        <?php endforeach; ?>
                    </ul>
                </div>
            <?php endif; ?>

            <?php if ($saved && $step !== 8): ?>
                <div class="alert alert-success alert-dismissable">
                    <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>
                    <i class="fa fa-check"></i> Settings saved successfully.
                </div>
            <?php endif; ?>


<?php
// ==================== STEP 1: BASIC INFO ====================
if ($step === 1): ?>
            <div class="step-title"><i class="fa fa-home"></i> Basic Information</div>
            <div class="step-description">Set up your hive's identity and location. These are required for your system to operate.</div>

            <form method="POST" action="setup-wizard.php?step=1">
                <input type="hidden" name="wizard_step" value="1">
                <input type="hidden" name="wizard_action" value="next" id="wizard_action_field">
                <input type="hidden" name="test_sensor_type" value="" id="test_sensor_type_field">

                <div class="form-group">
                    <label for="HIVENAME">Hive Name <span style="color:#d9534f;">*</span></label>
                    <input type="text" class="form-control" name="HIVENAME" value="<?php echo htmlspecialchars($config['HIVENAME'] ?? ''); ?>" id="HIVENAME" placeholder="e.g., Hive-01, Backyard-Hive" required>
                    <p class="help-text">A unique name to identify this hive (letters, numbers, dashes, underscores)</p>
                </div>

                <div class="row">
                    <div class="col-md-4">
                        <div class="form-group">
                            <label for="CITY">City <span style="color:#d9534f;">*</span></label>
                            <input type="text" class="form-control" name="CITY" value="<?php echo htmlspecialchars($config['CITY'] ?? ''); ?>" placeholder="e.g., Portland" required>
                        </div>
                    </div>
                    <div class="col-md-4">
                        <div class="form-group">
                            <label for="STATE">State <span style="color:#d9534f;">*</span></label>
                            <input type="text" class="form-control" name="STATE" value="<?php echo htmlspecialchars($config['STATE'] ?? ''); ?>" placeholder="e.g., Oregon" required>
                        </div>
                    </div>
                    <div class="col-md-4">
                        <div class="form-group">
                            <label for="TIMEZONE">Timezone</label>
                            <select class="form-control" name="TIMEZONE">
                                <?php
                                $timezones = [
                                    'Pacific/Honolulu' => '(UTC-10:00) Hawaii',
                                    'US/Alaska' => '(UTC-09:00) Alaska',
                                    'America/Los_Angeles' => '(UTC-08:00) Pacific Time',
                                    'US/Arizona' => '(UTC-07:00) Arizona',
                                    'America/Denver' => '(UTC-07:00) Mountain Time',
                                    'America/Chicago' => '(UTC-06:00) Central Time',
                                    'America/New_York' => '(UTC-05:00) Eastern Time',
                                    'America/Halifax' => '(UTC-04:00) Atlantic Time',
                                    'America/St_Johns' => '(UTC-03:30) Newfoundland',
                                    'Europe/London' => '(UTC+00:00) London',
                                    'Europe/Paris' => '(UTC+01:00) Paris',
                                    'Europe/Helsinki' => '(UTC+02:00) Helsinki',
                                    'Australia/Sydney' => '(UTC+10:00) Sydney',
                                    'Pacific/Auckland' => '(UTC+12:00) Auckland',
                                ];
                                foreach ($timezones as $tz => $label) {
                                    $sel = (($config['TIMEZONE'] ?? '') === $tz) ? "selected" : "";
                                    echo "<option value=\"$tz\" $sel>$label</option>";
                                }
                                ?>
                            </select>
                        </div>
                    </div>
                </div>

                <div style="margin-top: 20px;">
                    <label><i class="fa fa-map-marker"></i> Hive Location</label>
                    <p class="help-text">Enter your city and state above, then click "Look Up" to find your coordinates. Drag the marker to fine-tune the location.</p>
                    <div class="row" style="margin-bottom: 10px;">
                        <div class="col-md-3">
                            <div class="form-group" style="margin-bottom:5px;">
                                <label for="LATITUDE">Latitude</label>
                                <input type="text" class="form-control" name="LATITUDE" id="LATITUDE" value="<?php echo htmlspecialchars($config['LATITUDE'] ?? ''); ?>" placeholder="e.g., 41.1400">
                            </div>
                        </div>
                        <div class="col-md-3">
                            <div class="form-group" style="margin-bottom:5px;">
                                <label for="LONGITUDE">Longitude</label>
                                <input type="text" class="form-control" name="LONGITUDE" id="LONGITUDE" value="<?php echo htmlspecialchars($config['LONGITUDE'] ?? ''); ?>" placeholder="e.g., -73.2600">
                            </div>
                        </div>
                        <div class="col-md-3" style="padding-top: 25px;">
                            <button type="button" class="btn btn-test" id="btn-geocode" onclick="geocodeLocation()"><i class="fa fa-search"></i> Look Up</button>
                        </div>
                    </div>
                    <div id="geocode-status" style="margin-bottom: 8px; font-size: 13px; color: #737373;"></div>
                    <div id="location-map" style="height: 300px; border: 1px solid #ccc; border-radius: 4px;"></div>
                </div>

                <div class="wizard-buttons">
                    <a href="/pages/index.php" class="btn btn-back"><i class="fa fa-dashboard"></i> Dashboard</a>
                    <button type="submit" class="btn btn-primary"><i class="fa fa-arrow-right"></i> Next: Temp/Humidity</button>
                </div>
            </form>


<?php
// ==================== STEP 2: TEMP/HUMIDITY ====================
elseif ($step === 2): ?>
            <div class="step-title"><i class="fa fa-fire"></i> Temperature &amp; Humidity Sensor</div>
            <div class="step-description">Configure the sensor that monitors conditions inside your hive.</div>

            <form method="POST" action="setup-wizard.php?step=2" id="step-form">
                <input type="hidden" name="wizard_step" value="2">
                <input type="hidden" name="wizard_action" value="next" id="wizard_action_field">
                <input type="hidden" name="test_sensor_type" value="" id="test_sensor_type_field">

                <div class="enable-toggle">
                    <label><i class="fa fa-fire"></i> Enable Temp/Humidity Sensor</label>
                    <select name="ENABLE_HIVE_TEMP_CHK" id="sensor-enable" class="form-control" style="width:120px;">
                        <option value="yes" <?php if (($config['ENABLE_HIVE_TEMP_CHK'] ?? 'no') === 'yes') echo 'selected'; ?>>Enabled</option>
                        <option value="no" <?php if (($config['ENABLE_HIVE_TEMP_CHK'] ?? 'no') !== 'yes') echo 'selected'; ?>>Disabled</option>
                    </select>
                </div>

                <div id="sensor-config" style="<?php echo (($config['ENABLE_HIVE_TEMP_CHK'] ?? 'no') !== 'yes') ? 'display:none;' : ''; ?>">
                    <label>Select Your Sensor Type:</label>
                    <?php
                    $temp_types = [
                        'temperhum' => ['TemperHum (USB)', 'USB temperature and humidity sensor'],
                        'temper' => ['Temper (USB)', 'USB temperature-only sensor'],
                        'sht41trinkey' => ['SHT41 Trinkey (USB)', 'USB temperature and humidity sensor'],
                        'dht22' => ['DHT22 (GPIO)', 'Digital humidity and temperature sensor'],
                        'dht21' => ['DHT21 (GPIO)', 'Digital humidity and temperature sensor'],
                        'sht31d' => ['SHT31-D (I2C)', 'High-accuracy temperature and humidity sensor'],
                        'bme280' => ['BME280 (I2C)', 'Temperature, humidity, and pressure sensor'],
                        'bme680' => ['BME680 (I2C)', 'Environmental sensor with gas detection'],
                        'aht20' => ['AHT20 (I2C)', 'Temperature and humidity sensor'],
                        'broodminder' => ['BroodMinder T/TH (BLE)', 'Bluetooth brood monitoring sensor'],
                    ];
                    foreach ($temp_types as $val => $info): ?>
                        <div class="sensor-option <?php echo (($config['TEMPTYPE'] ?? '') === $val) ? 'selected' : ''; ?>">
                            <input type="radio" name="TEMPTYPE" value="<?php echo $val; ?>" id="temp_<?php echo $val; ?>"
                                <?php echo (($config['TEMPTYPE'] ?? '') === $val) ? 'checked' : ''; ?>>
                            <label for="temp_<?php echo $val; ?>"><strong><?php echo $info[0]; ?></strong> &mdash; <?php echo $info[1]; ?></label>
                        </div>
                    <?php endforeach; ?>

                    <div id="temp-options" style="margin-top:15px;">
                        <div id="opt-temperhum" class="temp-opt" style="display:none;">
                            <div class="form-group">
                                <label>Device Path</label>
                                <input type="text" class="form-control" name="HIVEDEVICE" value="<?php echo htmlspecialchars($config['HIVEDEVICE'] ?? '/dev/hidraw1'); ?>" placeholder="/dev/hidraw1">
                            </div>
                        </div>
                        <div id="opt-temper" class="temp-opt" style="display:none;">
                            <div class="row">
                                <div class="col-md-6">
                                    <div class="form-group">
                                        <label>Device Path</label>
                                        <input type="text" class="form-control" name="HIVEDEVICE" value="<?php echo htmlspecialchars($config['HIVEDEVICE'] ?? '/dev/hidraw1'); ?>">
                                    </div>
                                </div>
                                <div class="col-md-6">
                                    <div class="form-group">
                                        <label>Sub-Device</label>
                                        <input type="text" class="form-control" name="HIVE_TEMP_SUB" value="<?php echo htmlspecialchars($config['HIVE_TEMP_SUB'] ?? ''); ?>">
                                    </div>
                                </div>
                            </div>
                        </div>
                        <div id="opt-gpio" class="temp-opt" style="display:none;">
                            <div class="form-group">
                                <label>GPIO Pin</label>
                                <input type="text" class="form-control" name="HIVE_TEMP_GPIO" value="<?php echo htmlspecialchars($config['HIVE_TEMP_GPIO'] ?? '2'); ?>" style="width:100px;">
                                <p class="help-text">The GPIO pin number your sensor is connected to</p>
                            </div>
                        </div>
                        <div id="opt-broodminder" class="temp-opt" style="display:none;">
                            <div class="form-group">
                                <label>BLE Device Address</label>
                                <input type="text" class="form-control" name="HIVEDEVICE" id="wiz-ble-mac" value="<?php echo htmlspecialchars($config['HIVEDEVICE'] ?? ''); ?>" placeholder="06:09:42:1c:8a">
                                <button type="button" class="btn btn-info btn-sm ble-discover-btn" data-target="#wiz-ble-mac" style="margin-top:5px"><i class="fa fa-bluetooth fa-fw"></i> Discover Devices</button>
                                <div class="ble-scan-result" style="margin-top:8px"></div>
                                <p class="help-text">Full MAC address in lowercase, as printed on the device</p>
                            </div>
                        </div>
                    </div>

                    <div class="row" style="margin-top:15px;">
                        <div class="col-md-6">
                            <label>Calibration (optional)</label>
                            <div class="row">
                                <div class="col-xs-6">
                                    <div class="form-group">
                                        <label class="help-text">Temp Intercept</label>
                                        <input type="text" class="form-control" name="HIVE_TEMP_INTERCEPT" value="<?php echo htmlspecialchars($config['HIVE_TEMP_INTERCEPT'] ?? '0'); ?>">
                                    </div>
                                </div>
                                <div class="col-xs-6">
                                    <div class="form-group">
                                        <label class="help-text">Temp Slope</label>
                                        <input type="text" class="form-control" name="HIVE_TEMP_SLOPE" value="<?php echo htmlspecialchars($config['HIVE_TEMP_SLOPE'] ?? '1'); ?>">
                                    </div>
                                </div>
                            </div>
                            <div class="row">
                                <div class="col-xs-6">
                                    <div class="form-group">
                                        <label class="help-text">Humidity Intercept</label>
                                        <input type="text" class="form-control" name="HIVE_HUMIDITY_INTERCEPT" value="<?php echo htmlspecialchars($config['HIVE_HUMIDITY_INTERCEPT'] ?? '0'); ?>">
                                    </div>
                                </div>
                                <div class="col-xs-6">
                                    <div class="form-group">
                                        <label class="help-text">Humidity Slope</label>
                                        <input type="text" class="form-control" name="HIVE_HUMIDITY_SLOPE" value="<?php echo htmlspecialchars($config['HIVE_HUMIDITY_SLOPE'] ?? '1'); ?>">
                                    </div>
                                </div>
                            </div>
                        </div>
                        <div class="col-md-6">
                            <div class="test-panel">
                                <h4><i class="fa fa-bolt"></i> Test Sensor</h4>
                                <p class="help-text">Click to read your sensor and verify it's working.</p>
                                <button type="button" class="btn btn-test" onclick="testSensor('hivetemp')"><i class="fa fa-play"></i> Test Now</button>
                                <div class="test-result waiting" id="test-result">Waiting for test...</div>
                            </div>
                        </div>
                    </div>
                </div>

                <div class="wizard-buttons">
                    <a href="setup-wizard.php?step=1" class="btn btn-back"><i class="fa fa-arrow-left"></i> Back</a>
                    <div>
                        <button type="submit" name="wizard_action" value="skip" class="btn btn-skip"><i class="fa fa-forward"></i> Skip</button>
                        <button type="submit" class="btn btn-primary"><i class="fa fa-arrow-right"></i> Next: Weight</button>
                    </div>
                </div>
            </form>


<?php
// ==================== STEP 3: WEIGHT ====================
elseif ($step === 3): ?>
            <div class="step-title"><i class="fa fa-dashboard"></i> Weight Scale</div>
            <div class="step-description">Configure the scale that monitors your hive's weight for tracking nectar flow and colony health.</div>

            <form method="POST" action="setup-wizard.php?step=3" id="step-form">
                <input type="hidden" name="wizard_step" value="3">
                <input type="hidden" name="wizard_action" value="next" id="wizard_action_field">
                <input type="hidden" name="test_sensor_type" value="" id="test_sensor_type_field">

                <div class="enable-toggle">
                    <label><i class="fa fa-dashboard"></i> Enable Weight Scale</label>
                    <select name="ENABLE_HIVE_WEIGHT_CHK" id="sensor-enable" class="form-control" style="width:120px;">
                        <option value="yes" <?php if (($config['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no') === 'yes') echo 'selected'; ?>>Enabled</option>
                        <option value="no" <?php if (($config['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no') !== 'yes') echo 'selected'; ?>>Disabled</option>
                    </select>
                </div>

                <div id="sensor-config" style="<?php echo (($config['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no') !== 'yes') ? 'display:none;' : ''; ?>">
                    <label>Select Your Scale Type:</label>
                    <?php
                    $scale_types = [
                        'hx711' => ['HX711', 'Most common load cell amplifier for DIY hive scales'],
                        'phidget1046' => ['Phidget 1046', 'Phidget bridge interface for strain gauges'],
                        'cpw200plus' => ['CPW 200 Plus', 'Commercial bench scale with serial output'],
                    ];
                    foreach ($scale_types as $val => $info): ?>
                        <div class="sensor-option <?php echo (($config['SCALETYPE'] ?? '') === $val) ? 'selected' : ''; ?>">
                            <input type="radio" name="SCALETYPE" value="<?php echo $val; ?>" id="scale_<?php echo $val; ?>"
                                <?php echo (($config['SCALETYPE'] ?? '') === $val) ? 'checked' : ''; ?>>
                            <label for="scale_<?php echo $val; ?>"><strong><?php echo $info[0]; ?></strong> &mdash; <?php echo $info[1]; ?></label>
                        </div>
                    <?php endforeach; ?>

                    <!-- Calibration Sub-Wizard -->
                    <div id="cal-section" style="margin-top:15px;">

                        <div id="cpw-notice" style="display:none; padding:15px; background:#fcf8e3; border:1px solid #faebcc; border-radius:4px; margin-bottom:15px;">
                            <i class="fa fa-info-circle" style="color:#8a6d3b;"></i>
                            <strong>Factory Calibrated</strong> &mdash; The CPW 200 Plus is a pre-calibrated serial scale. No calibration wizard is needed. Use the Test Scale button to verify readings.
                        </div>

                        <div id="cal-wizard-launcher" style="margin-bottom:15px;">
                            <button type="button" class="btn btn-primary" id="btn-start-cal" onclick="CalWiz.start()" style="margin-right:10px;">
                                <i class="fa fa-sliders"></i> Calibrate Scale
                            </button>
                            <button type="button" class="btn btn-warning" id="btn-start-fieldcal" onclick="FieldCal.start()" style="margin-right:10px;">
                                <i class="fa fa-refresh"></i> Recalibrate Under Load
                            </button>
                            <p class="help-text" style="margin-top:8px;"><strong>Calibrate Scale</strong>: Full calibration (scale must be empty). <strong>Recalibrate Under Load</strong>: Adjust accuracy with hive on the scale using a known weight placed on top.</p>
                        </div>

                        <!-- Field Recalibration Sub-Wizard (hidden until started) -->
                        <div id="fieldcal-wizard" style="display:none; background:#fdf8ec; border:1px solid #f0e0b0; border-radius:4px; padding:20px; margin-bottom:15px;">
                            <div style="margin-bottom:15px; text-align:center;">
                                <span class="fieldcal-prog" id="fieldcal-prog-1" style="display:inline-block; padding:4px 12px; border-radius:12px; font-size:12px; font-weight:600; background:#e6b800; color:white; margin:0 2px;">1. Baseline</span>
                                <span class="fieldcal-prog" id="fieldcal-prog-2" style="display:inline-block; padding:4px 12px; border-radius:12px; font-size:12px; font-weight:600; background:#eee; color:#999; margin:0 2px;">2. Add Weight</span>
                                <span class="fieldcal-prog" id="fieldcal-prog-3" style="display:inline-block; padding:4px 12px; border-radius:12px; font-size:12px; font-weight:600; background:#eee; color:#999; margin:0 2px;">3. Results</span>
                            </div>

                            <!-- Step 1: Baseline -->
                            <div class="fieldcal-step" id="fieldcal-step-1">
                                <h4 style="margin-top:0;"><i class="fa fa-dashboard"></i> Step 1: Baseline Reading</h4>
                                <p>Leave the hive on the scale as-is. <strong>Do not add or remove anything.</strong> We'll take a baseline reading of the current load.</p>
                                <button type="button" class="btn btn-test" id="btn-fieldcal-baseline" onclick="FieldCal.takeBaseline()">
                                    <i class="fa fa-play"></i> Take Baseline Reading
                                </button>
                                <button type="button" class="btn btn-skip" onclick="FieldCal.cancel()" style="margin-left:8px;">Cancel</button>
                                <div id="fieldcal-baseline-result" class="test-result waiting" style="margin-top:10px;">Ready when you are...</div>
                            </div>

                            <!-- Step 2: Add Known Weight -->
                            <div class="fieldcal-step" id="fieldcal-step-2" style="display:none;">
                                <h4 style="margin-top:0;"><i class="fa fa-plus-circle"></i> Step 2: Add Known Weight On Top</h4>
                                <p>Place a <strong>known weight on top of the hive</strong> (e.g. a 10-lb bag of sugar, a dumbbell). Use at least 10 lbs for good accuracy.</p>
                                <div class="form-group" style="max-width:300px;">
                                    <label>Known Weight Added (lbs)</label>
                                    <input type="number" class="form-control" id="fieldcal-known-weight" min="1" max="500" step="0.1" placeholder="e.g. 10">
                                    <p class="help-text">Enter the exact weight you placed on top</p>
                                </div>
                                <button type="button" class="btn btn-test" id="btn-fieldcal-loaded" onclick="FieldCal.takeLoaded()">
                                    <i class="fa fa-play"></i> Take Loaded Reading
                                </button>
                                <button type="button" class="btn btn-back" onclick="FieldCal.showStep(1)" style="margin-left:8px;">
                                    <i class="fa fa-arrow-left"></i> Back
                                </button>
                                <button type="button" class="btn btn-skip" onclick="FieldCal.cancel()" style="margin-left:8px;">Cancel</button>
                                <div id="fieldcal-loaded-result" class="test-result waiting" style="margin-top:10px;">Place weight and click above...</div>
                            </div>

                            <!-- Step 3: Results -->
                            <div class="fieldcal-step" id="fieldcal-step-3" style="display:none;">
                                <h4 style="margin-top:0;"><i class="fa fa-check-circle" style="color:#5cb85c;"></i> Step 3: Recalibration Results</h4>
                                <table class="table" style="max-width:500px; margin-bottom:10px;">
                                    <tr><td style="font-weight:600;">Previous Slope</td><td id="fieldcal-old-slope">--</td></tr>
                                    <tr><td style="font-weight:600;">New Slope</td><td id="fieldcal-new-slope">--</td></tr>
                                    <tr><td style="font-weight:600;">Change</td><td id="fieldcal-slope-change">--</td></tr>
                                    <tr><td style="font-weight:600;">Known Weight Added</td><td id="fieldcal-result-known">--</td></tr>
                                    <tr><td style="font-weight:600;">Measured Delta</td><td id="fieldcal-result-delta">--</td></tr>
                                </table>
                                <div id="fieldcal-warning" style="display:none; padding:10px; background:#fcf8e3; border:1px solid #faebcc; border-radius:4px; color:#8a6d3b; margin-bottom:10px;">
                                    <i class="fa fa-exclamation-triangle"></i> <span id="fieldcal-warning-text"></span>
                                </div>
                                <button type="button" class="btn btn-primary" id="btn-fieldcal-save" onclick="FieldCal.save()">
                                    <i class="fa fa-check"></i> Save New Calibration
                                </button>
                                <button type="button" class="btn btn-back" onclick="FieldCal.start()" style="margin-left:8px;">
                                    <i class="fa fa-refresh"></i> Start Over
                                </button>
                                <button type="button" class="btn btn-skip" onclick="FieldCal.cancel()" style="margin-left:8px;">Cancel</button>
                                <div id="fieldcal-save-result" style="display:none; padding:10px; background:#dff0d8; border:1px solid #d6e9c6; border-radius:4px; color:#3c763d; margin-top:10px;">
                                    <i class="fa fa-check-circle"></i> Recalibration saved successfully! Intercept unchanged, slope updated.
                                </div>
                            </div>
                        </div>

                        <!-- Sub-wizard steps (hidden until started) -->
                        <div id="cal-wizard" style="display:none; background:#f5f5f5; border:1px solid #e0e0e0; border-radius:4px; padding:20px; margin-bottom:15px;">

                            <!-- Progress indicator -->
                            <div style="margin-bottom:15px; text-align:center;">
                                <span class="cal-prog" id="cal-prog-1" style="display:inline-block; padding:4px 12px; border-radius:12px; font-size:12px; font-weight:600; background:#e6b800; color:white; margin:0 2px;">1. Zero</span>
                                <span class="cal-prog" id="cal-prog-2" style="display:inline-block; padding:4px 12px; border-radius:12px; font-size:12px; font-weight:600; background:#eee; color:#999; margin:0 2px;">2. Load</span>
                                <span class="cal-prog" id="cal-prog-3" style="display:inline-block; padding:4px 12px; border-radius:12px; font-size:12px; font-weight:600; background:#eee; color:#999; margin:0 2px;">3. Results</span>
                            </div>

                            <!-- Step 1: Tare/Zero -->
                            <div class="cal-step" id="cal-step-1">
                                <h4 style="margin-top:0;"><i class="fa fa-dashboard"></i> Step 1: Zero the Scale</h4>
                                <p>Remove <strong>all weight</strong> from the scale, including any hive equipment. The scale should be completely empty.</p>
                                <p class="help-text">We'll take a raw sensor reading to establish the zero point.</p>
                                <button type="button" class="btn btn-test" id="btn-zero" onclick="CalWiz.takeZeroReading()">
                                    <i class="fa fa-play"></i> Take Zero Reading
                                </button>
                                <button type="button" class="btn btn-skip" onclick="CalWiz.cancel()" style="margin-left:8px;">Cancel</button>
                                <div id="cal-zero-result" class="test-result waiting" style="margin-top:10px;">Ready when you are...</div>
                            </div>

                            <!-- Step 2: Load -->
                            <div class="cal-step" id="cal-step-2" style="display:none;">
                                <h4 style="margin-top:0;"><i class="fa fa-download"></i> Step 2: Place Known Weight</h4>
                                <p>Place a <strong>known weight</strong> on the scale. Use the heaviest weight you have for better accuracy (e.g. 20-50 lbs).</p>
                                <div class="form-group" style="max-width:300px;">
                                    <label>Known Weight (lbs)</label>
                                    <input type="number" class="form-control" id="cal-known-weight" min="1" max="500" step="0.1" placeholder="e.g. 30">
                                    <p class="help-text">Enter the exact weight in pounds</p>
                                </div>
                                <button type="button" class="btn btn-test" id="btn-loaded" onclick="CalWiz.takeLoadedReading()">
                                    <i class="fa fa-play"></i> Take Loaded Reading
                                </button>
                                <button type="button" class="btn btn-back" onclick="CalWiz.showStep(1)" style="margin-left:8px;">
                                    <i class="fa fa-arrow-left"></i> Back
                                </button>
                                <button type="button" class="btn btn-skip" onclick="CalWiz.cancel()" style="margin-left:8px;">Cancel</button>
                                <div id="cal-loaded-result" class="test-result waiting" style="margin-top:10px;">Place weight and click above...</div>
                            </div>

                            <!-- Step 3: Results -->
                            <div class="cal-step" id="cal-step-3" style="display:none;">
                                <h4 style="margin-top:0;"><i class="fa fa-check-circle" style="color:#5cb85c;"></i> Step 3: Calibration Results</h4>
                                <table class="table" style="max-width:400px; margin-bottom:10px;">
                                    <tr><td style="font-weight:600;">Zero (Intercept)</td><td id="cal-result-intercept">--</td></tr>
                                    <tr><td style="font-weight:600;">Slope</td><td id="cal-result-slope">--</td></tr>
                                    <tr><td style="font-weight:600;">Known Weight</td><td id="cal-result-known">--</td></tr>
                                </table>

                                <div style="margin-bottom:15px;">
                                    <button type="button" class="btn btn-primary" id="btn-verify" onclick="CalWiz.verify()">
                                        <i class="fa fa-check"></i> Save &amp; Verify
                                    </button>
                                    <button type="button" class="btn btn-back" onclick="CalWiz.start()" style="margin-left:8px;">
                                        <i class="fa fa-refresh"></i> Start Over
                                    </button>
                                </div>
                                <div id="cal-verify-result" class="test-result waiting" style="display:none;">Verification result will appear here...</div>
                                <div id="cal-save-result" style="display:none; padding:10px; background:#dff0d8; border:1px solid #d6e9c6; border-radius:4px; color:#3c763d; margin-top:10px;">
                                    <i class="fa fa-check-circle"></i> Calibration saved successfully!
                                </div>
                            </div>
                        </div>

                        <!-- Hidden form fields that get populated by calibration wizard or manual entry -->
                        <input type="hidden" name="HIVE_WEIGHT_INTERCEPT" id="form-intercept" value="<?php echo htmlspecialchars($config['HIVE_WEIGHT_INTERCEPT'] ?? '0'); ?>">
                        <input type="hidden" name="HIVE_WEIGHT_SLOPE" id="form-slope" value="<?php echo htmlspecialchars($config['HIVE_WEIGHT_SLOPE'] ?? '1'); ?>">

                        <!-- Manual entry (collapsible) -->
                        <div style="margin-bottom:15px;">
                            <a href="#" onclick="$('#manual-cal').toggle(); return false;" style="font-size:13px; color:#737373;">
                                <i class="fa fa-wrench"></i> Advanced: Manual Calibration Entry
                            </a>
                        </div>
                        <div id="manual-cal" style="display:none;">
                            <div class="row">
                                <div class="col-md-6">
                                    <div class="form-group">
                                        <label class="help-text">Zero/Intercept</label>
                                        <input type="text" class="form-control" id="manual-intercept" value="<?php echo htmlspecialchars($config['HIVE_WEIGHT_INTERCEPT'] ?? '0'); ?>" onchange="document.getElementById('form-intercept').value=this.value;">
                                        <p class="help-text">Raw sensor value with no weight on the scale</p>
                                    </div>
                                </div>
                                <div class="col-md-6">
                                    <div class="form-group">
                                        <label class="help-text">Calibration/Slope</label>
                                        <input type="text" class="form-control" id="manual-slope" value="<?php echo htmlspecialchars($config['HIVE_WEIGHT_SLOPE'] ?? '1'); ?>" onchange="document.getElementById('form-slope').value=this.value;">
                                        <p class="help-text">Conversion factor from raw reading to lbs</p>
                                    </div>
                                </div>
                            </div>
                        </div>

                        <!-- Test Scale (always available) -->
                        <div class="test-panel">
                            <h4><i class="fa fa-bolt"></i> Test Scale</h4>
                            <p class="help-text">Read the scale using the current calibration values. Place a known weight to verify accuracy.</p>
                            <button type="button" class="btn btn-test" onclick="testSensor('hiveweight')"><i class="fa fa-play"></i> Test Now</button>
                            <div class="test-result waiting" id="test-result">Waiting for test...</div>
                        </div>

                        <!-- Environmental Drift Compensation -->
                        <div style="margin-top:20px; padding:15px; background:#f9f9f9; border:1px solid #e0e0e0; border-radius:4px;">
                            <h4 style="margin-top:0;"><i class="fa fa-fire"></i> Environmental Drift Compensation</h4>
                            <p class="help-text">Automatically corrects weight readings for temperature and humidity drift. Recalibrates weekly using nighttime data when bees are inactive.</p>
                            <select name="WEIGHT_COMPENSATION_ENABLED" class="form-control" style="width:120px;">
                                <option value="yes" <?php if (($config['WEIGHT_COMPENSATION_ENABLED'] ?? 'no') === 'yes') echo 'selected'; ?>>Enabled</option>
                                <option value="no" <?php if (($config['WEIGHT_COMPENSATION_ENABLED'] ?? 'no') !== 'yes') echo 'selected'; ?>>Disabled</option>
                            </select>
                        </div>
                    </div>
                </div>

                <div class="wizard-buttons">
                    <a href="setup-wizard.php?step=2" class="btn btn-back"><i class="fa fa-arrow-left"></i> Back</a>
                    <div>
                        <button type="submit" name="wizard_action" value="skip" class="btn btn-skip"><i class="fa fa-forward"></i> Skip</button>
                        <button type="submit" class="btn btn-primary"><i class="fa fa-arrow-right"></i> Next: Light</button>
                    </div>
                </div>
            </form>


<?php
// ==================== STEP 4: LUX ====================
elseif ($step === 4): ?>
            <div class="step-title"><i class="fa fa-sun-o"></i> Light Sensor</div>
            <div class="step-description">Configure a light sensor to track daylight levels near your hive.</div>

            <form method="POST" action="setup-wizard.php?step=4" id="step-form">
                <input type="hidden" name="wizard_step" value="4">
                <input type="hidden" name="wizard_action" value="next" id="wizard_action_field">
                <input type="hidden" name="test_sensor_type" value="" id="test_sensor_type_field">

                <div class="enable-toggle">
                    <label><i class="fa fa-sun-o"></i> Enable Light Sensor</label>
                    <select name="ENABLE_LUX" id="sensor-enable" class="form-control" style="width:120px;">
                        <option value="yes" <?php if (($config['ENABLE_LUX'] ?? 'no') === 'yes') echo 'selected'; ?>>Enabled</option>
                        <option value="no" <?php if (($config['ENABLE_LUX'] ?? 'no') !== 'yes') echo 'selected'; ?>>Disabled</option>
                    </select>
                </div>

                <div id="sensor-config" style="<?php echo (($config['ENABLE_LUX'] ?? 'no') !== 'yes') ? 'display:none;' : ''; ?>">
                    <label>Select Light Source:</label>
                    <?php
                    $lux_types = [
                        'tsl2591' => ['TSL 2591 (I2C)', 'High-range digital light sensor'],
                        'tsl2561' => ['TSL 2561 (I2C)', 'Visible and infrared light sensor'],
                        'wx' => ['Weather Station', 'Use light data from your weather source'],
                    ];
                    foreach ($lux_types as $val => $info): ?>
                        <div class="sensor-option <?php echo (($config['LUX_SOURCE'] ?? '') === $val) ? 'selected' : ''; ?>">
                            <input type="radio" name="LUX_SOURCE" value="<?php echo $val; ?>" id="lux_<?php echo $val; ?>"
                                <?php echo (($config['LUX_SOURCE'] ?? '') === $val) ? 'checked' : ''; ?>>
                            <label for="lux_<?php echo $val; ?>"><strong><?php echo $info[0]; ?></strong> &mdash; <?php echo $info[1]; ?></label>
                        </div>
                    <?php endforeach; ?>

                    <div class="row" style="margin-top:15px;">
                        <div class="col-md-6">
                            <label>Calibration (optional)</label>
                            <div class="form-group">
                                <label class="help-text">Intercept</label>
                                <input type="text" class="form-control" name="HIVE_LUX_INTERCEPT" value="<?php echo htmlspecialchars($config['HIVE_LUX_INTERCEPT'] ?? '0'); ?>">
                            </div>
                            <div class="form-group">
                                <label class="help-text">Slope</label>
                                <input type="text" class="form-control" name="HIVE_LUX_SLOPE" value="<?php echo htmlspecialchars($config['HIVE_LUX_SLOPE'] ?? '1'); ?>">
                            </div>
                        </div>
                        <div class="col-md-6">
                            <div class="test-panel">
                                <h4><i class="fa fa-bolt"></i> Test Light Sensor</h4>
                                <p class="help-text">Click to read the current light level.</p>
                                <button type="button" class="btn btn-test" onclick="testSensor('hivelux')"><i class="fa fa-play"></i> Test Now</button>
                                <div class="test-result waiting" id="test-result">Waiting for test...</div>
                            </div>
                        </div>
                    </div>
                </div>

                <div class="wizard-buttons">
                    <a href="setup-wizard.php?step=3" class="btn btn-back"><i class="fa fa-arrow-left"></i> Back</a>
                    <div>
                        <button type="submit" name="wizard_action" value="skip" class="btn btn-skip"><i class="fa fa-forward"></i> Skip</button>
                        <button type="submit" class="btn btn-primary"><i class="fa fa-arrow-right"></i> Next: Weather</button>
                    </div>
                </div>
            </form>


<?php
// ==================== STEP 5: WEATHER ====================
elseif ($step === 5): ?>
            <div class="step-title"><i class="fa fa-cloud"></i> Weather Source</div>
            <div class="step-description">Choose where to get ambient weather data (outdoor temperature, humidity, wind, rain).</div>

            <form method="POST" action="setup-wizard.php?step=5" id="step-form">
                <input type="hidden" name="wizard_step" value="5">
                <input type="hidden" name="wizard_action" value="next" id="wizard_action_field">
                <input type="hidden" name="test_sensor_type" value="" id="test_sensor_type_field">

                <label>Select Weather Source:</label>
                <?php
                $wx_types = [
                    'openmeteo' => ['Open-Meteo', 'Free, no API key &mdash; uses latitude/longitude (global coverage)'],
                    'openweathermap' => ['OpenWeatherMap', 'Free tier (1000 calls/day) &mdash; requires API key'],
                    'nws' => ['NWS (weather.gov)', 'Free, no API key &mdash; US locations only'],
                    'weatherapi' => ['WeatherAPI.com', 'Free tier (1M calls/month) &mdash; requires API key'],
                    'visualcrossing' => ['Visual Crossing', 'Free tier (1000 calls/day) &mdash; requires API key, includes solar radiation'],
                    'ambientwx' => ['AmbientWeather.net', 'Ambient Weather network &mdash; requires API key and station MAC'],
                    'hive' => ['WX Underground (Legacy)', 'Weather Underground &mdash; requires station ID'],
                    'localws' => ['Local Weather Station', 'IP-connected weather station on your network'],
                    'localsensors' => ['Local Hive Sensors', 'Use locally connected temp/humidity sensors for weather'],
                    'wf_tempest_local' => ['WeatherFlow Tempest (UDP)', 'Local UDP broadcast from Tempest station'],
                ];
                foreach ($wx_types as $val => $info): ?>
                    <div class="sensor-option <?php echo (($config['WEATHER_LEVEL'] ?? '') === $val) ? 'selected' : ''; ?>">
                        <input type="radio" name="WEATHER_LEVEL" value="<?php echo $val; ?>" id="wx_<?php echo $val; ?>"
                            <?php echo (($config['WEATHER_LEVEL'] ?? '') === $val) ? 'checked' : ''; ?>
                            onchange="showWxOptions()">
                        <label for="wx_<?php echo $val; ?>"><strong><?php echo $info[0]; ?></strong> &mdash; <?php echo $info[1]; ?></label>
                    </div>
                <?php endforeach; ?>

                <!-- Open-Meteo options (no config needed) -->
                <div id="wx-opt-openmeteo" class="wx-opt" style="display:none; margin-top:15px;">
                    <div class="alert alert-info">
                        <i class="fa fa-info-circle"></i> Open-Meteo uses your latitude/longitude from Step 2. No API key required. Data is sourced from ECMWF, DWD, and NOAA weather models.
                    </div>
                </div>

                <!-- OpenWeatherMap options -->
                <div id="wx-opt-openweathermap" class="wx-opt" style="display:none; margin-top:15px;">
                    <div class="form-group">
                        <label>API Key</label>
                        <input type="text" class="form-control" name="KEY" value="<?php echo htmlspecialchars($config['KEY'] ?? ''); ?>" placeholder="Get free key at openweathermap.org">
                        <p class="help-block">Sign up at <strong>openweathermap.org</strong> for a free API key (1000 calls/day).</p>
                    </div>
                </div>

                <!-- NWS options (no config needed) -->
                <div id="wx-opt-nws" class="wx-opt" style="display:none; margin-top:15px;">
                    <div class="alert alert-info">
                        <i class="fa fa-info-circle"></i> NWS uses your latitude/longitude to find the nearest observation station automatically. No API key required. <strong>US locations only.</strong>
                    </div>
                </div>

                <!-- WeatherAPI.com options -->
                <div id="wx-opt-weatherapi" class="wx-opt" style="display:none; margin-top:15px;">
                    <div class="form-group">
                        <label>API Key</label>
                        <input type="text" class="form-control" name="KEY" value="<?php echo htmlspecialchars($config['KEY'] ?? ''); ?>" placeholder="Get free key at weatherapi.com">
                        <p class="help-block">Sign up at <strong>weatherapi.com</strong> for a free API key (1M calls/month).</p>
                    </div>
                </div>

                <!-- Visual Crossing options -->
                <div id="wx-opt-visualcrossing" class="wx-opt" style="display:none; margin-top:15px;">
                    <div class="form-group">
                        <label>API Key</label>
                        <input type="text" class="form-control" name="KEY" value="<?php echo htmlspecialchars($config['KEY'] ?? ''); ?>" placeholder="Get free key at visualcrossing.com">
                        <p class="help-block">Sign up at <strong>visualcrossing.com</strong> for a free API key (1000 calls/day). Includes solar radiation data.</p>
                    </div>
                </div>

                <!-- WX Underground options (legacy) -->
                <div id="wx-opt-hive" class="wx-opt" style="display:none; margin-top:15px;">
                    <div class="form-group">
                        <label>Station ID</label>
                        <input type="text" class="form-control" name="WXSTATION" value="<?php echo htmlspecialchars($config['WXSTATION'] ?? ''); ?>" placeholder="e.g., KORPOR42">
                    </div>
                </div>

                <!-- Ambient WX options -->
                <div id="wx-opt-ambientwx" class="wx-opt" style="display:none; margin-top:15px;">
                    <div class="row">
                        <div class="col-md-6">
                            <div class="form-group">
                                <label>API Key</label>
                                <input type="text" class="form-control" name="KEY" value="<?php echo htmlspecialchars($config['KEY'] ?? ''); ?>">
                            </div>
                        </div>
                        <div class="col-md-6">
                            <div class="form-group">
                                <label>Station ID</label>
                                <input type="text" class="form-control" name="WXSTATION" value="<?php echo htmlspecialchars($config['WXSTATION'] ?? ''); ?>">
                            </div>
                        </div>
                    </div>
                </div>

                <!-- Local WS options -->
                <div id="wx-opt-localws" class="wx-opt" style="display:none; margin-top:15px;">
                    <div class="row">
                        <div class="col-md-6">
                            <label>Station Type</label>
                            <div class="sensor-option">
                                <input type="radio" name="local_wx_type" value="WS1400ip" id="lwx_ws1400" <?php echo (($config['local_wx_type'] ?? '') === 'WS1400ip') ? 'checked' : ''; ?>>
                                <label for="lwx_ws1400">WS1400ip</label>
                            </div>
                            <div class="sensor-option">
                                <input type="radio" name="local_wx_type" value="ourweather" id="lwx_ow" <?php echo (($config['local_wx_type'] ?? '') === 'ourweather') ? 'checked' : ''; ?>>
                                <label for="lwx_ow">OurWeather</label>
                            </div>
                        </div>
                        <div class="col-md-6">
                            <div class="form-group">
                                <label>Station IP Address</label>
                                <input type="text" class="form-control" name="local_wx_url" value="<?php echo htmlspecialchars($config['local_wx_url'] ?? ''); ?>" placeholder="192.168.1.100">
                            </div>
                        </div>
                    </div>
                </div>

                <!-- Local Sensors options -->
                <div id="wx-opt-localsensors" class="wx-opt" style="display:none; margin-top:15px;">
                    <label>Sensor Type</label>
                    <?php
                    $wx_sensor_types = ['temperhum','dht21','dht22','sht31d','bme280','bme680','aht20','sht41trinkey'];
                    foreach ($wx_sensor_types as $wst): ?>
                        <div class="sensor-option" style="display:inline-block; margin-right:5px;">
                            <input type="radio" name="WXTEMPTYPE" value="<?php echo $wst; ?>" id="wst_<?php echo $wst; ?>"
                                <?php echo (($config['WXTEMPTYPE'] ?? '') === $wst) ? 'checked' : ''; ?>>
                            <label for="wst_<?php echo $wst; ?>"><?php echo strtoupper($wst); ?></label>
                        </div>
                    <?php endforeach; ?>
                    <div class="form-group" style="margin-top:10px;">
                        <label>GPIO Pin</label>
                        <input type="text" class="form-control" name="WX_TEMP_GPIO" value="<?php echo htmlspecialchars($config['WX_TEMP_GPIO'] ?? '3'); ?>" style="width:100px;">
                    </div>
                </div>

                <!-- Tempest options -->
                <div id="wx-opt-wf_tempest_local" class="wx-opt" style="display:none; margin-top:15px;">
                    <div class="form-group">
                        <label>Station Serial</label>
                        <input type="text" class="form-control" name="WXSTATION" value="<?php echo htmlspecialchars($config['WXSTATION'] ?? ''); ?>" placeholder="ST-00012345">
                    </div>
                </div>

                <!-- Hidden fields for unshown inputs -->
                <input type="hidden" name="WX_TEMPER_DEVICE" value="<?php echo htmlspecialchars($config['WX_TEMPER_DEVICE'] ?? ''); ?>">

                <div class="test-panel" style="margin-top:20px;">
                    <h4><i class="fa fa-bolt"></i> Test Weather Source</h4>
                    <p class="help-text">Save your settings first, then test to verify weather data is arriving.</p>
                    <button type="button" class="btn btn-test" onclick="testSensor('weather')"><i class="fa fa-play"></i> Test Now</button>
                    <div class="test-result waiting" id="test-result">Waiting for test...</div>
                </div>

                <div class="wizard-buttons">
                    <a href="setup-wizard.php?step=4" class="btn btn-back"><i class="fa fa-arrow-left"></i> Back</a>
                    <div>
                        <button type="submit" name="wizard_action" value="skip" class="btn btn-skip"><i class="fa fa-forward"></i> Skip</button>
                        <button type="submit" class="btn btn-primary"><i class="fa fa-arrow-right"></i> Next: Camera</button>
                    </div>
                </div>
            </form>

            <script>
            function showWxOptions() {
                document.querySelectorAll('.wx-opt').forEach(function(el) { el.style.display = 'none'; });
                var sel = document.querySelector('input[name="WEATHER_LEVEL"]:checked');
                if (sel) {
                    var opt = document.getElementById('wx-opt-' + sel.value);
                    if (opt) opt.style.display = 'block';
                }
            }
            showWxOptions();
            </script>


<?php
// ==================== STEP 6: CAMERA & BEE COUNTER ====================
elseif ($step === 6): ?>
            <div class="step-title"><i class="fa fa-camera"></i> Camera &amp; Bee Counter</div>
            <div class="step-description">Set up a camera for hive monitoring and/or a bee counter to track flight activity.</div>

            <form method="POST" action="setup-wizard.php?step=6" id="step-form">
                <input type="hidden" name="wizard_step" value="6">
                <input type="hidden" name="wizard_action" value="next" id="wizard_action_field">
                <input type="hidden" name="test_sensor_type" value="" id="test_sensor_type_field">

                <div class="row">
                    <div class="col-md-6">
                        <div class="panel panel-default">
                            <div class="panel-heading"><strong><i class="fa fa-camera"></i> Camera</strong></div>
                            <div class="panel-body">
                                <div class="enable-toggle" style="margin-bottom:10px;">
                                    <label>Enable</label>
                                    <select name="ENABLE_HIVE_CAMERA" class="form-control" style="width:120px;" id="cam-enable">
                                        <option value="yes" <?php if (($config['ENABLE_HIVE_CAMERA'] ?? 'no') === 'yes') echo 'selected'; ?>>Yes</option>
                                        <option value="no" <?php if (($config['ENABLE_HIVE_CAMERA'] ?? 'no') !== 'yes') echo 'selected'; ?>>No</option>
                                    </select>
                                </div>
                                <div id="cam-config" style="<?php echo (($config['ENABLE_HIVE_CAMERA'] ?? 'no') !== 'yes') ? 'display:none;' : ''; ?>">
                                    <div class="sensor-option <?php echo (($config['CAMERATYPE'] ?? '') === 'PI') ? 'selected' : ''; ?>">
                                        <input type="radio" name="CAMERATYPE" value="PI" id="cam_pi" <?php echo (($config['CAMERATYPE'] ?? '') === 'PI') ? 'checked' : ''; ?>>
                                        <label for="cam_pi">Pi Camera</label>
                                    </div>
                                    <div class="sensor-option <?php echo (($config['CAMERATYPE'] ?? '') === 'USB') ? 'selected' : ''; ?>">
                                        <input type="radio" name="CAMERATYPE" value="USB" id="cam_usb" <?php echo (($config['CAMERATYPE'] ?? '') === 'USB') ? 'checked' : ''; ?>>
                                        <label for="cam_usb">USB Camera</label>
                                    </div>

                                    <div class="test-panel" style="margin-top:10px;">
                                        <button type="button" class="btn btn-test btn-sm" onclick="testSensor('camera')"><i class="fa fa-play"></i> Test Camera</button>
                                        <div class="test-result waiting" id="test-result-cam" style="min-height:30px; margin-top:5px;">Waiting...</div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                    <div class="col-md-6">
                        <div class="panel panel-default">
                            <div class="panel-heading"><strong><i class="fa fa-bug"></i> Bee Counter</strong></div>
                            <div class="panel-body">
                                <div class="enable-toggle" style="margin-bottom:10px;">
                                    <label>Enable</label>
                                    <select name="ENABLE_BEECOUNTER" class="form-control" style="width:120px;" id="cnt-enable">
                                        <option value="yes" <?php if (($config['ENABLE_BEECOUNTER'] ?? 'no') === 'yes') echo 'selected'; ?>>Yes</option>
                                        <option value="no" <?php if (($config['ENABLE_BEECOUNTER'] ?? 'no') !== 'yes') echo 'selected'; ?>>No</option>
                                    </select>
                                </div>
                                <div id="cnt-config" style="<?php echo (($config['ENABLE_BEECOUNTER'] ?? 'no') !== 'yes') ? 'display:none;' : ''; ?>">
                                    <div class="sensor-option <?php echo (($config['COUNTERTYPE'] ?? '') === 'PICAMERA') ? 'selected' : ''; ?>">
                                        <input type="radio" name="COUNTERTYPE" value="PICAMERA" id="cnt_picam" <?php echo (($config['COUNTERTYPE'] ?? '') === 'PICAMERA') ? 'checked' : ''; ?>>
                                        <label for="cnt_picam">Pi Camera Counter</label>
                                    </div>
                                    <div class="sensor-option <?php echo (($config['COUNTERTYPE'] ?? '') === 'GATES') ? 'selected' : ''; ?>">
                                        <input type="radio" name="COUNTERTYPE" value="GATES" id="cnt_gates" <?php echo (($config['COUNTERTYPE'] ?? '') === 'GATES') ? 'checked' : ''; ?>>
                                        <label for="cnt_gates">Gate Sensor</label>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>

                <div class="wizard-buttons">
                    <a href="setup-wizard.php?step=5" class="btn btn-back"><i class="fa fa-arrow-left"></i> Back</a>
                    <div>
                        <button type="submit" name="wizard_action" value="skip" class="btn btn-skip"><i class="fa fa-forward"></i> Skip</button>
                        <button type="submit" class="btn btn-primary"><i class="fa fa-arrow-right"></i> Next: Air Quality</button>
                    </div>
                </div>
            </form>

            <script>
            document.getElementById('cam-enable').addEventListener('change', function() {
                document.getElementById('cam-config').style.display = (this.value === 'yes') ? 'block' : 'none';
            });
            document.getElementById('cnt-enable').addEventListener('change', function() {
                document.getElementById('cnt-config').style.display = (this.value === 'yes') ? 'block' : 'none';
            });
            </script>


<?php
// ==================== STEP 7: AIR QUALITY ====================
elseif ($step === 7): ?>
            <div class="step-title"><i class="fa fa-leaf"></i> Air Quality</div>
            <div class="step-description">Monitor air quality near your hives. Poor air quality can affect foraging behavior.</div>

            <form method="POST" action="setup-wizard.php?step=7" id="step-form">
                <input type="hidden" name="wizard_step" value="7">
                <input type="hidden" name="wizard_action" value="next" id="wizard_action_field">
                <input type="hidden" name="test_sensor_type" value="" id="test_sensor_type_field">

                <div class="enable-toggle">
                    <label><i class="fa fa-leaf"></i> Enable Air Quality Monitoring</label>
                    <select name="ENABLE_AIR" id="sensor-enable" class="form-control" style="width:120px;">
                        <option value="yes" <?php if (($config['ENABLE_AIR'] ?? 'no') === 'yes') echo 'selected'; ?>>Enabled</option>
                        <option value="no" <?php if (($config['ENABLE_AIR'] ?? 'no') !== 'yes') echo 'selected'; ?>>Disabled</option>
                    </select>
                </div>

                <div id="sensor-config" style="<?php echo (($config['ENABLE_AIR'] ?? 'no') !== 'yes') ? 'display:none;' : ''; ?>">
                    <?php $airType = $config['AIR_TYPE'] ?? ''; ?>
                    <label>Select Air Quality Source:</label>
                    <div class="sensor-option <?php echo ($airType === 'purpleapi' || $airType === 'purple') ? 'selected' : ''; ?>">
                        <input type="radio" name="AIR_TYPE" value="purpleapi" id="air_purpleapi"
                            onchange="updateAirOptions()"
                            <?php echo ($airType === 'purpleapi' || $airType === 'purple') ? 'checked' : ''; ?>>
                        <label for="air_purpleapi"><strong>PurpleAir - API</strong> &mdash; Read from PurpleAir cloud API (requires API key)</label>
                    </div>
                    <div class="sensor-option <?php echo ($airType === 'purplelocal') ? 'selected' : ''; ?>">
                        <input type="radio" name="AIR_TYPE" value="purplelocal" id="air_purplelocal"
                            onchange="updateAirOptions()"
                            <?php echo ($airType === 'purplelocal') ? 'checked' : ''; ?>>
                        <label for="air_purplelocal"><strong>PurpleAir - Local</strong> &mdash; Read directly from a PurpleAir sensor on your network</label>
                    </div>

                    <div id="air-config-api" style="<?php echo ($airType === 'purpleapi' || $airType === 'purple') ? '' : 'display:none;'; ?>">
                        <div class="form-group" style="margin-top:15px;">
                            <label>Station ID</label>
                            <input type="text" class="form-control" name="AIR_ID" id="air-id-api" value="<?php echo htmlspecialchars($config['AIR_ID'] ?? ''); ?>" placeholder="e.g. 7634">
                            <p class="help-text">Go to <a href="https://www.purpleair.com/map" target="_blank">purpleair.com/map</a> to find a sensor near your hives and get its ID.</p>
                        </div>
                        <div class="form-group">
                            <label>API Read Key</label>
                            <input type="text" class="form-control" name="AIR_API" value="<?php echo htmlspecialchars($config['AIR_API'] ?? ''); ?>" placeholder="e.g. XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX">
                            <p class="help-text">Get your API key from <a href="https://develop.purpleair.com/keys" target="_blank">develop.purpleair.com/keys</a></p>
                        </div>
                    </div>

                    <div id="air-config-local" style="<?php echo ($airType === 'purplelocal') ? '' : 'display:none;'; ?>">
                        <div class="form-group" style="margin-top:15px;">
                            <label>Station ID</label>
                            <input type="text" class="form-control" name="AIR_ID" id="air-id-local" value="<?php echo htmlspecialchars($config['AIR_ID'] ?? ''); ?>" placeholder="e.g. 7634">
                        </div>
                        <div class="form-group">
                            <label>Local Sensor URL</label>
                            <input type="text" class="form-control" name="AIR_LOCAL_URL" value="<?php echo htmlspecialchars($config['AIR_LOCAL_URL'] ?? ''); ?>" placeholder="http://192.168.1.x/json">
                            <p class="help-text">The URL of your PurpleAir sensor on your local network. Must include the <strong>/json</strong> path (e.g. http://192.168.1.100/json).</p>
                        </div>
                    </div>

                    <div class="test-panel">
                        <h4><i class="fa fa-bolt"></i> Test Air Quality</h4>
                        <p class="help-text">Save settings first, then test to verify data is arriving.</p>
                        <button type="button" class="btn btn-test" onclick="testSensor('airquality')"><i class="fa fa-play"></i> Test Now</button>
                        <div class="test-result waiting" id="test-result">Waiting for test...</div>
                    </div>
                </div>

                <hr style="margin: 25px 0; border-color: #e7e7e7;">

                <div class="enable-toggle">
                    <label><i class="fa fa-institution"></i> EPA AirNow (O3, NO2, PM2.5, PM10)</label>
                    <select name="ENABLE_AIRNOW" id="airnow-enable" class="form-control" style="width:120px;">
                        <option value="yes" <?php if (($config['ENABLE_AIRNOW'] ?? 'no') === 'yes') echo 'selected'; ?>>Enabled</option>
                        <option value="no" <?php if (($config['ENABLE_AIRNOW'] ?? 'no') !== 'yes') echo 'selected'; ?>>Disabled</option>
                    </select>
                </div>

                <div id="airnow-config" style="<?php echo (($config['ENABLE_AIRNOW'] ?? 'no') !== 'yes') ? 'display:none;' : ''; ?>">
                    <p class="help-text" style="margin-bottom: 15px;">
                        EPA AirNow provides hourly O3, NO2, PM2.5, and PM10 AQI data from the nearest government air quality monitor.
                        <strong>No local sensor needed</strong> &mdash; works as a standalone source or as a backup if you don't have PurpleAir.
                        Data is free; you just need an API key.
                    </p>

                    <div class="form-group">
                        <label>AirNow API Key</label>
                        <input type="text" class="form-control" name="KEY_AIRNOW" value="<?php echo htmlspecialchars($config['KEY_AIRNOW'] ?? ''); ?>" placeholder="e.g., XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX">
                        <p class="help-text">Get a free API key at <a href="https://docs.airnowapi.org/account/request/" target="_blank">docs.airnowapi.org</a>. Uses your hive's latitude/longitude from Step 1 to find the nearest monitor.</p>
                    </div>

                    <div class="form-group">
                        <label>Search Distance (miles)</label>
                        <input type="text" class="form-control" name="AIRNOW_DISTANCE" value="<?php echo htmlspecialchars($config['AIRNOW_DISTANCE'] ?? '25'); ?>" placeholder="25" style="width: 120px;">
                        <p class="help-text">How far to search for an EPA monitor from your hive location. Default: 25 miles.</p>
                    </div>

                    <div class="test-panel">
                        <h4><i class="fa fa-bolt"></i> Test EPA AirNow</h4>
                        <p class="help-text">Save settings first, then test to verify EPA data is arriving.</p>
                        <button type="button" class="btn btn-test" onclick="testSensor('airnow')"><i class="fa fa-play"></i> Test Now</button>
                        <div class="test-result waiting" id="test-result-airnow">Waiting for test...</div>
                    </div>
                </div>

                <div class="wizard-buttons">
                    <a href="setup-wizard.php?step=6" class="btn btn-back"><i class="fa fa-arrow-left"></i> Back</a>
                    <div>
                        <button type="submit" name="wizard_action" value="skip" class="btn btn-skip"><i class="fa fa-forward"></i> Skip</button>
                        <button type="submit" class="btn btn-primary"><i class="fa fa-arrow-right"></i> Next: Review</button>
                    </div>
                </div>
            </form>


<?php
// ==================== STEP 8: REVIEW ====================
elseif ($step === 8): ?>
            <div class="step-title"><i class="fa fa-check-circle"></i> Setup Review</div>
            <div class="step-description">Here's a summary of your configuration. Click any section to go back and make changes.</div>

            <?php
            $sections = [
                1 => ['Basic Info', 'fa-home', [
                    'Hive Name' => $config['HIVENAME'] ?? 'Not set',
                    'Location' => ($config['CITY'] ?? '') . ', ' . ($config['STATE'] ?? ''),
                    'Coordinates' => (!empty($config['LATITUDE']) && !empty($config['LONGITUDE']))
                        ? $config['LATITUDE'] . ', ' . $config['LONGITUDE']
                        : 'Not set',
                    'Timezone' => $config['TIMEZONE'] ?? 'Not set',
                ]],
                2 => ['Temp/Humidity', 'fa-fire', [
                    'Status' => $config['ENABLE_HIVE_TEMP_CHK'] ?? 'no',
                    'Sensor Type' => (($config['ENABLE_HIVE_TEMP_CHK'] ?? 'no') === 'yes') ? strtoupper($config['TEMPTYPE'] ?? '') : 'N/A',
                ]],
                3 => ['Weight Scale', 'fa-dashboard', [
                    'Status' => $config['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no',
                    'Scale Type' => (($config['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no') === 'yes') ? strtoupper($config['SCALETYPE'] ?? '') : 'N/A',
                    'Calibration' => (($config['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no') === 'yes')
                        ? 'Slope: ' . ($config['HIVE_WEIGHT_SLOPE'] ?? '1') . ', Intercept: ' . ($config['HIVE_WEIGHT_INTERCEPT'] ?? '0')
                        : 'N/A',
                    'Drift Compensation' => (($config['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no') === 'yes')
                        ? (($config['WEIGHT_COMPENSATION_ENABLED'] ?? 'no') === 'yes' ? 'Enabled (auto)' : 'Disabled')
                        : 'N/A',
                ]],
                4 => ['Light Sensor', 'fa-sun-o', [
                    'Status' => $config['ENABLE_LUX'] ?? 'no',
                    'Source' => (($config['ENABLE_LUX'] ?? 'no') === 'yes') ? strtoupper($config['LUX_SOURCE'] ?? '') : 'N/A',
                ]],
                5 => ['Weather Source', 'fa-cloud', (function() use ($config) {
                    $wx_names = [
                        'openmeteo' => 'Open-Meteo',
                        'openweathermap' => 'OpenWeatherMap',
                        'nws' => 'NWS (weather.gov)',
                        'weatherapi' => 'WeatherAPI.com',
                        'visualcrossing' => 'Visual Crossing',
                        'ambientwx' => 'AmbientWeather.net',
                        'hive' => 'WX Underground',
                        'localws' => 'Local Weather Station',
                        'localsensors' => 'Local Sensors',
                        'wf_tempest_local' => 'WeatherFlow Tempest',
                    ];
                    $level = $config['WEATHER_LEVEL'] ?? '';
                    $name = $wx_names[$level] ?? strtoupper($level ?: 'Not set');
                    $details = ['Source' => $name];
                    if (in_array($level, ['hive', 'ambientwx', 'wf_tempest_local'])) {
                        $details['Station'] = !empty($config['WXSTATION']) ? $config['WXSTATION'] : 'N/A';
                    }
                    if (in_array($level, ['openweathermap', 'weatherapi', 'visualcrossing'])) {
                        $details['API Key'] = !empty($config['KEY']) ? '****' . substr($config['KEY'], -4) : 'Not set';
                    }
                    return $details;
                })()],
                6 => ['Camera & Counter', 'fa-camera', [
                    'Camera' => ($config['ENABLE_HIVE_CAMERA'] ?? 'no') . ((($config['ENABLE_HIVE_CAMERA'] ?? 'no') === 'yes') ? ' (' . ($config['CAMERATYPE'] ?? '') . ')' : ''),
                    'Bee Counter' => ($config['ENABLE_BEECOUNTER'] ?? 'no') . ((($config['ENABLE_BEECOUNTER'] ?? 'no') === 'yes') ? ' (' . ($config['COUNTERTYPE'] ?? '') . ')' : ''),
                ]],
                7 => ['Air Quality', 'fa-leaf', array_merge(
                    ['PurpleAir' => $config['ENABLE_AIR'] ?? 'no'],
                    (($config['ENABLE_AIR'] ?? 'no') === 'yes') ? [
                        'PA Source' => (($config['AIR_TYPE'] ?? '') === 'purplelocal') ? 'PurpleAir (Local)' : 'PurpleAir (API)',
                        'Station ID' => $config['AIR_ID'] ?? 'Not set',
                    ] : [],
                    ['EPA AirNow' => $config['ENABLE_AIRNOW'] ?? 'no'],
                    (($config['ENABLE_AIRNOW'] ?? 'no') === 'yes') ? [
                        'Search Distance' => ($config['AIRNOW_DISTANCE'] ?? '25') . ' miles',
                        'API Key' => !empty($config['KEY_AIRNOW']) ? '****' . substr($config['KEY_AIRNOW'], -4) : 'Not set',
                    ] : []
                )],
            ];
            ?>

            <?php foreach ($sections as $snum => $sec): ?>
                <div class="review-section">
                    <h4>
                        <a href="setup-wizard.php?step=<?php echo $snum; ?>" style="color:#e6b800; text-decoration:none;">
                            <i class="fa <?php echo $sec[1]; ?>"></i> <?php echo $sec[0]; ?>
                            <i class="fa fa-pencil" style="font-size:12px; margin-left:5px;"></i>
                        </a>
                    </h4>
                    <table class="table review-table">
                        <?php foreach ($sec[2] as $label => $value): ?>
                            <tr>
                                <td class="label-col"><?php echo $label; ?></td>
                                <td class="value-col">
                                    <?php if ($value === 'yes'): ?>
                                        <span class="label badge-enabled">Enabled</span>
                                    <?php elseif ($value === 'no'): ?>
                                        <span class="label badge-disabled">Disabled</span>
                                    <?php else: ?>
                                        <?php echo htmlspecialchars($value); ?>
                                    <?php endif; ?>
                                </td>
                            </tr>
                        <?php endforeach; ?>
                    </table>
                </div>
            <?php endforeach; ?>

            <div class="wizard-buttons">
                <a href="setup-wizard.php?step=7" class="btn btn-back"><i class="fa fa-arrow-left"></i> Back</a>
                <div>
                    <a href="/admin/instrumentconfig.php" class="btn btn-skip"><i class="fa fa-cog"></i> Advanced Settings</a>
                    <a href="/pages/index.php" class="btn btn-primary btn-lg"><i class="fa fa-dashboard"></i> Go to Dashboard</a>
                </div>
            </div>

<?php endif; ?>

        </div>
    </div>

    <script src="../bower_components/jquery/dist/jquery.min.js"></script>
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>
    <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js" integrity="sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo=" crossorigin=""></script>

    <script>
    // Location map for Step 1
    var locationMap = null;
    var locationMarker = null;
    var hiveIcon = L.divIcon({
        className: '',
        html: '<div style="width:28px;height:28px;background:#e6b800;border:3px solid #333;border-radius:50%;box-shadow:0 2px 6px rgba(0,0,0,0.4);cursor:grab;"></div>',
        iconSize: [28, 28],
        iconAnchor: [14, 14]
    });

    function initLocationMap() {
        var mapEl = document.getElementById('location-map');
        if (!mapEl) return;

        var lat = parseFloat(document.getElementById('LATITUDE').value) || 39.8283;
        var lng = parseFloat(document.getElementById('LONGITUDE').value) || -98.5795;
        var hasCoords = document.getElementById('LATITUDE').value !== '' && document.getElementById('LONGITUDE').value !== '';
        var zoom = hasCoords ? 13 : 4;

        locationMap = L.map('location-map').setView([lat, lng], zoom);
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            maxZoom: 19,
            attribution: '&copy; OpenStreetMap contributors'
        }).addTo(locationMap);

        if (hasCoords) {
            locationMarker = L.marker([lat, lng], { draggable: true, icon: hiveIcon }).addTo(locationMap);
            locationMarker.on('dragend', function(e) {
                var pos = e.target.getLatLng();
                document.getElementById('LATITUDE').value = pos.lat.toFixed(6);
                document.getElementById('LONGITUDE').value = pos.lng.toFixed(6);
                document.getElementById('geocode-status').innerHTML = '<i class="fa fa-arrows"></i> Marker moved to ' + pos.lat.toFixed(6) + ', ' + pos.lng.toFixed(6);
            });
        }
    }

    function geocodeLocation() {
        var city = document.querySelector('input[name="CITY"]').value.trim();
        var state = document.querySelector('input[name="STATE"]').value.trim();
        if (!city || !state) {
            document.getElementById('geocode-status').innerHTML = '<span style="color:#d9534f;">Please enter a city and state first.</span>';
            return;
        }

        var btn = document.getElementById('btn-geocode');
        btn.disabled = true;
        btn.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Looking up...';
        document.getElementById('geocode-status').innerHTML = '';

        $.ajax({
            url: 'geocode.php?city=' + encodeURIComponent(city) + '&state=' + encodeURIComponent(state),
            timeout: 15000,
            dataType: 'json',
            success: function(data) {
                btn.disabled = false;
                btn.innerHTML = '<i class="fa fa-search"></i> Look Up';
                if (data && data.length > 0) {
                    var lat = parseFloat(data[0].lat);
                    var lng = parseFloat(data[0].lon);
                    document.getElementById('LATITUDE').value = lat.toFixed(6);
                    document.getElementById('LONGITUDE').value = lng.toFixed(6);
                    document.getElementById('geocode-status').innerHTML = '<span style="color:#5cb85c;"><i class="fa fa-check"></i> Found: ' + escHtml(data[0].display_name) + '</span>';
                    placeMarker(lat, lng);
                } else {
                    document.getElementById('geocode-status').innerHTML = '<span style="color:#d9534f;"><i class="fa fa-exclamation-triangle"></i> Location not found. Try a different city name or enter coordinates manually.</span>';
                }
            },
            error: function() {
                btn.disabled = false;
                btn.innerHTML = '<i class="fa fa-search"></i> Look Up';
                document.getElementById('geocode-status').innerHTML = '<span style="color:#d9534f;"><i class="fa fa-exclamation-triangle"></i> Geocoding service unavailable. Enter coordinates manually.</span>';
            }
        });
    }

    function placeMarker(lat, lng) {
        if (!locationMap) return;
        locationMap.setView([lat, lng], 13);
        if (locationMarker) {
            locationMarker.setLatLng([lat, lng]);
        } else {
            locationMarker = L.marker([lat, lng], { draggable: true, icon: hiveIcon }).addTo(locationMap);
            locationMarker.on('dragend', function(e) {
                var pos = e.target.getLatLng();
                document.getElementById('LATITUDE').value = pos.lat.toFixed(6);
                document.getElementById('LONGITUDE').value = pos.lng.toFixed(6);
                document.getElementById('geocode-status').innerHTML = '<i class="fa fa-arrows"></i> Marker moved to ' + pos.lat.toFixed(6) + ', ' + pos.lng.toFixed(6);
            });
        }
    }

    // Allow manual lat/lon entry to update the map
    function updateMapFromInputs() {
        var lat = parseFloat(document.getElementById('LATITUDE') ? document.getElementById('LATITUDE').value : '');
        var lng = parseFloat(document.getElementById('LONGITUDE') ? document.getElementById('LONGITUDE').value : '');
        if (!isNaN(lat) && !isNaN(lng) && lat >= -90 && lat <= 90 && lng >= -180 && lng <= 180) {
            placeMarker(lat, lng);
        }
    }

    $(document).ready(function() {
        initLocationMap();
        $('#LATITUDE, #LONGITUDE').on('change', updateMapFromInputs);
    });
    </script>

    <script>
    // Enable/disable toggle for sensor config sections
    var enableSel = document.getElementById('sensor-enable');
    if (enableSel) {
        enableSel.addEventListener('change', function() {
            var cfg = document.getElementById('sensor-config');
            if (cfg) cfg.style.display = (this.value === 'yes') ? 'block' : 'none';
        });
    }

    // Highlight selected radio options
    document.querySelectorAll('.sensor-option input[type="radio"]').forEach(function(radio) {
        radio.addEventListener('change', function() {
            var group = this.getAttribute('name');
            document.querySelectorAll('input[name="' + group + '"]').forEach(function(r) {
                r.closest('.sensor-option').classList.remove('selected');
            });
            this.closest('.sensor-option').classList.add('selected');
        });
    });

    // Show/hide temp-specific options
    function showTempOptions() {
        document.querySelectorAll('.temp-opt').forEach(function(el) { el.style.display = 'none'; });
        var sel = document.querySelector('input[name="TEMPTYPE"]:checked');
        if (!sel) return;
        var v = sel.value;
        if (v === 'temperhum') document.getElementById('opt-temperhum').style.display = 'block';
        else if (v === 'temper') document.getElementById('opt-temper').style.display = 'block';
        else if (v === 'dht22' || v === 'dht21') document.getElementById('opt-gpio').style.display = 'block';
        else if (v === 'broodminder') document.getElementById('opt-broodminder').style.display = 'block';
    }
    if (document.querySelector('input[name="TEMPTYPE"]')) {
        document.querySelectorAll('input[name="TEMPTYPE"]').forEach(function(r) {
            r.addEventListener('change', showTempOptions);
        });
        showTempOptions();
    }

    // Save form, then test sensor — submits the form with action=test, page reloads and auto-fires AJAX
    function testSensor(type) {
        var form = document.getElementById('step-form');
        if (!form) return;
        document.getElementById('wizard_action_field').value = 'test';
        document.getElementById('test_sensor_type_field').value = type;
        form.submit();
    }

    // Run AJAX sensor test (called after page reload with autotest param)
    function runSensorTest(type) {
        var resultId = (type === 'camera') ? 'test-result-cam' : (type === 'airnow') ? 'test-result-airnow' : 'test-result';
        var el = document.getElementById(resultId);
        if (!el) return;
        el.className = 'test-result waiting';
        el.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Settings saved. Reading sensor... this may take up to a minute';

        var ajaxTimeout = (type === 'hiveweight' || type === 'airnow') ? 90000 : 30000;
        $.ajax({
            url: 'livevalue.php?sensor=' + type,
            timeout: ajaxTimeout,
            success: function(data) {
                if (data && data.trim().length > 0) {
                    el.className = 'test-result success';
                    el.innerHTML = data.replace(/\n/g, '<br>');
                } else {
                    el.className = 'test-result error';
                    el.innerHTML = 'No data returned. Check sensor connection and configuration.';
                }
            },
            error: function(xhr, status) {
                el.className = 'test-result error';
                if (status === 'timeout') {
                    el.innerHTML = 'Timeout: sensor read took too long. Check wiring and configuration.';
                } else {
                    el.innerHTML = 'Error connecting to sensor test endpoint.';
                }
            }
        });
    }

    // Show/hide calibration wizard vs CPW notice based on scale type
    function updateScaleOptions() {
        var sel = document.querySelector('input[name="SCALETYPE"]:checked');
        if (!sel) return;
        var v = sel.value;
        var cpwNotice = document.getElementById('cpw-notice');
        var calLauncher = document.getElementById('cal-wizard-launcher');
        var calWizard = document.getElementById('cal-wizard');
        var fieldcalWizard = document.getElementById('fieldcal-wizard');
        if (!cpwNotice) return;
        if (v === 'cpw200plus') {
            cpwNotice.style.display = 'block';
            if (calLauncher) calLauncher.style.display = 'none';
            if (calWizard) calWizard.style.display = 'none';
            if (fieldcalWizard) fieldcalWizard.style.display = 'none';
        } else {
            cpwNotice.style.display = 'none';
            if (calLauncher) calLauncher.style.display = 'block';
        }
    }
    if (document.querySelector('input[name="SCALETYPE"]')) {
        document.querySelectorAll('input[name="SCALETYPE"]').forEach(function(r) {
            r.addEventListener('change', updateScaleOptions);
        });
        updateScaleOptions();
    }

    function updateAirOptions() {
        var sel = document.querySelector('input[name="AIR_TYPE"]:checked');
        var apiDiv = document.getElementById('air-config-api');
        var localDiv = document.getElementById('air-config-local');
        if (!apiDiv || !localDiv) return;
        if (sel && sel.value === 'purplelocal') {
            apiDiv.style.display = 'none';
            localDiv.style.display = 'block';
            apiDiv.querySelectorAll('input[name]').forEach(function(i) { i.disabled = true; });
            localDiv.querySelectorAll('input[name]').forEach(function(i) { i.disabled = false; });
        } else {
            apiDiv.style.display = 'block';
            localDiv.style.display = 'none';
            apiDiv.querySelectorAll('input[name]').forEach(function(i) { i.disabled = false; });
            localDiv.querySelectorAll('input[name]').forEach(function(i) { i.disabled = true; });
        }
        document.querySelectorAll('.sensor-option').forEach(function(opt) {
            var radio = opt.querySelector('input[type="radio"]');
            if (radio) opt.className = 'sensor-option' + (radio.checked ? ' selected' : '');
        });
    }
    if (document.querySelector('input[name="AIR_TYPE"]')) {
        updateAirOptions();
    }

    // AirNow enable/disable toggle
    var airnowSel = document.getElementById('airnow-enable');
    if (airnowSel) {
        airnowSel.addEventListener('change', function() {
            var cfg = document.getElementById('airnow-config');
            if (cfg) cfg.style.display = (this.value === 'yes') ? 'block' : 'none';
        });
    }

    function escHtml(s) {
        var d = document.createElement('div');
        d.appendChild(document.createTextNode(s));
        return d.innerHTML;
    }

    // Calibration Sub-Wizard
    var CalWiz = {
        rawZero: null,
        rawLoaded: null,
        slope: null,
        intercept: null,

        getScaleType: function() {
            var sel = document.querySelector('input[name="SCALETYPE"]:checked');
            return sel ? sel.value : '';
        },

        showStep: function(n) {
            document.querySelectorAll('.cal-step').forEach(function(el) { el.style.display = 'none'; });
            var step = document.getElementById('cal-step-' + n);
            if (step) step.style.display = 'block';
            // Update progress indicators
            for (var i = 1; i <= 3; i++) {
                var prog = document.getElementById('cal-prog-' + i);
                if (!prog) continue;
                if (i < n) { prog.style.background = '#5cb85c'; prog.style.color = 'white'; }
                else if (i === n) { prog.style.background = '#e6b800'; prog.style.color = 'white'; }
                else { prog.style.background = '#eee'; prog.style.color = '#999'; }
            }
        },

        start: function() {
            this.rawZero = null;
            this.rawLoaded = null;
            this.slope = null;
            this.intercept = null;
            document.getElementById('cal-wizard').style.display = 'block';
            document.getElementById('cal-zero-result').className = 'test-result waiting';
            document.getElementById('cal-zero-result').innerHTML = 'Ready when you are...';
            document.getElementById('cal-save-result').style.display = 'none';
            document.getElementById('cal-verify-result').style.display = 'none';
            this.showStep(1);
        },

        cancel: function() {
            document.getElementById('cal-wizard').style.display = 'none';
        },

        setButtonLoading: function(btnId, loading) {
            var btn = document.getElementById(btnId);
            if (!btn) return;
            btn.disabled = loading;
            if (loading) {
                btn.setAttribute('data-orig', btn.innerHTML);
                btn.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Reading...';
            } else {
                var orig = btn.getAttribute('data-orig');
                if (orig) btn.innerHTML = orig;
            }
        },

        takeZeroReading: function() {
            var scaleType = this.getScaleType();
            if (!scaleType) { alert('Please select a scale type first.'); return; }
            var self = this;
            var el = document.getElementById('cal-zero-result');
            el.className = 'test-result waiting';
            el.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Taking 5 samples for zero reading &mdash; this may take up to a minute';
            this.setButtonLoading('btn-zero', true);

            $.ajax({
                url: 'calibrate_raw_reading.php?scaletype=' + scaleType + '&samples=5',
                timeout: 120000,
                dataType: 'json',
                success: function(data) {
                    self.setButtonLoading('btn-zero', false);
                    if (data.success) {
                        self.rawZero = parseFloat(data.raw_value);
                        el.className = 'test-result success';
                        el.innerHTML = 'Zero reading: <strong>' + escHtml(String(data.raw_value)) + '</strong> (' + data.sample_count + '/' + data.samples_requested + ' samples)';
                        setTimeout(function() { self.showStep(2); }, 800);
                    } else {
                        el.className = 'test-result error';
                        el.textContent = data.error || 'Failed to get reading';
                    }
                },
                error: function(xhr, status) {
                    self.setButtonLoading('btn-zero', false);
                    el.className = 'test-result error';
                    el.innerHTML = (status === 'timeout')
                        ? 'Timeout: sensor did not respond within 60 seconds. Check wiring.'
                        : 'Error communicating with sensor endpoint.';
                }
            });
        },

        takeLoadedReading: function() {
            var scaleType = this.getScaleType();
            var knownWeight = parseFloat(document.getElementById('cal-known-weight').value);
            if (!knownWeight || knownWeight <= 0) {
                alert('Please enter a valid known weight greater than zero.');
                return;
            }
            var self = this;
            var el = document.getElementById('cal-loaded-result');
            el.className = 'test-result waiting';
            el.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Taking 5 samples for loaded reading &mdash; this may take up to a minute';
            this.setButtonLoading('btn-loaded', true);

            $.ajax({
                url: 'calibrate_raw_reading.php?scaletype=' + scaleType + '&samples=5',
                timeout: 120000,
                dataType: 'json',
                success: function(data) {
                    self.setButtonLoading('btn-loaded', false);
                    if (data.success) {
                        self.rawLoaded = parseFloat(data.raw_value);
                        el.className = 'test-result success';
                        el.innerHTML = 'Loaded reading: <strong>' + escHtml(String(data.raw_value)) + '</strong> (' + data.sample_count + '/' + data.samples_requested + ' samples)';

                        var result = self.calculate(self.rawZero, self.rawLoaded, knownWeight, scaleType);
                        if (result.error) {
                            el.className = 'test-result error';
                            el.textContent = result.error;
                            return;
                        }

                        self.slope = result.slope;
                        self.intercept = result.intercept;

                        document.getElementById('cal-result-intercept').textContent = self.intercept;
                        document.getElementById('cal-result-slope').textContent = self.slope;
                        document.getElementById('cal-result-known').textContent = knownWeight + ' lbs';

                        setTimeout(function() { self.showStep(3); }, 800);
                    } else {
                        el.className = 'test-result error';
                        el.textContent = data.error || 'Failed to get reading';
                    }
                },
                error: function(xhr, status) {
                    self.setButtonLoading('btn-loaded', false);
                    el.className = 'test-result error';
                    el.innerHTML = (status === 'timeout')
                        ? 'Timeout: sensor did not respond within 60 seconds. Check wiring.'
                        : 'Error communicating with sensor endpoint.';
                }
            });
        },

        calculate: function(rawZero, rawLoaded, knownWeight, scaleType) {
            var diff = rawLoaded - rawZero;
            if (Math.abs(diff) < 1) {
                return { error: 'Zero and loaded readings are too similar. The sensor may not be responding to weight changes. Check wiring and connections.' };
            }
            var slope, intercept;
            intercept = rawZero;
            if (scaleType === 'phidget1046') {
                // Phidget: weight = (raw - zero) * multiplier
                slope = knownWeight / diff;
            } else {
                // HX711: weight = (raw - zero) / slope
                slope = diff / knownWeight;
            }
            return {
                slope: parseFloat(slope.toFixed(6)),
                intercept: parseFloat(intercept.toFixed(2))
            };
        },

        verify: function() {
            var el = document.getElementById('cal-verify-result');
            el.style.display = 'block';
            el.className = 'test-result waiting';
            el.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Saving and verifying... leave the weight on the scale';
            this.setButtonLoading('btn-verify', true);
            var self = this;

            // Save first, then read through the calibrated path
            $.ajax({
                url: 'calibrate_save.php',
                method: 'POST',
                data: { slope: self.slope, intercept: self.intercept, scaletype: self.getScaleType() },
                dataType: 'json',
                timeout: 15000,
                success: function() {
                    // Update hidden form fields so wizard form submission has current values
                    document.getElementById('form-slope').value = self.slope;
                    document.getElementById('form-intercept').value = self.intercept;
                    var mi = document.getElementById('manual-intercept');
                    var ms = document.getElementById('manual-slope');
                    if (mi) mi.value = self.intercept;
                    if (ms) ms.value = self.slope;

                    // Now read through the normal calibrated endpoint
                    $.ajax({
                        url: 'livevalue.php?sensor=hiveweight',
                        timeout: 90000,
                        success: function(data) {
                            self.setButtonLoading('btn-verify', false);
                            if (data && data.trim().length > 0) {
                                el.className = 'test-result success';
                                el.innerHTML = '<strong>Verification reading:</strong><br>' + escHtml(data).replace(/\n/g, '<br>') + '<br><br><em>Compare this to the weight you placed on the scale.</em>';
                                document.getElementById('cal-save-result').style.display = 'block';
                            } else {
                                el.className = 'test-result error';
                                el.innerHTML = 'No data returned. Check sensor.';
                            }
                        },
                        error: function() {
                            self.setButtonLoading('btn-verify', false);
                            el.className = 'test-result error';
                            el.innerHTML = 'Verification read failed. The calibration values were saved — try "Test Scale" below.';
                            document.getElementById('cal-save-result').style.display = 'block';
                        }
                    });
                },
                error: function() {
                    self.setButtonLoading('btn-verify', false);
                    el.className = 'test-result error';
                    el.innerHTML = 'Failed to save calibration for verification. Check database permissions.';
                }
            });
        },

        save: function() {
            var self = this;
            this.setButtonLoading('btn-save-cal', true);

            $.ajax({
                url: 'calibrate_save.php',
                method: 'POST',
                data: { slope: self.slope, intercept: self.intercept, scaletype: self.getScaleType() },
                dataType: 'json',
                timeout: 15000,
                success: function(data) {
                    self.setButtonLoading('btn-save-cal', false);
                    if (data.success) {
                        // Update the hidden form fields
                        document.getElementById('form-slope').value = self.slope;
                        document.getElementById('form-intercept').value = self.intercept;
                        // Update manual entry fields too
                        var mi = document.getElementById('manual-intercept');
                        var ms = document.getElementById('manual-slope');
                        if (mi) mi.value = self.intercept;
                        if (ms) ms.value = self.slope;
                        // Show success
                        document.getElementById('cal-save-result').style.display = 'block';
                    } else {
                        alert('Save failed: ' + (data.error || 'Unknown error'));
                    }
                },
                error: function() {
                    self.setButtonLoading('btn-save-cal', false);
                    alert('Failed to save calibration. Check database permissions.');
                }
            });
        }
    };

    // Field Recalibration (under load)
    var FieldCal = {
        rawBaseline: null,
        rawLoaded: null,
        newSlope: null,
        oldSlope: parseFloat('<?php echo $config['HIVE_WEIGHT_SLOPE'] ?? '1'; ?>'),
        currentIntercept: parseFloat('<?php echo $config['HIVE_WEIGHT_INTERCEPT'] ?? '0'; ?>'),

        getScaleType: function() {
            var sel = document.querySelector('input[name="SCALETYPE"]:checked');
            return sel ? sel.value : '';
        },

        showStep: function(n) {
            document.querySelectorAll('.fieldcal-step').forEach(function(el) { el.style.display = 'none'; });
            var step = document.getElementById('fieldcal-step-' + n);
            if (step) step.style.display = 'block';
            for (var i = 1; i <= 3; i++) {
                var prog = document.getElementById('fieldcal-prog-' + i);
                if (!prog) continue;
                if (i < n) { prog.style.background = '#5cb85c'; prog.style.color = 'white'; }
                else if (i === n) { prog.style.background = '#e6b800'; prog.style.color = 'white'; }
                else { prog.style.background = '#eee'; prog.style.color = '#999'; }
            }
        },

        start: function() {
            this.rawBaseline = null;
            this.rawLoaded = null;
            this.newSlope = null;
            this.oldSlope = parseFloat(document.getElementById('form-slope').value) || 1;
            this.currentIntercept = parseFloat(document.getElementById('form-intercept').value) || 0;
            document.getElementById('fieldcal-wizard').style.display = 'block';
            document.getElementById('fieldcal-baseline-result').className = 'test-result waiting';
            document.getElementById('fieldcal-baseline-result').innerHTML = 'Ready when you are...';
            document.getElementById('fieldcal-save-result').style.display = 'none';
            document.getElementById('fieldcal-warning').style.display = 'none';
            this.showStep(1);
        },

        cancel: function() {
            document.getElementById('fieldcal-wizard').style.display = 'none';
        },

        setButtonLoading: function(btnId, loading) {
            var btn = document.getElementById(btnId);
            if (!btn) return;
            btn.disabled = loading;
            if (loading) {
                btn.setAttribute('data-orig', btn.innerHTML);
                btn.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Reading...';
            } else {
                var orig = btn.getAttribute('data-orig');
                if (orig) btn.innerHTML = orig;
            }
        },

        takeBaseline: function() {
            var scaleType = this.getScaleType();
            if (!scaleType) { alert('Please select a scale type first.'); return; }
            var self = this;
            var el = document.getElementById('fieldcal-baseline-result');
            el.className = 'test-result waiting';
            el.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Taking 5 baseline samples &mdash; do not touch the hive';
            this.setButtonLoading('btn-fieldcal-baseline', true);

            $.ajax({
                url: 'calibrate_raw_reading.php?scaletype=' + scaleType + '&samples=5',
                timeout: 120000,
                dataType: 'json',
                success: function(data) {
                    self.setButtonLoading('btn-fieldcal-baseline', false);
                    if (data.success) {
                        self.rawBaseline = parseFloat(data.raw_value);
                        el.className = 'test-result success';
                        el.innerHTML = 'Baseline: <strong>' + escHtml(String(data.raw_value)) + '</strong> (' + data.sample_count + ' samples)';
                        setTimeout(function() { self.showStep(2); }, 800);
                    } else {
                        el.className = 'test-result error';
                        el.textContent = data.error || 'Failed to get reading';
                    }
                },
                error: function(xhr, status) {
                    self.setButtonLoading('btn-fieldcal-baseline', false);
                    el.className = 'test-result error';
                    el.innerHTML = (status === 'timeout')
                        ? 'Timeout: sensor did not respond. Check wiring.'
                        : 'Error communicating with sensor.';
                }
            });
        },

        takeLoaded: function() {
            var scaleType = this.getScaleType();
            var knownWeight = parseFloat(document.getElementById('fieldcal-known-weight').value);
            if (!knownWeight || knownWeight <= 0) {
                alert('Please enter a valid weight greater than zero.');
                return;
            }
            var self = this;
            var el = document.getElementById('fieldcal-loaded-result');
            el.className = 'test-result waiting';
            el.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Taking 5 loaded samples &mdash; do not touch the hive';
            this.setButtonLoading('btn-fieldcal-loaded', true);

            $.ajax({
                url: 'calibrate_raw_reading.php?scaletype=' + scaleType + '&samples=5',
                timeout: 120000,
                dataType: 'json',
                success: function(data) {
                    self.setButtonLoading('btn-fieldcal-loaded', false);
                    if (data.success) {
                        self.rawLoaded = parseFloat(data.raw_value);
                        el.className = 'test-result success';
                        el.innerHTML = 'Loaded: <strong>' + escHtml(String(data.raw_value)) + '</strong> (' + data.sample_count + ' samples)';

                        var rawDiff = self.rawLoaded - self.rawBaseline;
                        if (Math.abs(rawDiff) < 1) {
                            el.className = 'test-result error';
                            el.textContent = 'No weight change detected. The baseline and loaded readings are nearly identical. Make sure the weight is on the scale.';
                            return;
                        }

                        if (scaleType === 'phidget1046') {
                            self.newSlope = knownWeight / rawDiff;
                        } else {
                            self.newSlope = rawDiff / knownWeight;
                        }
                        self.newSlope = parseFloat(self.newSlope.toFixed(6));

                        var measuredDelta;
                        if (scaleType === 'phidget1046') {
                            measuredDelta = rawDiff * self.newSlope;
                        } else {
                            measuredDelta = rawDiff / self.newSlope;
                        }

                        var pctChange = ((self.newSlope - self.oldSlope) / Math.abs(self.oldSlope) * 100).toFixed(1);
                        document.getElementById('fieldcal-old-slope').textContent = self.oldSlope;
                        document.getElementById('fieldcal-new-slope').textContent = self.newSlope;
                        document.getElementById('fieldcal-slope-change').textContent = pctChange + '%';
                        document.getElementById('fieldcal-result-known').textContent = knownWeight + ' lbs';
                        document.getElementById('fieldcal-result-delta').textContent = measuredDelta.toFixed(2) + ' lbs';

                        var warn = document.getElementById('fieldcal-warning');
                        var warnText = document.getElementById('fieldcal-warning-text');
                        if (Math.abs(parseFloat(pctChange)) > 20) {
                            warn.style.display = 'block';
                            warnText.textContent = 'Slope changed by ' + pctChange + '%. This is a large shift. Double-check your known weight is accurate and the hive was not disturbed.';
                        } else if (knownWeight < 10) {
                            warn.style.display = 'block';
                            warnText.textContent = 'A heavier known weight (10+ lbs) improves accuracy. Results with light weights may be less precise.';
                        } else {
                            warn.style.display = 'none';
                        }

                        setTimeout(function() { self.showStep(3); }, 800);
                    } else {
                        el.className = 'test-result error';
                        el.textContent = data.error || 'Failed to get reading';
                    }
                },
                error: function(xhr, status) {
                    self.setButtonLoading('btn-fieldcal-loaded', false);
                    el.className = 'test-result error';
                    el.innerHTML = (status === 'timeout')
                        ? 'Timeout: sensor did not respond. Check wiring.'
                        : 'Error communicating with sensor.';
                }
            });
        },

        save: function() {
            var self = this;
            this.setButtonLoading('btn-fieldcal-save', true);

            $.ajax({
                url: 'calibrate_save.php',
                method: 'POST',
                data: { slope: self.newSlope, intercept: self.currentIntercept, scaletype: self.getScaleType() },
                dataType: 'json',
                timeout: 15000,
                success: function(data) {
                    self.setButtonLoading('btn-fieldcal-save', false);
                    if (data.success) {
                        document.getElementById('form-slope').value = self.newSlope;
                        var ms = document.getElementById('manual-slope');
                        if (ms) ms.value = self.newSlope;
                        document.getElementById('fieldcal-save-result').style.display = 'block';
                    } else {
                        alert('Save failed: ' + (data.error || 'Unknown error'));
                    }
                },
                error: function() {
                    self.setButtonLoading('btn-fieldcal-save', false);
                    alert('Failed to save. Check database permissions.');
                }
            });
        }
    };

    // BLE Discover (BroodMinder)
    $(document).on('click', '.ble-discover-btn', function() {
        var btn = $(this), res = btn.siblings('.ble-scan-result');
        btn.prop('disabled', true).find('i').removeClass('fa-bluetooth').addClass('fa-spinner fa-spin');
        res.html('<span class="text-muted">Scanning for BroodMinder devices (~15 seconds)...</span>');
        $.ajax({
            url: 'livevalue.php?sensor=blescan', cache: false, timeout: 30000,
            success: function(d) {
                var lines = d.trim().split('\n');
                var macs = [];
                lines.forEach(function(line) {
                    var m = line.match(/([0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2})/i);
                    if (m && macs.indexOf(m[1].toLowerCase()) === -1) macs.push(m[1].toLowerCase());
                });
                if (macs.length === 0) {
                    res.html('<span class="text-warning"><i class="fa fa-exclamation-triangle"></i> No BroodMinder devices found. Make sure the sensor is nearby and powered on.</span>');
                } else {
                    var html = '<strong>Found ' + macs.length + ' device(s):</strong><br>';
                    macs.forEach(function(mac) {
                        html += '<a href="#" class="ble-pick label label-info" style="margin:2px;display:inline-block;cursor:pointer" data-mac="' + mac + '">' + mac + '</a> ';
                    });
                    html += '<br><small class="text-muted">Click an address to select it.</small>';
                    res.html(html);
                }
            },
            error: function() { res.html('<span class="text-danger"><i class="fa fa-times"></i> Scan failed. Check Bluetooth adapter.</span>'); },
            complete: function() { btn.prop('disabled', false).find('i').removeClass('fa-spinner fa-spin').addClass('fa-bluetooth'); }
        });
    });
    $(document).on('click', '.ble-pick', function(e) {
        e.preventDefault();
        var mac = $(this).data('mac');
        var target = $(this).closest('.form-group').find('input[name="HIVEDEVICE"]');
        if (target.length) target.val(mac).trigger('change');
        $(this).closest('.ble-scan-result').find('.ble-pick').removeClass('label-success');
        $(this).addClass('label-success');
    });

    // Auto-fire test if we came back from a save+test action
    $(document).ready(function() {
        var params = new URLSearchParams(window.location.search);
        var autotest = params.get('autotest');
        if (autotest) {
            runSensorTest(autotest);
        }
    });
    </script>
</body>
</html>
