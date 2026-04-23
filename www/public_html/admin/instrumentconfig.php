<?PHP
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
include($_SERVER["DOCUMENT_ROOT"] . "/include/security-init.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "/include/sensor_health_check.php");
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

function h($v) { return htmlspecialchars($v ?? '', ENT_QUOTES, 'UTF-8'); }
function sel($a, $b) { return $a == $b ? " selected='selected'" : ''; }
function chk($a, $b) { return $a == $b ? ' checked' : ''; }

$save_success = false;
$save_errors = [];

if ($_SERVER["REQUEST_METHOD"] == "GET") {
    $sth = $conn->prepare("SELECT * FROM hiveconfig");
    $sth->execute();
    $result = $sth->fetch(PDO::FETCH_ASSOC);
}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    require_csrf_token();

    $v = new Valitron\Validator($_POST);
    $v->rule('required', ['ENABLE_HIVE_CAMERA', 'ENABLE_HIVE_WEIGHT_CHK', 'ENABLE_HIVE_TEMP_CHK', 'ENABLE_LUX'], 1)->message('{field} is required');
    $v->rule('slug', ['WXTEMPTYPE']);
    $v->rule('in', ['ENABLE_HIVE_WEIGHT', 'ENABLE_LUX', 'ENABLE_HIVE_CAMERA', 'ENABLE_HIVE_WEIGHT_CHK', 'ENABLE_HIVE_TEMP_CHK', 'ENABLE_BEECOUNTER', 'ENABLE_AIR', 'ENABLE_POLLEN'], ['no', 'yes']);
    $v->rule('integer', ['HIVE_TEMP_GPIO', 'HIVE_LUX_GPIO', 'HIVE_WEIGHT_GPIO', 'HIVE_TEMP_SUB'], 1)->message('{field} must be a integer');
    $v->rule('numeric', ['HIVE_WEIGHT_SLOPE', 'HIVE_WEIGHT_INTERCEPT', 'HIVE_LUX_SLOPE', 'HIVE_LUX_INTERCEPT', 'HIVE_TEMP_SLOPE', 'HIVE_TEMP_INTERCEPT', 'WX_TEMP_SLOPE','WX_TEMP_INTERCEPT','HIVE_HUMIDITY_SLOPE','HIVE_HUMIDITY_INTERCEPT','WX_HUMIDITY_SLOPE','WX_HUMIDITY_INTERCEPT','WEIGHT_TEMP_COEFF','WEIGHT_HUMIDITY_COEFF','WEIGHT_REF_TEMP','WEIGHT_REF_HUMIDITY'], 1)->message('{field} must be numeric');
    $v->rule('in', ['WEIGHT_COMPENSATION_ENABLED'], ['no', 'yes']);
    $v->rule('alphaNum', ['SCALETYPE', 'TEMPTYPE', 'LUX_SOURCE', 'COUNTERTYPE', 'CAMERATYPE', 'local_wx_type', 'AIR_ID', 'AIR_TYPE'], 1)->message('{field} must be alphaNum only');
    $v->rule('integer', ['WX_MAX_STALE_MINUTES']);
    $v->rule('min', ['WX_MAX_STALE_MINUTES'], 30);
    $v->rule('max', ['WX_MAX_STALE_MINUTES'], 360);
    $v->rule('lengthmax', ['WX_TEMP_GPIO', 'HIVE_LUX_GPIO', 'HIVE_WEIGHT_GPIO'], 2);

    if ($v->validate()) {
        $sth = $conn->prepare("SELECT * FROM hiveconfig");
        $sth->execute();
        $old = $sth->fetch(PDO::FETCH_ASSOC);

        $ENABLE_HIVE_TEMP_CHK = test_input($_POST["ENABLE_HIVE_TEMP_CHK"]);
        if ($ENABLE_HIVE_TEMP_CHK == "no") {
            $TEMPTYPE = $old['TEMPTYPE']; $HIVEDEVICE = $old['HIVEDEVICE'];
            $HIVE_TEMP_SUB = $old['HIVE_TEMP_SUB']; $HIVE_TEMP_GPIO = $old['HIVE_TEMP_GPIO'];
            $HIVE_TEMP_SLOPE = $old['HIVE_TEMP_SLOPE']; $HIVE_TEMP_INTERCEPT = $old['HIVE_TEMP_INTERCEPT'];
            $HIVE_HUMIDITY_SLOPE = $old['HIVE_HUMIDITY_SLOPE']; $HIVE_HUMIDITY_INTERCEPT = $old['HIVE_HUMIDITY_INTERCEPT'];
        } else {
            $TEMPTYPE = test_input($_POST["TEMPTYPE"]); $HIVEDEVICE = test_input($_POST["HIVEDEVICE"]);
            $HIVE_TEMP_SUB = test_input($_POST["HIVE_TEMP_SUB"]); $HIVE_TEMP_GPIO = test_input($_POST["HIVE_TEMP_GPIO"]);
            $HIVE_TEMP_SLOPE = test_input($_POST["HIVE_TEMP_SLOPE"]); $HIVE_TEMP_INTERCEPT = test_input($_POST["HIVE_TEMP_INTERCEPT"]);
            $HIVE_HUMIDITY_SLOPE = test_input($_POST["HIVE_HUMIDITY_SLOPE"]); $HIVE_HUMIDITY_INTERCEPT = test_input($_POST["HIVE_HUMIDITY_INTERCEPT"]);
        }

        $ENABLE_HIVE_WEIGHT_CHK = test_input($_POST["ENABLE_HIVE_WEIGHT_CHK"]);
        if ($ENABLE_HIVE_WEIGHT_CHK == "no") {
            $SCALETYPE = $old['SCALETYPE']; $HIVE_WEIGHT_GPIO = $old['HIVE_WEIGHT_GPIO'];
            $HIVE_WEIGHT_SLOPE = $old['HIVE_WEIGHT_SLOPE']; $HIVE_WEIGHT_INTERCEPT = $old['HIVE_WEIGHT_INTERCEPT'];
            $WEIGHT_COMPENSATION_ENABLED = $old['WEIGHT_COMPENSATION_ENABLED'] ?? 'no';
            $WEIGHT_TEMP_COEFF = $old['WEIGHT_TEMP_COEFF'] ?? '0'; $WEIGHT_HUMIDITY_COEFF = $old['WEIGHT_HUMIDITY_COEFF'] ?? '0';
            $WEIGHT_REF_TEMP = $old['WEIGHT_REF_TEMP'] ?? ''; $WEIGHT_REF_HUMIDITY = $old['WEIGHT_REF_HUMIDITY'] ?? '';
        } else {
            $SCALETYPE = test_input($_POST["SCALETYPE"]); $HIVE_WEIGHT_GPIO = test_input($_POST["HIVE_WEIGHT_GPIO"]);
            $HIVE_WEIGHT_SLOPE = test_input($_POST["HIVE_WEIGHT_SLOPE"]); $HIVE_WEIGHT_INTERCEPT = test_input($_POST["HIVE_WEIGHT_INTERCEPT"]);
            $WEIGHT_COMPENSATION_ENABLED = test_input($_POST["WEIGHT_COMPENSATION_ENABLED"] ?? 'no');
            $WEIGHT_TEMP_COEFF = test_input($_POST["WEIGHT_TEMP_COEFF"] ?? '0'); $WEIGHT_HUMIDITY_COEFF = test_input($_POST["WEIGHT_HUMIDITY_COEFF"] ?? '0');
            $WEIGHT_REF_TEMP = test_input($_POST["WEIGHT_REF_TEMP"] ?? ''); $WEIGHT_REF_HUMIDITY = test_input($_POST["WEIGHT_REF_HUMIDITY"] ?? '');
        }

        $ENABLE_LUX = test_input($_POST["ENABLE_LUX"]);
        if ($ENABLE_LUX == "no") {
            $LUX_SOURCE = $old['LUX_SOURCE']; $HIVE_LUX_SLOPE = $old['HIVE_LUX_SLOPE'];
            $HIVE_LUX_INTERCEPT = $old['HIVE_LUX_INTERCEPT']; $HIVE_LUX_GPIO = $old['HIVE_LUX_GPIO'];
        } else {
            $LUX_SOURCE = test_input($_POST["LUX_SOURCE"]); $HIVE_LUX_SLOPE = test_input($_POST["HIVE_LUX_SLOPE"]);
            $HIVE_LUX_INTERCEPT = test_input($_POST["HIVE_LUX_INTERCEPT"]); $HIVE_LUX_GPIO = test_input($_POST["HIVE_LUX_GPIO"]);
        }

        $ENABLE_HIVE_CAMERA = test_input($_POST["ENABLE_HIVE_CAMERA"]);
        $ENABLE_BEECOUNTER = test_input($_POST["ENABLE_BEECOUNTER"]);
        $CAMERATYPE = test_input($_POST["CAMERATYPE"]);
        $COUNTERTYPE = test_input($_POST["COUNTERTYPE"]);

        $weather_level = test_input($_POST["WEATHER_LEVEL"]);
        $weather_detail = test_input($_POST["WEATHER_DETAIL"]);
        $key = test_input($_POST["KEY"]);
        $key_openweathermap = test_input($_POST["KEY_OPENWEATHERMAP"] ?? '');
        $key_weatherapi = test_input($_POST["KEY_WEATHERAPI"] ?? '');
        $key_visualcrossing = test_input($_POST["KEY_VISUALCROSSING"] ?? '');
        $key_pirateweather = test_input($_POST["KEY_PIRATEWEATHER"] ?? '');
        $key_tomorrow = test_input($_POST["KEY_TOMORROW"] ?? '');
        $key_ambee = test_input($_POST["KEY_AMBEE"] ?? '');
        $wxstation = test_input($_POST["WXSTATION"]);
        $wxapikey = trim(strip_tags($_POST["WXAPIKEY"] ?? ''));
        $WXTEMPTYPE = test_input($_POST["WXTEMPTYPE"]);
        $WX_TEMPER_DEVICE = test_input_allow_slash($_POST["WX_TEMPER_DEVICE"]);
        $WX_TEMP_GPIO = test_input($_POST["WX_TEMP_GPIO"]);
        $weather_fallback = test_input($_POST["WEATHER_FALLBACK"] ?? '');
        $weather_fallback_2 = test_input($_POST["WEATHER_FALLBACK_2"] ?? '');
        $wx_max_stale_minutes = test_input($_POST["WX_MAX_STALE_MINUTES"] ?? '120');
        $local_wx_type = test_input($_POST["local_wx_type"]);
        $local_wx_url = test_input($_POST["local_wx_url"]);
        $WX_TEMP_SLOPE = test_input($_POST["WX_TEMP_SLOPE"]);
        $WX_TEMP_INTERCEPT = test_input($_POST["WX_TEMP_INTERCEPT"]);
        $WX_HUMIDITY_SLOPE = test_input($_POST["WX_HUMIDITY_SLOPE"]);
        $WX_HUMIDITY_INTERCEPT = test_input($_POST["WX_HUMIDITY_INTERCEPT"]);

        $ENABLE_AIR = test_input($_POST["ENABLE_AIR"]);
        $AIR_TYPE = test_input($_POST["AIR_TYPE"]);
        $AIR_ID = test_input($_POST["AIR_ID"]);
        $AIR_API = test_input($_POST["AIR_API"] ?? '');
        $AIR_LOCAL_URL = test_input($_POST["AIR_LOCAL_URL"] ?? '');
        $ENABLE_AIRNOW = test_input($_POST["ENABLE_AIRNOW"] ?? 'no');
        $KEY_AIRNOW = test_input($_POST["KEY_AIRNOW"] ?? '');
        $AIRNOW_DISTANCE = test_input($_POST["AIRNOW_DISTANCE"] ?? '25');
        $ENABLE_POLLEN = test_input($_POST["ENABLE_POLLEN"] ?? 'yes');

        $ver = $conn->prepare("SELECT version FROM hiveconfig");
        $ver->execute();
        $ver = $ver->fetchColumn();
        $version = ++$ver;

        try {
            $doit = $conn->prepare("UPDATE hiveconfig SET ENABLE_HIVE_CAMERA=?,ENABLE_HIVE_WEIGHT_CHK=?,ENABLE_HIVE_TEMP_CHK=?,SCALETYPE=?,TEMPTYPE=?,version=?,HIVEDEVICE=?,ENABLE_LUX=?,LUX_SOURCE=?,HIVE_TEMP_GPIO=?,HIVE_WEIGHT_SLOPE=?,HIVE_WEIGHT_INTERCEPT=?,ENABLE_BEECOUNTER=?,CAMERATYPE=?,COUNTERTYPE=?,weather_level=?,key=?,wxstation=?,WXTEMPTYPE=?,WX_TEMPER_DEVICE=?,WX_TEMP_GPIO=?,weather_detail=?,local_wx_type=?,local_wx_url=?, HIVE_LUX_SLOPE=?, HIVE_LUX_INTERCEPT=?, HIVE_TEMP_SLOPE=?, HIVE_TEMP_INTERCEPT=?, WX_TEMP_SLOPE=?, WX_TEMP_INTERCEPT=?, HIVE_HUMIDITY_SLOPE=?, HIVE_HUMIDITY_INTERCEPT=?, WX_HUMIDITY_SLOPE=?, WX_HUMIDITY_INTERCEPT=?, HIVE_LUX_GPIO=?, HIVE_WEIGHT_GPIO=?,HIVE_TEMP_SUB=?,ENABLE_AIR=?,AIR_TYPE=?,AIR_ID=?,AIR_API=?,AIR_LOCAL_URL=?,WEIGHT_COMPENSATION_ENABLED=?,WEIGHT_TEMP_COEFF=?,WEIGHT_HUMIDITY_COEFF=?,WEIGHT_REF_TEMP=?,WEIGHT_REF_HUMIDITY=?,WEATHER_FALLBACK=?,WEATHER_FALLBACK_2=?,WX_MAX_STALE_MINUTES=?,KEY_OPENWEATHERMAP=?,KEY_WEATHERAPI=?,KEY_VISUALCROSSING=?,KEY_PIRATEWEATHER=?,KEY_TOMORROW=?,KEY_AMBEE=?,ENABLE_AIRNOW=?,KEY_AIRNOW=?,AIRNOW_DISTANCE=?,WXAPIKEY=?,ENABLE_POLLEN=? WHERE id=1");
            $doit->execute(array($ENABLE_HIVE_CAMERA,$ENABLE_HIVE_WEIGHT_CHK,$ENABLE_HIVE_TEMP_CHK,$SCALETYPE,$TEMPTYPE,$version,$HIVEDEVICE,$ENABLE_LUX,$LUX_SOURCE,$HIVE_TEMP_GPIO,$HIVE_WEIGHT_SLOPE,$HIVE_WEIGHT_INTERCEPT,$ENABLE_BEECOUNTER,$CAMERATYPE,$COUNTERTYPE,$weather_level,$key,$wxstation,$WXTEMPTYPE,$WX_TEMPER_DEVICE,$WX_TEMP_GPIO,$weather_detail,$local_wx_type,$local_wx_url,$HIVE_LUX_SLOPE,$HIVE_LUX_INTERCEPT,$HIVE_TEMP_SLOPE,$HIVE_TEMP_INTERCEPT,$WX_TEMP_SLOPE,$WX_TEMP_INTERCEPT,$HIVE_HUMIDITY_SLOPE,$HIVE_HUMIDITY_INTERCEPT,$WX_HUMIDITY_SLOPE,$WX_HUMIDITY_INTERCEPT,$HIVE_LUX_GPIO,$HIVE_WEIGHT_GPIO,$HIVE_TEMP_SUB,$ENABLE_AIR,$AIR_TYPE,$AIR_ID,$AIR_API,$AIR_LOCAL_URL,$WEIGHT_COMPENSATION_ENABLED,$WEIGHT_TEMP_COEFF,$WEIGHT_HUMIDITY_COEFF,$WEIGHT_REF_TEMP,$WEIGHT_REF_HUMIDITY,$weather_fallback,$weather_fallback_2,$wx_max_stale_minutes,$key_openweathermap,$key_weatherapi,$key_visualcrossing,$key_pirateweather,$key_tomorrow,$key_ambee,$ENABLE_AIRNOW,$KEY_AIRNOW,$AIRNOW_DISTANCE,$wxapikey,$ENABLE_POLLEN));
            sleep(1);
            $save_success = true;
        } catch (PDOException $e) {
            $save_errors[] = ['Save failed: ' . $e->getMessage()];
        }

        $sth = $conn->prepare("SELECT * FROM hiveconfig");
        $sth->execute();
        $result = $sth->fetch(PDO::FETCH_ASSOC);
    } else {
        $save_errors = array_values($v->errors());
        $sth = $conn->prepare("SELECT * FROM hiveconfig");
        $sth->execute();
        $result = $sth->fetch(PDO::FETCH_ASSOC);
    }
}

