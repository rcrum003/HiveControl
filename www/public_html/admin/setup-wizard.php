<?PHP
// Setup Wizard - Multi-Step Sensor Configuration Guide
// Version 2026041901
// Walks users through configuring each sensor with live testing

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

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
            if ($v->validate()) {
                $update_fields = ['HIVENAME=?', 'CITY=?', 'STATE=?', 'TIMEZONE=?'];
                $update_values = [
                    test_input($_POST['HIVENAME']),
                    test_input($_POST['CITY']),
                    test_input($_POST['STATE']),
                    test_input_allow_slash($_POST['TIMEZONE'] ?? ($config['TIMEZONE'] ?? 'America/New_York'))
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
                    $update_fields = array_merge($update_fields, ['SCALETYPE=?', 'HIVE_WEIGHT_GPIO=?', 'HIVE_WEIGHT_SLOPE=?', 'HIVE_WEIGHT_INTERCEPT=?']);
                    $update_values = array_merge($update_values, [
                        test_input($_POST['SCALETYPE']),
                        test_input($_POST['HIVE_WEIGHT_GPIO'] ?? ($config['HIVE_WEIGHT_GPIO'] ?? '')),
                        test_input($_POST['HIVE_WEIGHT_SLOPE'] ?? '1'),
                        test_input($_POST['HIVE_WEIGHT_INTERCEPT'] ?? '0')
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
            $update_fields = ['ENABLE_AIR=?'];
            $update_values = [$enable];
            if ($enable === 'yes') {
                $update_fields = array_merge($update_fields, ['AIR_TYPE=?', 'AIR_ID=?']);
                $update_values = array_merge($update_values, [
                    test_input($_POST['AIR_TYPE'] ?? ($config['AIR_TYPE'] ?? '')),
                    test_input($_POST['AIR_ID'] ?? ($config['AIR_ID'] ?? ''))
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
    2 => ['label' => 'Temp/Humidity', 'icon' => 'fa-thermometer-half'],
    3 => ['label' => 'Weight',        'icon' => 'fa-balance-scale'],
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
        case 7: return ($config['ENABLE_AIR'] ?? 'no') === 'yes';
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
        <div class="wizard-header">
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

                <div class="wizard-buttons">
                    <a href="/pages/index.php" class="btn btn-back"><i class="fa fa-dashboard"></i> Dashboard</a>
                    <button type="submit" class="btn btn-primary"><i class="fa fa-arrow-right"></i> Next: Temp/Humidity</button>
                </div>
            </form>


<?php
// ==================== STEP 2: TEMP/HUMIDITY ====================
elseif ($step === 2): ?>
            <div class="step-title"><i class="fa fa-thermometer-half"></i> Temperature &amp; Humidity Sensor</div>
            <div class="step-description">Configure the sensor that monitors conditions inside your hive.</div>

            <form method="POST" action="setup-wizard.php?step=2" id="step-form">
                <input type="hidden" name="wizard_step" value="2">
                <input type="hidden" name="wizard_action" value="next" id="wizard_action_field">
                <input type="hidden" name="test_sensor_type" value="" id="test_sensor_type_field">

                <div class="enable-toggle">
                    <label><i class="fa fa-thermometer-half"></i> Enable Temp/Humidity Sensor</label>
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
                        'dht22' => ['DHT22 (GPIO)', 'Digital humidity and temperature sensor'],
                        'dht21' => ['DHT21 (GPIO)', 'Digital humidity and temperature sensor'],
                        'sht31d' => ['SHT31-D (I2C)', 'High-accuracy temperature and humidity sensor'],
                        'bme280' => ['BME280 (I2C)', 'Temperature, humidity, and pressure sensor'],
                        'bme680' => ['BME680 (I2C)', 'Environmental sensor with gas detection'],
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
                                <input type="text" class="form-control" name="HIVEDEVICE" value="<?php echo htmlspecialchars($config['HIVEDEVICE'] ?? ''); ?>" placeholder="06:09:42:1c:8a">
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
            <div class="step-title"><i class="fa fa-balance-scale"></i> Weight Scale</div>
            <div class="step-description">Configure the scale that monitors your hive's weight for tracking nectar flow and colony health.</div>

            <form method="POST" action="setup-wizard.php?step=3" id="step-form">
                <input type="hidden" name="wizard_step" value="3">
                <input type="hidden" name="wizard_action" value="next" id="wizard_action_field">
                <input type="hidden" name="test_sensor_type" value="" id="test_sensor_type_field">

                <div class="enable-toggle">
                    <label><i class="fa fa-balance-scale"></i> Enable Weight Scale</label>
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
                            <span class="help-text">Guided calibration: zero the scale, place a known weight, auto-calculate.</span>
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
                                <h4 style="margin-top:0;"><i class="fa fa-balance-scale"></i> Step 1: Zero the Scale</h4>
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
                                    <button type="button" class="btn btn-test" id="btn-verify" onclick="CalWiz.verify()">
                                        <i class="fa fa-check"></i> Verify Calibration
                                    </button>
                                    <button type="button" class="btn btn-primary" id="btn-save-cal" onclick="CalWiz.save()" style="margin-left:8px;">
                                        <i class="fa fa-floppy-o"></i> Save Calibration
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
                    'hive' => ['WX Underground', 'Online weather service &mdash; requires station ID'],
                    'ambientwx' => ['AmbientWeather.net', 'Ambient Weather network &mdash; requires API key and station ID'],
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

                <!-- WX Underground options -->
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
                    $wx_sensor_types = ['temperhum','dht21','dht22','sht31d','bme280','bme680'];
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
                    <label>Select Air Quality Source:</label>
                    <div class="sensor-option <?php echo (($config['AIR_TYPE'] ?? '') === 'purple') ? 'selected' : ''; ?>">
                        <input type="radio" name="AIR_TYPE" value="purple" id="air_purple"
                            <?php echo (($config['AIR_TYPE'] ?? '') === 'purple') ? 'checked' : ''; ?>>
                        <label for="air_purple"><strong>PurpleAir</strong> &mdash; Community air quality sensor network</label>
                    </div>

                    <div class="form-group" style="margin-top:15px;">
                        <label>Station ID</label>
                        <input type="text" class="form-control" name="AIR_ID" value="<?php echo htmlspecialchars($config['AIR_ID'] ?? ''); ?>" placeholder="Find your sensor at purpleair.com/map">
                        <p class="help-text">Go to <a href="https://www.purpleair.com/map" target="_blank">purpleair.com/map</a> to find a sensor near your hives and get its ID.</p>
                    </div>

                    <div class="test-panel">
                        <h4><i class="fa fa-bolt"></i> Test Air Quality</h4>
                        <p class="help-text">Save settings first, then test to verify data is arriving.</p>
                        <button type="button" class="btn btn-test" onclick="testSensor('airquality')"><i class="fa fa-play"></i> Test Now</button>
                        <div class="test-result waiting" id="test-result">Waiting for test...</div>
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
                    'Timezone' => $config['TIMEZONE'] ?? 'Not set',
                ]],
                2 => ['Temp/Humidity', 'fa-thermometer-half', [
                    'Status' => $config['ENABLE_HIVE_TEMP_CHK'] ?? 'no',
                    'Sensor Type' => (($config['ENABLE_HIVE_TEMP_CHK'] ?? 'no') === 'yes') ? strtoupper($config['TEMPTYPE'] ?? '') : 'N/A',
                ]],
                3 => ['Weight Scale', 'fa-balance-scale', [
                    'Status' => $config['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no',
                    'Scale Type' => (($config['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no') === 'yes') ? strtoupper($config['SCALETYPE'] ?? '') : 'N/A',
                    'Calibration' => (($config['ENABLE_HIVE_WEIGHT_CHK'] ?? 'no') === 'yes')
                        ? 'Slope: ' . ($config['HIVE_WEIGHT_SLOPE'] ?? '1') . ', Intercept: ' . ($config['HIVE_WEIGHT_INTERCEPT'] ?? '0')
                        : 'N/A',
                ]],
                4 => ['Light Sensor', 'fa-sun-o', [
                    'Status' => $config['ENABLE_LUX'] ?? 'no',
                    'Source' => (($config['ENABLE_LUX'] ?? 'no') === 'yes') ? strtoupper($config['LUX_SOURCE'] ?? '') : 'N/A',
                ]],
                5 => ['Weather Source', 'fa-cloud', [
                    'Source' => strtoupper($config['WEATHER_LEVEL'] ?? 'Not set'),
                    'Station' => !empty($config['WXSTATION']) ? $config['WXSTATION'] : 'N/A',
                ]],
                6 => ['Camera & Counter', 'fa-camera', [
                    'Camera' => ($config['ENABLE_HIVE_CAMERA'] ?? 'no') . ((($config['ENABLE_HIVE_CAMERA'] ?? 'no') === 'yes') ? ' (' . ($config['CAMERATYPE'] ?? '') . ')' : ''),
                    'Bee Counter' => ($config['ENABLE_BEECOUNTER'] ?? 'no') . ((($config['ENABLE_BEECOUNTER'] ?? 'no') === 'yes') ? ' (' . ($config['COUNTERTYPE'] ?? '') . ')' : ''),
                ]],
                7 => ['Air Quality', 'fa-leaf', [
                    'Status' => $config['ENABLE_AIR'] ?? 'no',
                    'Source' => (($config['ENABLE_AIR'] ?? 'no') === 'yes') ? strtoupper($config['AIR_TYPE'] ?? '') : 'N/A',
                ]],
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
        var resultId = (type === 'camera') ? 'test-result-cam' : 'test-result';
        var el = document.getElementById(resultId);
        if (!el) return;
        el.className = 'test-result waiting';
        el.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Settings saved. Reading sensor... this may take up to a minute';

        var ajaxTimeout = (type === 'hiveweight') ? 90000 : 30000;
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
        if (!cpwNotice) return;
        if (v === 'cpw200plus') {
            cpwNotice.style.display = 'block';
            if (calLauncher) calLauncher.style.display = 'none';
            if (calWizard) calWizard.style.display = 'none';
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
            el.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Taking zero reading... this takes a few seconds';
            this.setButtonLoading('btn-zero', true);

            $.ajax({
                url: 'calibrate_raw_reading.php?scaletype=' + scaleType,
                timeout: 60000,
                dataType: 'json',
                success: function(data) {
                    self.setButtonLoading('btn-zero', false);
                    if (data.success) {
                        self.rawZero = parseFloat(data.raw_value);
                        el.className = 'test-result success';
                        el.innerHTML = 'Zero reading: <strong>' + data.raw_value + '</strong>';
                        setTimeout(function() { self.showStep(2); }, 800);
                    } else {
                        el.className = 'test-result error';
                        el.innerHTML = data.error || 'Failed to get reading';
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
            el.innerHTML = '<i class="fa fa-spinner fa-spin"></i> Taking loaded reading... this takes a few seconds';
            this.setButtonLoading('btn-loaded', true);

            $.ajax({
                url: 'calibrate_raw_reading.php?scaletype=' + scaleType,
                timeout: 60000,
                dataType: 'json',
                success: function(data) {
                    self.setButtonLoading('btn-loaded', false);
                    if (data.success) {
                        self.rawLoaded = parseFloat(data.raw_value);
                        el.className = 'test-result success';
                        el.innerHTML = 'Loaded reading: <strong>' + data.raw_value + '</strong>';

                        var result = self.calculate(self.rawZero, self.rawLoaded, knownWeight, scaleType);
                        if (result.error) {
                            el.className = 'test-result error';
                            el.innerHTML = result.error;
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
                        el.innerHTML = data.error || 'Failed to get reading';
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
                slope: parseFloat(slope.toFixed(2)),
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
                data: { slope: self.slope, intercept: self.intercept },
                dataType: 'json',
                timeout: 15000,
                success: function() {
                    // Now read through the normal calibrated endpoint
                    $.ajax({
                        url: 'livevalue.php?sensor=hiveweight',
                        timeout: 90000,
                        success: function(data) {
                            self.setButtonLoading('btn-verify', false);
                            if (data && data.trim().length > 0) {
                                el.className = 'test-result success';
                                el.innerHTML = '<strong>Verification reading:</strong><br>' + data.replace(/\n/g, '<br>') + '<br><br><em>Compare this to the weight you placed on the scale.</em>';
                            } else {
                                el.className = 'test-result error';
                                el.innerHTML = 'No data returned. Check sensor.';
                            }
                        },
                        error: function() {
                            self.setButtonLoading('btn-verify', false);
                            el.className = 'test-result error';
                            el.innerHTML = 'Verification read failed. The calibration values were saved — try "Test Scale" below.';
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
                data: { slope: self.slope, intercept: self.intercept },
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