$health = get_sensor_health($conn);
$sh = $health['sensors'];

$active_tab = 'hive';
if ($_SERVER["REQUEST_METHOD"] == "POST" && isset($_POST['_active_tab'])) {
    $at = test_input($_POST['_active_tab']);
    if (in_array($at, ['hive', 'weather', 'environment'])) $active_tab = $at;
}

$wx_health_rows = [];
try {
    $wh_sth = $conn->prepare("SELECT provider, COUNT(*) as total, SUM(success) as ok, ROUND(AVG(success)*100,0) as rate FROM weather_health WHERE timestamp >= datetime('now', '-1 day', 'localtime') GROUP BY provider ORDER BY total DESC");
    $wh_sth->execute();
    $wx_health_rows = $wh_sth->fetchAll(PDO::FETCH_ASSOC);
} catch (Exception $e) {}

$wx_lat = h($result['LATITUDE'] ?? '');
$wx_lon = h($result['LONGITUDE'] ?? '');

$hidden_fields = [
    'ENABLE_HIVE_TEMP_CHK','TEMPTYPE','HIVEDEVICE','HIVE_TEMP_SUB','HIVE_TEMP_GPIO',
    'HIVE_TEMP_SLOPE','HIVE_TEMP_INTERCEPT','HIVE_HUMIDITY_SLOPE','HIVE_HUMIDITY_INTERCEPT',
    'ENABLE_HIVE_WEIGHT_CHK','SCALETYPE','HIVE_WEIGHT_GPIO','HIVE_WEIGHT_SLOPE','HIVE_WEIGHT_INTERCEPT',
    'WEIGHT_COMPENSATION_ENABLED','WEIGHT_TEMP_COEFF','WEIGHT_HUMIDITY_COEFF','WEIGHT_REF_TEMP','WEIGHT_REF_HUMIDITY',
    'ENABLE_LUX','LUX_SOURCE','HIVE_LUX_GPIO','HIVE_LUX_SLOPE','HIVE_LUX_INTERCEPT',
    'ENABLE_BEECOUNTER','COUNTERTYPE','ENABLE_HIVE_CAMERA','CAMERATYPE',
    'WEATHER_LEVEL','WEATHER_DETAIL','KEY','KEY_OPENWEATHERMAP','KEY_WEATHERAPI','KEY_VISUALCROSSING',
    'KEY_PIRATEWEATHER','KEY_TOMORROW','KEY_AMBEE','WXSTATION','WXAPIKEY','WXTEMPTYPE',
    'WX_TEMPER_DEVICE','WX_TEMP_GPIO','local_wx_type','local_wx_url',
    'WX_TEMP_SLOPE','WX_TEMP_INTERCEPT','WX_HUMIDITY_SLOPE','WX_HUMIDITY_INTERCEPT',
    'WEATHER_FALLBACK','WEATHER_FALLBACK_2','WX_MAX_STALE_MINUTES',
    'ENABLE_AIR','AIR_TYPE','AIR_ID','AIR_API','AIR_LOCAL_URL',
    'ENABLE_AIRNOW','KEY_AIRNOW','AIRNOW_DISTANCE',
    'ENABLE_POLLEN'
];
?>

<!DOCTYPE html>
<html lang="en">

    <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>

    <div id="wrapper">
        <div class="row">
            <div class="col-lg-12">
                <h1 class="page-header">Settings - Instruments</h1>
            </div>
        </div>

<?php if ($save_success): ?>
        <div class="alert alert-success alert-dismissable" id="save-alert">
            <button type="button" class="close" data-dismiss="alert">&times;</button>
            <i class="fa fa-check"></i> Configuration saved successfully.
        </div>
<?php endif; ?>

<?php if (!empty($save_errors)): ?>
        <div class="alert alert-danger alert-dismissable">
            <button type="button" class="close" data-dismiss="alert">&times;</button>
            <?php foreach ($save_errors as $err) { echo h($err[0]) . "<br>"; } ?>
        </div>
<?php endif; ?>

        <!-- Sensor Status -->
        <div style="margin-bottom:15px">
<?php
$badge_map = [
    'hivetemp' => ['l' => 'Temp', 'i' => 'fa-thermometer-half'],
    'weight'   => ['l' => 'Weight', 'i' => 'fa-balance-scale'],
    'weather'  => ['l' => 'Weather', 'i' => 'fa-cloud'],
    'light'    => ['l' => 'Light', 'i' => 'fa-sun-o'],
    'beecount' => ['l' => 'Bees', 'i' => 'fa-bug'],
    'air'      => ['l' => 'Air', 'i' => 'fa-industry'],
    'pollen'   => ['l' => 'Pollen', 'i' => 'fa-pagelines'],
];
foreach ($badge_map as $bk => $b) {
    $bs = $sh[$bk];
    if (!$bs['enabled'] && !$bs['always']) continue;
    echo '<span class="label ' . status_label_class($bs['status']) . '" style="margin-right:4px;font-size:12px;padding:5px 8px"><i class="fa ' . $b['i'] . '"></i> ' . $b['l'] . '</span> ';
}
?>
            <a href="/admin/sensor_health.php" class="btn btn-default btn-xs" style="margin-left:8px"><i class="fa fa-heartbeat"></i> Health Dashboard</a>
        </div>

        <form method="POST" action="<?= h($_SERVER["PHP_SELF"]) ?>">
            <?php echo csrf_field(); ?>
            <input type="hidden" name="_active_tab" id="_active_tab" value="<?= h($active_tab) ?>">
<?php foreach ($hidden_fields as $fn): ?>
            <input type="hidden" name="<?= h($fn) ?>" value="<?= h($result[$fn] ?? '') ?>">
<?php endforeach; ?>

            <!-- Tabs -->
            <ul class="nav nav-tabs" id="config-tabs">
                <li<?= $active_tab === 'hive' ? ' class="active"' : '' ?>><a data-toggle="tab" href="#tab-hive"><i class="fa fa-home"></i> Hive Sensors</a></li>
                <li<?= $active_tab === 'weather' ? ' class="active"' : '' ?>><a data-toggle="tab" href="#tab-weather"><i class="fa fa-cloud"></i> Weather</a></li>
                <li<?= $active_tab === 'environment' ? ' class="active"' : '' ?>><a data-toggle="tab" href="#tab-environment"><i class="fa fa-leaf"></i> Environment</a></li>
            </ul>

            <div class="tab-content" style="padding-top:15px">

<!-- ========== TAB: HIVE SENSORS ========== -->
<div id="tab-hive" class="tab-pane fade<?= $active_tab === 'hive' ? ' in active' : '' ?>">

<?php /* --- TEMP/HUMIDITY --- */
$temp_on = ($result['ENABLE_HIVE_TEMP_CHK'] == 'yes');
$temp_pcls = $temp_on ? status_panel_class($sh['hivetemp']['status']) : 'panel-default';
?>
<div class="panel <?= $temp_pcls ?>">
    <div class="panel-heading">
        <div class="row">
            <div class="col-xs-8"><i class="fa fa-thermometer-half fa-fw"></i> <strong>Temperature / Humidity</strong>
            <?php if ($temp_on): ?><span class="label <?= status_label_class($sh['hivetemp']['status']) ?>" style="margin-left:8px"><?= h($sh['hivetemp']['status_label']) ?></span><?php endif; ?>
            </div>
            <div class="col-xs-4 text-right">
                <select name="ENABLE_HIVE_TEMP_CHK" class="form-control input-sm enable-toggle" data-target="#body-temp" style="display:inline-block;width:auto">
                    <option value="yes"<?= sel($result['ENABLE_HIVE_TEMP_CHK'], 'yes') ?>>Enabled</option>
                    <option value="no"<?= sel($result['ENABLE_HIVE_TEMP_CHK'], 'no') ?>>Disabled</option>
                </select>
            </div>
        </div>
    </div>
    <div id="body-temp" class="panel-body collapse<?= $temp_on ? ' in' : '' ?>">
        <div class="form-group">
            <label>Sensor Type</label><br>
<?php
$ttypes = ['temperhum'=>'TemperHum','temper'=>'Temper','dht22'=>'DHT22','dht21'=>'DHT21','sht31d'=>'SHT31-D','bme280'=>'BME280','bme680'=>'BME680','broodminder'=>'BroodMinder'];
foreach ($ttypes as $tv => $tl) {
    echo '<label class="radio-inline"><input type="radio" name="TEMPTYPE" value="'.$tv.'"'.chk($result['TEMPTYPE'],$tv).' class="temp-type-radio"> '.$tl.'</label>';
}
?>
        </div>
        <div class="temp-option" data-types="temperhum,temper" style="<?= in_array($result['TEMPTYPE'], ['temperhum','temper']) ? '' : 'display:none' ?>">
            <div class="form-group"><label>Device Path</label>
                <input type="text" name="HIVEDEVICE" class="form-control" style="max-width:300px" value="<?= h($result['HIVEDEVICE']) ?>" placeholder="/dev/hidraw1">
                <p class="help-block">Use <code>tempered -e</code> to list devices</p>
            </div>
        </div>
        <div class="temp-option" data-types="broodminder" style="<?= $result['TEMPTYPE'] == 'broodminder' ? '' : 'display:none' ?>">
            <div class="form-group"><label>BLE MAC Address</label>
                <input type="text" name="HIVEDEVICE" class="form-control" style="max-width:300px" value="<?= h($result['HIVEDEVICE']) ?>" placeholder="06:09:16:42:1c:8a">
                <p class="help-block">Run <code>sudo python3 /home/HiveControl/software/broodminder/BM_Scan_bleak.py</code> to find nearby devices</p>
            </div>
        </div>
        <div class="temp-option" data-types="temper" style="<?= $result['TEMPTYPE'] == 'temper' ? '' : 'display:none' ?>">
            <div class="form-group"><label>Sub-Device</label>
                <input type="text" name="HIVE_TEMP_SUB" class="form-control" style="max-width:100px" value="<?= h($result['HIVE_TEMP_SUB']) ?>">
            </div>
        </div>
        <div class="temp-option" data-types="dht22,dht21" style="<?= in_array($result['TEMPTYPE'], ['dht22','dht21']) ? '' : 'display:none' ?>">
            <div class="form-group"><label>GPIO Pin</label>
                <input type="text" name="HIVE_TEMP_GPIO" class="form-control" style="max-width:80px" value="<?= h($result['HIVE_TEMP_GPIO']) ?>">
            </div>
        </div>
        <hr style="margin:10px 0">
        <label>Calibration</label>
        <p class="help-block" style="margin-top:0">Offset is subtracted from raw reading (0 = none). Slope is multiplied after offset (1 = no scaling).</p>
        <div class="row">
            <div class="col-sm-3"><label class="small text-muted">Temp Offset</label><input type="text" name="HIVE_TEMP_INTERCEPT" class="form-control input-sm" value="<?= h($result['HIVE_TEMP_INTERCEPT']) ?>"></div>
            <div class="col-sm-3"><label class="small text-muted">Temp Slope</label><input type="text" name="HIVE_TEMP_SLOPE" class="form-control input-sm" value="<?= h($result['HIVE_TEMP_SLOPE']) ?>"></div>
            <div class="col-sm-3"><label class="small text-muted">Humidity Offset</label><input type="text" name="HIVE_HUMIDITY_INTERCEPT" class="form-control input-sm" value="<?= h($result['HIVE_HUMIDITY_INTERCEPT']) ?>"></div>
            <div class="col-sm-3"><label class="small text-muted">Humidity Slope</label><input type="text" name="HIVE_HUMIDITY_SLOPE" class="form-control input-sm" value="<?= h($result['HIVE_HUMIDITY_SLOPE']) ?>"></div>
        </div>
        <div style="margin-top:12px">
            <button type="button" class="btn btn-info btn-sm test-sensor" data-sensor="hivetemp" data-result="#test-temp"><i class="fa fa-refresh"></i> Test Sensor</button>
            <span id="test-temp" style="margin-left:10px"></span>
        </div>
    </div>
</div>

<?php /* --- WEIGHT --- */
$wt_on = ($result['ENABLE_HIVE_WEIGHT_CHK'] == 'yes');
$wt_pcls = $wt_on ? status_panel_class($sh['weight']['status']) : 'panel-default';
?>
<div class="panel <?= $wt_pcls ?>">
    <div class="panel-heading">
        <div class="row">
            <div class="col-xs-8"><i class="fa fa-balance-scale fa-fw"></i> <strong>Weight Scale</strong>
            <?php if ($wt_on): ?><span class="label <?= status_label_class($sh['weight']['status']) ?>" style="margin-left:8px"><?= h($sh['weight']['status_label']) ?></span><?php endif; ?>
            </div>
            <div class="col-xs-4 text-right">
                <select name="ENABLE_HIVE_WEIGHT_CHK" id="enable-weight" class="form-control input-sm enable-toggle" data-target="#body-weight" style="display:inline-block;width:auto">
                    <option value="yes"<?= sel($result['ENABLE_HIVE_WEIGHT_CHK'], 'yes') ?>>Enabled</option>
                    <option value="no"<?= sel($result['ENABLE_HIVE_WEIGHT_CHK'], 'no') ?>>Disabled</option>
                </select>
            </div>
        </div>
    </div>
    <div id="body-weight" class="panel-body collapse<?= $wt_on ? ' in' : '' ?>">
        <div class="form-group">
            <label>Scale Type</label><br>
<?php
$stypes = ['phidget1046'=>'Phidget 1046','hx711'=>'HX711','cpw200plus'=>'CPW 200 Plus','None'=>'None'];
foreach ($stypes as $sv => $sl) {
    echo '<label class="radio-inline"><input type="radio" name="SCALETYPE" value="'.$sv.'"'.chk($result['SCALETYPE'],$sv).' class="scale-type-radio"> '.$sl.'</label>';
}
?>
        </div>
        <hr style="margin:10px 0">
        <label>Calibration</label>
        <div class="row">
            <div class="col-sm-3"><label class="small text-muted">Offset / Intercept</label><input type="text" name="HIVE_WEIGHT_INTERCEPT" class="form-control input-sm" value="<?= h($result['HIVE_WEIGHT_INTERCEPT']) ?>"></div>
            <div class="col-sm-3"><label class="small text-muted">Slope</label><input type="text" name="HIVE_WEIGHT_SLOPE" class="form-control input-sm" value="<?= h($result['HIVE_WEIGHT_SLOPE']) ?>"></div>
        </div>
        <div style="margin-top:12px">
            <span class="scale-option" data-types="hx711" style="<?= $result['SCALETYPE'] == 'hx711' ? '' : 'display:none' ?>">
                <a class="btn btn-primary btn-sm" href="/admin/setup-wizard.php?step=3"><i class="fa fa-magic"></i> Calibrate Wizard</a>
            </span>
            <button type="button" class="btn btn-info btn-sm test-sensor" data-sensor="hiveweight" data-result="#test-weight"><i class="fa fa-refresh"></i> Test Sensor</button>
            <span id="test-weight" style="margin-left:10px"></span>
        </div>
    </div>
</div>

<?php /* --- WEIGHT DRIFT COMPENSATION --- */ ?>
<div id="panel-drift" class="panel panel-default" style="<?= $wt_on ? '' : 'display:none' ?>">
    <div class="panel-heading">
        <div class="row">
            <div class="col-xs-8"><i class="fa fa-sliders fa-fw"></i> <strong>Weight Drift Compensation</strong></div>
            <div class="col-xs-4 text-right">
                <select name="WEIGHT_COMPENSATION_ENABLED" class="form-control input-sm enable-toggle" data-target="#body-drift" style="display:inline-block;width:auto">
                    <option value="yes"<?= sel($result['WEIGHT_COMPENSATION_ENABLED'] ?? 'no', 'yes') ?>>Enabled</option>
                    <option value="no"<?= sel($result['WEIGHT_COMPENSATION_ENABLED'] ?? 'no', 'no') ?>>Disabled</option>
                </select>
            </div>
        </div>
    </div>
    <div id="body-drift" class="panel-body collapse<?= ($result['WEIGHT_COMPENSATION_ENABLED'] ?? 'no') == 'yes' ? ' in' : '' ?>">
<?php
$last_cal = $result['WEIGHT_LAST_CALIBRATED'] ?? null;
if ($last_cal) {
    $r2 = $result['WEIGHT_CALIBRATION_R2'] ?? null;
    $tc = $result['WEIGHT_TEMP_COEFF'] ?? 0;
    $hc = $result['WEIGHT_HUMIDITY_COEFF'] ?? 0;
    echo '<p><i class="fa fa-check-circle" style="color:green"></i> <strong>Auto-calibrated:</strong> ' . h($last_cal) . '</p>';
    echo '<p>R&sup2;=' . number_format((float)$r2, 4) . ' &bull; Temp: ' . number_format((float)$tc, 6) . ' lbs/&deg;F &bull; Humidity: ' . number_format((float)$hc, 6) . ' lbs/%RH</p>';
} else {
    echo '<p><i class="fa fa-clock-o" style="color:#e6b800"></i> <strong>Waiting for data.</strong> Auto-calibration runs weekly at 5am using nighttime readings. Needs 50+ samples over 14 days with 5&deg;F+ temperature variation.</p>';
}
?>
    </div>
</div>

<?php /* --- LIGHT --- */
$lux_on = ($result['ENABLE_LUX'] == 'yes');
$lux_pcls = $lux_on ? status_panel_class($sh['light']['status']) : 'panel-default';
?>
<div class="panel <?= $lux_pcls ?>">
    <div class="panel-heading">
        <div class="row">
            <div class="col-xs-8"><i class="fa fa-sun-o fa-fw"></i> <strong>Light Source (Lux)</strong>
            <?php if ($lux_on): ?><span class="label <?= status_label_class($sh['light']['status']) ?>" style="margin-left:8px"><?= h($sh['light']['status_label']) ?></span><?php endif; ?>
            </div>
            <div class="col-xs-4 text-right">
                <select name="ENABLE_LUX" class="form-control input-sm enable-toggle" data-target="#body-lux" style="display:inline-block;width:auto">
                    <option value="yes"<?= sel($result['ENABLE_LUX'], 'yes') ?>>Enabled</option>
                    <option value="no"<?= sel($result['ENABLE_LUX'], 'no') ?>>Disabled</option>
                </select>
            </div>
        </div>
    </div>
    <div id="body-lux" class="panel-body collapse<?= $lux_on ? ' in' : '' ?>">
        <div class="form-group">
            <label>Source</label><br>
            <label class="radio-inline"><input type="radio" name="LUX_SOURCE" value="tsl2591"<?= chk($result['LUX_SOURCE'], 'tsl2591') ?>> TSL 2591</label>
            <label class="radio-inline"><input type="radio" name="LUX_SOURCE" value="tsl2561"<?= chk($result['LUX_SOURCE'], 'tsl2561') ?>> TSL 2561</label>
            <label class="radio-inline"><input type="radio" name="LUX_SOURCE" value="wx"<?= chk($result['LUX_SOURCE'], 'wx') ?>> Weather Station</label>
        </div>
        <hr style="margin:10px 0">
        <label>Calibration</label>
        <div class="row">
            <div class="col-sm-3"><label class="small text-muted">Offset</label><input type="text" name="HIVE_LUX_INTERCEPT" class="form-control input-sm" value="<?= h($result['HIVE_LUX_INTERCEPT']) ?>"></div>
            <div class="col-sm-3"><label class="small text-muted">Slope</label><input type="text" name="HIVE_LUX_SLOPE" class="form-control input-sm" value="<?= h($result['HIVE_LUX_SLOPE']) ?>"></div>
        </div>
        <div style="margin-top:12px">
            <button type="button" class="btn btn-info btn-sm test-sensor" data-sensor="hivelux" data-result="#test-lux"><i class="fa fa-refresh"></i> Test Sensor</button>
            <span id="test-lux" style="margin-left:10px"></span>
        </div>
    </div>
</div>

<?php /* --- BEE COUNTER --- */
$bee_on = ($result['ENABLE_BEECOUNTER'] == 'yes');
$bee_pcls = $bee_on ? status_panel_class($sh['beecount']['status']) : 'panel-default';
?>
<div class="panel <?= $bee_pcls ?>">
    <div class="panel-heading">
        <div class="row">
            <div class="col-xs-8"><i class="fa fa-bug fa-fw"></i> <strong>Bee Counter</strong>
            <?php if ($bee_on): ?><span class="label <?= status_label_class($sh['beecount']['status']) ?>" style="margin-left:8px"><?= h($sh['beecount']['status_label']) ?></span><?php endif; ?>
            </div>
            <div class="col-xs-4 text-right">
                <select name="ENABLE_BEECOUNTER" class="form-control input-sm enable-toggle" data-target="#body-bee" style="display:inline-block;width:auto">
                    <option value="yes"<?= sel($result['ENABLE_BEECOUNTER'], 'yes') ?>>Enabled</option>
                    <option value="no"<?= sel($result['ENABLE_BEECOUNTER'], 'no') ?>>Disabled</option>
                </select>
            </div>
        </div>
    </div>
    <div id="body-bee" class="panel-body collapse<?= $bee_on ? ' in' : '' ?>">
        <div class="form-group">
            <label>Counter Type</label><br>
            <label class="radio-inline"><input type="radio" name="COUNTERTYPE" value="PICAMERA"<?= chk($result['COUNTERTYPE'], 'PICAMERA') ?>> Pi Camera</label>
            <label class="radio-inline"><input type="radio" name="COUNTERTYPE" value="GATES"<?= chk($result['COUNTERTYPE'], 'GATES') ?>> Gates</label>
        </div>
    </div>
</div>

<?php /* --- CAMERA --- */ ?>
<div class="panel panel-default">
    <div class="panel-heading">
        <div class="row">
            <div class="col-xs-8"><i class="fa fa-video-camera fa-fw"></i> <strong>Camera</strong></div>
            <div class="col-xs-4 text-right">
                <select name="ENABLE_HIVE_CAMERA" class="form-control input-sm enable-toggle" data-target="#body-camera" style="display:inline-block;width:auto">
                    <option value="yes"<?= sel($result['ENABLE_HIVE_CAMERA'], 'yes') ?>>Enabled</option>
                    <option value="no"<?= sel($result['ENABLE_HIVE_CAMERA'], 'no') ?>>Disabled</option>
                </select>
            </div>
        </div>
    </div>
    <div id="body-camera" class="panel-body collapse<?= $result['ENABLE_HIVE_CAMERA'] == 'yes' ? ' in' : '' ?>">
        <div class="form-group">
            <label>Camera Type</label><br>
            <label class="radio-inline"><input type="radio" name="CAMERATYPE" value="PI"<?= chk($result['CAMERATYPE'], 'PI') ?>> Pi Camera</label>
            <label class="radio-inline"><input type="radio" name="CAMERATYPE" value="USB"<?= chk($result['CAMERATYPE'], 'USB') ?>> USB</label>
        </div>
    </div>
</div>

</div><!-- /tab-hive -->

<!-- ========== TAB: WEATHER ========== -->
<div id="tab-weather" class="tab-pane fade<?= $active_tab === 'weather' ? ' in active' : '' ?>">

<div class="panel <?= status_panel_class($sh['weather']['status']) ?>">
    <div class="panel-heading">
        <i class="fa fa-cloud fa-fw"></i> <strong>Primary Weather Source</strong>
        <span class="label <?= status_label_class($sh['weather']['status']) ?>" style="margin-left:8px"><?= h($sh['weather']['status_label']) ?></span>
    </div>
    <div class="panel-body">
        <div class="form-group">
            <label>Weather Source</label>
            <select name="WEATHER_LEVEL" id="weather-source" class="form-control" style="max-width:350px">
                <optgroup label="Cloud APIs (Free)">
                    <option value="openmeteo"<?= sel($result['WEATHER_LEVEL'], 'openmeteo') ?>>Open-Meteo (No Key)</option>
                    <option value="nws"<?= sel($result['WEATHER_LEVEL'], 'nws') ?>>NWS weather.gov (US Only)</option>
                </optgroup>
                <optgroup label="Cloud APIs (Key Required)">
                    <option value="openweathermap"<?= sel($result['WEATHER_LEVEL'], 'openweathermap') ?>>OpenWeatherMap</option>
                    <option value="weatherapi"<?= sel($result['WEATHER_LEVEL'], 'weatherapi') ?>>WeatherAPI.com</option>
                    <option value="visualcrossing"<?= sel($result['WEATHER_LEVEL'], 'visualcrossing') ?>>Visual Crossing</option>
                    <option value="pirateweather"<?= sel($result['WEATHER_LEVEL'], 'pirateweather') ?>>Pirate Weather</option>
                </optgroup>
                <optgroup label="Personal Weather Stations">
                    <option value="ambientwx"<?= sel($result['WEATHER_LEVEL'], 'ambientwx') ?>>AmbientWeather.net</option>
                    <option value="hive"<?= sel($result['WEATHER_LEVEL'], 'hive') ?>>WX Underground</option>
                    <option value="wf_tempest_local"<?= sel($result['WEATHER_LEVEL'], 'wf_tempest_local') ?>>WF Tempest UDP</option>
                </optgroup>
                <optgroup label="Local Hardware">
                    <option value="localws"<?= sel($result['WEATHER_LEVEL'], 'localws') ?>>Local Weather Station</option>
                    <option value="localsensors"<?= sel($result['WEATHER_LEVEL'], 'localsensors') ?>>Local Hive Sensors</option>
                </optgroup>
            </select>
        </div>
        <hr>

        <!-- Open-Meteo -->
        <div class="wx-source-detail" data-source="openmeteo" style="<?= $result['WEATHER_LEVEL'] == 'openmeteo' ? '' : 'display:none' ?>">
            <p>Location: <strong><?= $wx_lat ?>, <?= $wx_lon ?></strong></p>
            <p class="help-block">Free &mdash; no API key needed. Data from ECMWF, DWD, NOAA. Change location in <a href="hiveconfig.php">Hive Config</a>.</p>
        </div>

        <!-- NWS -->
        <div class="wx-source-detail" data-source="nws" style="<?= $result['WEATHER_LEVEL'] == 'nws' ? '' : 'display:none' ?>">
            <p>Location: <strong><?= $wx_lat ?>, <?= $wx_lon ?></strong></p>
            <p class="help-block">Free &mdash; no API key needed. <strong>US only.</strong> Change in <a href="hiveconfig.php">Hive Config</a>.</p>
        </div>

        <!-- OpenWeatherMap -->
        <div class="wx-source-detail" data-source="openweathermap" style="<?= $result['WEATHER_LEVEL'] == 'openweathermap' ? '' : 'display:none' ?>">
            <div class="form-group"><label>API Key</label>
                <input type="text" name="KEY_OPENWEATHERMAP" class="form-control" style="max-width:400px" value="<?= h($result['KEY_OPENWEATHERMAP'] ?? $result['KEY'] ?? '') ?>">
                <p class="help-block">Free: 1,000 calls/day. <a href="https://openweathermap.org/api" target="_blank">Get a key</a>. Location: <?= $wx_lat ?>, <?= $wx_lon ?></p>
            </div>
        </div>

        <!-- WeatherAPI -->
        <div class="wx-source-detail" data-source="weatherapi" style="<?= $result['WEATHER_LEVEL'] == 'weatherapi' ? '' : 'display:none' ?>">
            <div class="form-group"><label>API Key</label>
                <input type="text" name="KEY_WEATHERAPI" class="form-control" style="max-width:400px" value="<?= h($result['KEY_WEATHERAPI'] ?? $result['KEY'] ?? '') ?>">
                <p class="help-block">Free: 1M calls/month. <a href="https://www.weatherapi.com/signup.aspx" target="_blank">Get a key</a>. Location: <?= $wx_lat ?>, <?= $wx_lon ?></p>
            </div>
        </div>

        <!-- Visual Crossing -->
        <div class="wx-source-detail" data-source="visualcrossing" style="<?= $result['WEATHER_LEVEL'] == 'visualcrossing' ? '' : 'display:none' ?>">
            <div class="form-group"><label>API Key</label>
                <input type="text" name="KEY_VISUALCROSSING" class="form-control" style="max-width:400px" value="<?= h($result['KEY_VISUALCROSSING'] ?? $result['KEY'] ?? '') ?>">
                <p class="help-block">Free: 1,000 calls/day. Includes solar radiation. <a href="https://www.visualcrossing.com/sign-up" target="_blank">Get a key</a>. Location: <?= $wx_lat ?>, <?= $wx_lon ?></p>
            </div>
        </div>

        <!-- Pirate Weather -->
        <div class="wx-source-detail" data-source="pirateweather" style="<?= $result['WEATHER_LEVEL'] == 'pirateweather' ? '' : 'display:none' ?>">
            <div class="form-group"><label>API Key</label>
                <input type="text" name="KEY_PIRATEWEATHER" class="form-control" style="max-width:400px" value="<?= h($result['KEY_PIRATEWEATHER'] ?? $result['KEY'] ?? '') ?>">
                <p class="help-block">Free: 20,000 calls/day. <a href="https://pirateweather.net" target="_blank">Get a key</a>. Location: <?= $wx_lat ?>, <?= $wx_lon ?></p>
            </div>
        </div>

        <!-- WX Underground -->
        <div class="wx-source-detail" data-source="hive" style="<?= $result['WEATHER_LEVEL'] == 'hive' ? '' : 'display:none' ?>">
            <div class="form-group"><label>API Key</label>
                <input type="text" name="WXAPIKEY" class="form-control" style="max-width:400px" value="<?= h($result['WXAPIKEY'] ?? '') ?>">
                <p class="help-block">PWS owners get a free key. <a href="https://www.wunderground.com/member/api-keys" target="_blank">Get yours</a></p>
            </div>
        </div>

        <!-- AmbientWeather -->
        <div class="wx-source-detail" data-source="ambientwx" style="<?= $result['WEATHER_LEVEL'] == 'ambientwx' ? '' : 'display:none' ?>">
            <div class="form-group"><label>API Key</label>
                <input type="text" name="KEY" class="form-control" style="max-width:400px" value="<?= h($result['KEY']) ?>">
            </div>
        </div>

        <!-- Shared Station ID -->
        <div class="wx-source-detail" data-source="hive,ambientwx,wf_tempest_local" style="<?= in_array($result['WEATHER_LEVEL'], ['hive','ambientwx','wf_tempest_local']) ? '' : 'display:none' ?>">
            <div class="form-group"><label id="lbl-wxstation"><?php
                $sl = ['hive'=>'Station ID','ambientwx'=>'Station MAC','wf_tempest_local'=>'Station Serial'];
                echo $sl[$result['WEATHER_LEVEL']] ?? 'Station ID';
            ?></label>
                <input type="text" name="WXSTATION" class="form-control" style="max-width:300px" value="<?= h($result['WXSTATION']) ?>">
            </div>
        </div>

        <!-- Local Weather Station -->
        <div class="wx-source-detail" data-source="localws" style="<?= $result['WEATHER_LEVEL'] == 'localws' ? '' : 'display:none' ?>">
            <div class="form-group"><label>Station Type</label><br>
                <label class="radio-inline"><input type="radio" name="local_wx_type" value="WS1400ip"<?= chk($result['local_wx_type'], 'WS1400ip') ?>> WS1400ip</label>
                <label class="radio-inline"><input type="radio" name="local_wx_type" value="ourweather"<?= chk($result['local_wx_type'], 'ourweather') ?>> OurWeather</label>
            </div>
            <div class="form-group"><label>Station IP Address</label>
                <input type="text" name="local_wx_url" class="form-control" style="max-width:250px" value="<?= h($result['local_wx_url']) ?>" placeholder="192.168.1.x">
            </div>
        </div>

        <!-- Local Sensors -->
        <div class="wx-source-detail" data-source="localsensors" style="<?= $result['WEATHER_LEVEL'] == 'localsensors' ? '' : 'display:none' ?>">
            <div class="form-group"><label>Sensor Type</label><br>
<?php
$wst = ['temperhum'=>'TemperHum','dht21'=>'DHT21','dht22'=>'DHT22','sht31d'=>'SHT31-D','bme280'=>'BME280','bme680'=>'BME680'];
foreach ($wst as $wv => $wl) {
    echo '<label class="radio-inline"><input type="radio" name="WXTEMPTYPE" value="'.$wv.'"'.chk($result['WXTEMPTYPE'],$wv).' class="wxsensor-type-radio"> '.$wl.'</label>';
}
?>
            </div>
            <div class="wxsensor-option" data-types="temperhum" style="<?= $result['WXTEMPTYPE'] == 'temperhum' ? '' : 'display:none' ?>">
                <div class="form-group"><label>Device Path</label>
                    <input type="text" name="WX_TEMPER_DEVICE" class="form-control" style="max-width:300px" value="<?= h($result['WX_TEMPER_DEVICE']) ?>">
                </div>
            </div>
            <div class="wxsensor-option" data-types="dht21,dht22" style="<?= in_array($result['WXTEMPTYPE'], ['dht21','dht22']) ? '' : 'display:none' ?>">
                <div class="form-group"><label>GPIO Pin</label>
                    <input type="text" name="WX_TEMP_GPIO" class="form-control" style="max-width:80px" value="<?= h($result['WX_TEMP_GPIO']) ?>">
                </div>
            </div>
        </div>

    </div>
</div>

<!-- Fallback Chain -->
<div class="panel panel-default">
    <div class="panel-heading"><i class="fa fa-random fa-fw"></i> <strong>Weather Fallback Chain</strong></div>
    <div class="panel-body">
        <div class="row">
            <div class="col-sm-6">
                <div class="form-group"><label>Fallback 1</label>
                    <select name="WEATHER_FALLBACK" id="weather-fb1" class="form-control">
                        <option value=""<?= sel($result['WEATHER_FALLBACK'] ?? '', '') ?>>None (No Fallback)</option>
                        <optgroup label="Free">
                            <option value="openmeteo"<?= sel($result['WEATHER_FALLBACK'] ?? '', 'openmeteo') ?>>Open-Meteo</option>
                            <option value="nws"<?= sel($result['WEATHER_FALLBACK'] ?? '', 'nws') ?>>NWS (US Only)</option>
                        </optgroup>
                        <optgroup label="Key Required">
                            <option value="openweathermap"<?= sel($result['WEATHER_FALLBACK'] ?? '', 'openweathermap') ?>>OpenWeatherMap</option>
                            <option value="weatherapi"<?= sel($result['WEATHER_FALLBACK'] ?? '', 'weatherapi') ?>>WeatherAPI.com</option>
                            <option value="visualcrossing"<?= sel($result['WEATHER_FALLBACK'] ?? '', 'visualcrossing') ?>>Visual Crossing</option>
                            <option value="pirateweather"<?= sel($result['WEATHER_FALLBACK'] ?? '', 'pirateweather') ?>>Pirate Weather</option>
                        </optgroup>
                    </select>
                    <div id="fb1-info" style="margin-top:5px"></div>
                </div>
            </div>
            <div class="col-sm-6">
                <div class="form-group"><label>Fallback 2</label>
                    <select name="WEATHER_FALLBACK_2" id="weather-fb2" class="form-control">
                        <option value=""<?= sel($result['WEATHER_FALLBACK_2'] ?? '', '') ?>>None</option>
                        <optgroup label="Free">
                            <option value="openmeteo"<?= sel($result['WEATHER_FALLBACK_2'] ?? '', 'openmeteo') ?>>Open-Meteo</option>
                            <option value="nws"<?= sel($result['WEATHER_FALLBACK_2'] ?? '', 'nws') ?>>NWS (US Only)</option>
                        </optgroup>
                        <optgroup label="Key Required">
                            <option value="openweathermap"<?= sel($result['WEATHER_FALLBACK_2'] ?? '', 'openweathermap') ?>>OpenWeatherMap</option>
                            <option value="weatherapi"<?= sel($result['WEATHER_FALLBACK_2'] ?? '', 'weatherapi') ?>>WeatherAPI.com</option>
                            <option value="visualcrossing"<?= sel($result['WEATHER_FALLBACK_2'] ?? '', 'visualcrossing') ?>>Visual Crossing</option>
                            <option value="pirateweather"<?= sel($result['WEATHER_FALLBACK_2'] ?? '', 'pirateweather') ?>>Pirate Weather</option>
                        </optgroup>
                    </select>
                    <div id="fb2-info" style="margin-top:5px"></div>
                </div>
            </div>
        </div>
        <p class="help-block">If the primary fails or returns stale data, the next source in the chain is tried. Free providers (Open-Meteo, NWS) make good fallbacks.</p>
    </div>
</div>

<!-- Staleness & Health -->
<div class="panel panel-default">
    <div class="panel-heading"><i class="fa fa-heartbeat fa-fw"></i> <strong>Staleness &amp; Health</strong></div>
    <div class="panel-body">
        <div class="form-group"><label>Stale Data Threshold</label>
            <div class="input-group" style="max-width:200px">
                <input type="number" name="WX_MAX_STALE_MINUTES" class="form-control" min="30" max="360" value="<?= h($result['WX_MAX_STALE_MINUTES'] ?? '120') ?>">
                <span class="input-group-addon">minutes</span>
            </div>
            <p class="help-block">Reject observations older than this and try the next source. Default: 120.</p>
        </div>
        <label>Provider Health (24h)</label><br>
<?php
if (empty($wx_health_rows)) {
    echo '<span class="text-muted">No health data yet.</span>';
} else {
    foreach ($wx_health_rows as $hr) {
        $r = intval($hr['rate']);
        $hcls = $r >= 95 ? 'success' : ($r >= 80 ? 'warning' : 'danger');
        echo '<span class="label label-' . $hcls . '">' . h($hr['provider']) . ': ' . $r . '%</span> ';
    }
}
?>
        <br><br><a href="/admin/sensor_health.php"><i class="fa fa-bar-chart"></i> Full Sensor Health Dashboard</a>
    </div>
</div>

</div><!-- /tab-weather -->

<!-- ========== TAB: ENVIRONMENT ========== -->
<div id="tab-environment" class="tab-pane fade<?= $active_tab === 'environment' ? ' in active' : '' ?>">

<?php /* --- AIR QUALITY --- */
$air_on = ($result['ENABLE_AIR'] == 'yes');
$air_pcls = $air_on ? status_panel_class($sh['air']['status']) : 'panel-default';
?>
<div class="panel <?= $air_pcls ?>">
    <div class="panel-heading">
        <div class="row">
            <div class="col-xs-8"><i class="fa fa-industry fa-fw"></i> <strong>Air Quality (PurpleAir)</strong>
            <?php if ($air_on): ?><span class="label <?= status_label_class($sh['air']['status']) ?>" style="margin-left:8px"><?= h($sh['air']['status_label']) ?></span><?php endif; ?>
            </div>
            <div class="col-xs-4 text-right">
                <select name="ENABLE_AIR" class="form-control input-sm enable-toggle" data-target="#body-air" style="display:inline-block;width:auto">
                    <option value="yes"<?= sel($result['ENABLE_AIR'], 'yes') ?>>Enabled</option>
                    <option value="no"<?= sel($result['ENABLE_AIR'], 'no') ?>>Disabled</option>
                </select>
            </div>
        </div>
    </div>
    <div id="body-air" class="panel-body collapse<?= $air_on ? ' in' : '' ?>">
        <div class="form-group"><label>Source Type</label><br>
            <label class="radio-inline"><input type="radio" name="AIR_TYPE" value="purpleapi"<?= (in_array($result['AIR_TYPE'], ['purpleapi','purple'])) ? ' checked' : '' ?> class="air-type-radio"> PurpleAir - API</label>
            <label class="radio-inline"><input type="radio" name="AIR_TYPE" value="purplelocal"<?= chk($result['AIR_TYPE'], 'purplelocal') ?> class="air-type-radio"> PurpleAir - Local</label>
        </div>
        <div class="form-group"><label>Station ID</label>
            <input type="text" name="AIR_ID" class="form-control" style="max-width:200px" value="<?= h($result['AIR_ID']) ?>">
            <p class="help-block">Find at <a href="https://www.purpleair.com/map" target="_blank">purpleair.com/map</a></p>
        </div>
        <div class="air-option" data-types="purpleapi,purple" style="<?= in_array($result['AIR_TYPE'], ['purpleapi','purple']) ? '' : 'display:none' ?>">
            <div class="form-group"><label>API Read Key</label>
                <input type="text" name="AIR_API" class="form-control" style="max-width:400px" value="<?= h($result['AIR_API'] ?? '') ?>">
            </div>
        </div>
        <div class="air-option" data-types="purplelocal" style="<?= $result['AIR_TYPE'] == 'purplelocal' ? '' : 'display:none' ?>">
            <div class="form-group"><label>Local URL</label>
                <input type="text" name="AIR_LOCAL_URL" class="form-control" style="max-width:350px" value="<?= h($result['AIR_LOCAL_URL'] ?? '') ?>" placeholder="http://192.168.1.x/json">
            </div>
        </div>
    </div>
</div>

<?php /* --- EPA AIRNOW --- */
$epa_on = (($result['ENABLE_AIRNOW'] ?? 'no') == 'yes');
$epa_pcls = $epa_on ? status_panel_class($sh['epa']['status']) : 'panel-default';
?>
<div class="panel <?= $epa_pcls ?>">
    <div class="panel-heading">
        <div class="row">
            <div class="col-xs-8"><i class="fa fa-leaf fa-fw"></i> <strong>EPA AirNow</strong> <small class="text-muted">Ozone &amp; NO2</small>
            <?php if ($epa_on): ?><span class="label <?= status_label_class($sh['epa']['status']) ?>" style="margin-left:8px"><?= h($sh['epa']['status_label']) ?></span><?php endif; ?>
            </div>
            <div class="col-xs-4 text-right">
                <select name="ENABLE_AIRNOW" class="form-control input-sm enable-toggle" data-target="#body-epa" style="display:inline-block;width:auto">
                    <option value="yes"<?= sel($result['ENABLE_AIRNOW'] ?? 'no', 'yes') ?>>Enabled</option>
                    <option value="no"<?= sel($result['ENABLE_AIRNOW'] ?? 'no', 'no') ?>>Disabled</option>
                </select>
            </div>
        </div>
    </div>
    <div id="body-epa" class="panel-body collapse<?= $epa_on ? ' in' : '' ?>">
        <div class="form-group"><label>AirNow API Key</label>
            <input type="text" name="KEY_AIRNOW" class="form-control" style="max-width:400px" value="<?= h($result['KEY_AIRNOW'] ?? '') ?>">
            <p class="help-block"><a href="https://docs.airnowapi.org/account/request/" target="_blank">Get a free key</a>. Free: 500 requests/day.</p>
        </div>
        <div class="form-group"><label>Search Radius</label>
            <div class="input-group" style="max-width:150px">
                <input type="number" name="AIRNOW_DISTANCE" class="form-control" min="5" max="100" value="<?= h($result['AIRNOW_DISTANCE'] ?? '25') ?>">
                <span class="input-group-addon">miles</span>
            </div>
        </div>
    </div>
</div>

<?php /* --- POLLEN --- */
$pollen_on = (($result['ENABLE_POLLEN'] ?? 'yes') == 'yes');
$pollen_pcls = $pollen_on ? status_panel_class($sh['pollen']['status']) : 'panel-default';
?>
<div class="panel <?= $pollen_pcls ?>">
    <div class="panel-heading">
        <div class="row">
            <div class="col-xs-8"><i class="fa fa-pagelines fa-fw"></i> <strong>Pollen Data</strong>
            <?php if ($pollen_on): ?><span class="label <?= status_label_class($sh['pollen']['status']) ?>" style="margin-left:8px"><?= h($sh['pollen']['status_label']) ?></span><?php endif; ?>
            <small class="text-muted" style="margin-left:8px">Primary: Pollen.com (US, free)</small>
            </div>
            <div class="col-xs-4 text-right">
                <select name="ENABLE_POLLEN" class="form-control input-sm enable-toggle" data-target="#body-pollen" style="display:inline-block;width:auto">
                    <option value="yes"<?= sel($result['ENABLE_POLLEN'] ?? 'yes', 'yes') ?>>Enabled</option>
                    <option value="no"<?= sel($result['ENABLE_POLLEN'] ?? 'yes', 'no') ?>>Disabled</option>
                </select>
            </div>
        </div>
    </div>
    <div id="body-pollen" class="panel-body collapse<?= $pollen_on ? ' in' : '' ?>">
        <p>Optional fallback providers &mdash; enter API keys for additional data sources.</p>
        <div class="row">
            <div class="col-sm-6">
                <div class="form-group"><label>Tomorrow.io API Key</label>
                    <input type="text" name="KEY_TOMORROW" class="form-control" value="<?= h($result['KEY_TOMORROW'] ?? '') ?>" placeholder="Optional">
                    <p class="help-block"><a href="https://www.tomorrow.io/weather-api/" target="_blank">tomorrow.io</a></p>
                </div>
            </div>
            <div class="col-sm-6">
                <div class="form-group"><label>Ambee API Key</label>
                    <input type="text" name="KEY_AMBEE" class="form-control" value="<?= h($result['KEY_AMBEE'] ?? '') ?>" placeholder="Optional">
                    <p class="help-block"><a href="https://www.getambee.com/" target="_blank">getambee.com</a></p>
                </div>
            </div>
        </div>
    </div>
</div>

</div><!-- /tab-environment -->

            </div><!-- /tab-content -->

            <div style="margin:15px 0 30px">
                <button type="submit" class="btn btn-primary btn-lg"><i class="fa fa-save"></i> Save Changes</button>
                <span id="unsaved-warning" class="text-warning" style="margin-left:15px;display:none"><i class="fa fa-exclamation-triangle"></i> You have unsaved changes</span>
            </div>

        </form>

    </div>
    </div>
    </div>

    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>
    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>
    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>
    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

    <?PHP
    $verfile = __DIR__ . '/hiveconfig.ver';
    $FILEVERSION = file_exists($verfile) ? file_get_contents($verfile) : 0;
    if ($result['VERSION'] > $FILEVERSION) {
        $foo = shell_exec("sudo /home/HiveControl/scripts/data/hiveconfig.sh");
    }
    ?>

    <script>
    $(document).ready(function() {
        // Tab persistence
        var hash = window.location.hash;
        if (hash) {
            var tabLink = $('#config-tabs a[href="' + hash + '"]');
            if (tabLink.length) tabLink.tab('show');
        }
        $('#config-tabs a').on('shown.bs.tab', function(e) {
            var id = $(e.target).attr('href').replace('#tab-', '');
            $('#_active_tab').val(id);
            history.replaceState(null, null, $(e.target).attr('href'));
        });

        // Enable/Disable → collapse panel body
        $('.enable-toggle').on('change', function() {
            var t = $(this).data('target');
            if ($(this).val() === 'yes') { $(t).collapse('show'); }
            else { $(t).collapse('hide'); }
        });

        // Weight enable → drift panel visibility
        $('#enable-weight').on('change', function() {
            $('#panel-drift').toggle($(this).val() === 'yes');
        });

        // Generic radio show/hide helper
        function radioToggle(radioClass, optionClass) {
            $('input.' + radioClass).on('change', function() {
                var val = $(this).val();
                $('.' + optionClass).each(function() {
                    var types = $(this).data('types').split(',');
                    $(this).toggle(types.indexOf(val) >= 0);
                });
            });
        }
        radioToggle('temp-type-radio', 'temp-option');
        radioToggle('scale-type-radio', 'scale-option');
        radioToggle('wxsensor-type-radio', 'wxsensor-option');
        radioToggle('air-type-radio', 'air-option');

        // Weather source → show/hide details
        var stationLabels = {'hive':'Station ID','ambientwx':'Station MAC','wf_tempest_local':'Station Serial'};
        $('#weather-source').on('change', function() {
            var src = $(this).val();
            $('.wx-source-detail').each(function() {
                var sources = $(this).data('source').split(',');
                $(this).toggle(sources.indexOf(src) >= 0);
            });
            if (stationLabels[src]) $('#lbl-wxstation').text(stationLabels[src]);
            updateFallbackInfo();
        });

        // Fallback warnings
        function updateFallbackInfo() {
            var p = $('#weather-source').val();
            var f1 = $('#weather-fb1').val();
            var f2 = $('#weather-fb2').val();
            if (f1 && f1 === p) {
                $('#fb1-info').html('<span class="text-danger"><strong>Warning:</strong> Same as primary.</span>');
            } else if (f1) {
                $('#fb1-info').html('<small class="text-muted">If ' + p + ' fails, ' + f1 + ' will be used.</small>');
            } else {
                $('#fb1-info').html('<small class="text-muted">Recommended: Set a fallback for reliability.</small>');
            }
            if (f2 && (f2 === p || f2 === f1)) {
                $('#fb2-info').html('<span class="text-danger"><strong>Warning:</strong> Duplicate in chain.</span>');
            } else if (f2) {
                $('#fb2-info').html('<small class="text-muted">Last resort if both primary and fallback 1 fail.</small>');
            } else {
                $('#fb2-info').html('<small class="text-muted">Optional third-tier source.</small>');
            }
        }
        $('#weather-fb1, #weather-fb2').on('change', updateFallbackInfo);
        updateFallbackInfo();

        // Test sensor (AJAX)
        $('.test-sensor').on('click', function() {
            var btn = $(this), sensor = btn.data('sensor'), res = $(btn.data('result'));
            btn.prop('disabled', true).find('i').addClass('fa-spin');
            res.html('<span class="text-muted">Reading...</span>');
            $.ajax({
                url: 'livevalue.php?sensor=' + sensor, cache: false, timeout: 60000,
                success: function(d) { res.html(d); },
                error: function() { res.html('<span class="text-danger">Read failed</span>'); },
                complete: function() { btn.prop('disabled', false).find('i').removeClass('fa-spin'); }
            });
        });

        // Unsaved changes
        var dirty = false;
        $('form').on('change input', 'input, select, textarea', function() {
            dirty = true; $('#unsaved-warning').show();
        });
        $('form').on('submit', function() { dirty = false; });
        $(window).on('beforeunload', function() { if (dirty) return 'You have unsaved changes.'; });

        <?php if ($save_success): ?>
        $('html, body').animate({scrollTop: 0}, 300);
        setTimeout(function() { $('#save-alert').fadeOut(3000); }, 5000);
        <?php endif; ?>
    });
    </script>

</body>
</html>
