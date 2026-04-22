<?PHP
# Version 2020052801

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
// SECURITY FIX: Include security-init for CSRF protection and security headers
include($_SERVER["DOCUMENT_ROOT"] . "/include/security-init.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

//Check input for badness
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



if ($_SERVER["REQUEST_METHOD"] == "GET") {
$sth = $conn->prepare("SELECT * FROM hiveconfig");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);

    if(isset($_GET["livevalue"])) {
        // exists
        if (empty($_GET["livevalue"])) {
            // Default to Day if not  set or empty
            $livevalue = "no";
            } else {
                $livevalue = test_input($_GET["livevalue"]);
            }
        } else {
        $livevalue = "no";
    }

}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
// Set Error Fields

    $v = new Valitron\Validator($_POST);
    $v->rule('required', ['ENABLE_HIVE_CAMERA', 'ENABLE_HIVE_WEIGHT_CHK', 'ENABLE_HIVE_TEMP_CHK', 'ENABLE_LUX'], 1)->message('{field} is required');
    $v->rule('slug', ['WXTEMPTYPE']);
    $v->rule('in', ['ENABLE_HIVE_WEIGHT', 'ENABLE_LUX', 'ENABLE_HIVE_CAMERA', 'ENABLE_HIVE_WEIGHT_CHK', 'ENABLE_HIVE_TEMP_CHK', 'ENABLE_BEECOUNTER', 'ENABLE_AIR'], ['no', 'yes']);
    $v->rule('integer', ['HIVE_TEMP_GPIO', 'HIVE_LUX_GPIO', 'HIVE_WEIGHT_GPIO', 'HIVE_TEMP_SUB'], 1)->message('{field} must be a integer');
    $v->rule('numeric', ['HIVE_WEIGHT_SLOPE', 'HIVE_WEIGHT_INTERCEPT', 'HIVE_LUX_SLOPE', 'HIVE_LUX_INTERCEPT', 'HIVE_TEMP_SLOPE', 'HIVE_TEMP_INTERCEPT', 'WX_TEMP_SLOPE','WX_TEMP_INTERCEPT','HIVE_HUMIDITY_SLOPE','HIVE_HUMIDITY_INTERCEPT','WX_HUMIDITY_SLOPE','WX_HUMIDITY_INTERCEPT','WEIGHT_TEMP_COEFF','WEIGHT_HUMIDITY_COEFF','WEIGHT_REF_TEMP','WEIGHT_REF_HUMIDITY'], 1)->message('{field} must be numeric');
    $v->rule('in', ['WEIGHT_COMPENSATION_ENABLED'], ['no', 'yes']);
    $v->rule('alphaNum', ['SCALETYPE', 'TEMPTYPE', 'LUX_SOURCE', 'COUNTERTYPE', 'CAMERATYPE', 'local_wx_type', 'AIR_ID', 'AIR_TYPE'], 1)->message('{field} must be alphaNum only');
    $v->rule('integer', ['WX_MAX_STALE_MINUTES']);
    $v->rule('min', ['WX_MAX_STALE_MINUTES'], 30);
    $v->rule('max', ['WX_MAX_STALE_MINUTES'], 360);
    $v->rule('lengthmax', ['WX_TEMP_GPIO', 'HIVE_LUX_GPIO', 'HIVE_WEIGHT_GPIO'], 2);

}


?>

<!DOCTYPE html>
<html lang="en">


    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->
    <div id="wrapper">
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Settings - Instruments</h1>
                </div>
                <!-- /.col-lg-12 -->
            </div>
            <!-- /.row -->
            <form method="POST" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]);?>">
        <?PHP
        if ($_SERVER["REQUEST_METHOD"] == "POST") {
        
if($v->validate()) {

  // Get Data, just in case we didn't submit, so we can save values
    $sth = $conn->prepare("SELECT * FROM hiveconfig");
    $sth->execute();
    $old = $sth->fetch(PDO::FETCH_ASSOC);
    
    
// Make sure no bad stuff got through our filters
// Probably not needed, but doesn't hurt

    #Hive Temp
    $ENABLE_HIVE_TEMP_CHK = test_input($_POST["ENABLE_HIVE_TEMP_CHK"]);    
    if ( $ENABLE_HIVE_TEMP_CHK == "no" ) {
        # Set values to whatever they were before
        $TEMPTYPE = $old['TEMPTYPE'];
        $HIVEDEVICE = $old['HIVEDEVICE'];
        $HIVE_TEMP_SUB = $old['HIVE_TEMP_SUB'];
        $HIVE_TEMP_GPIO = $old['HIVE_TEMP_GPIO'];
        $HIVE_TEMP_SLOPE = $old['HIVE_TEMP_SLOPE'];
        $HIVE_TEMP_INTERCEPT = $old['HIVE_TEMP_INTERCEPT'];
        $HIVE_HUMIDITY_SLOPE = $old['HIVE_HUMIDITY_SLOPE'];
        $HIVE_HUMIDITY_INTERCEPT = $old['HIVE_HUMIDITY_INTERCEPT'];
        
        echo '<input type="hidden" name="TEMPTYPE" value="' . $TEMPTYPE . '">
        <input type="hidden" name="HIVEDEVICE" value="' . $HIVEDEVICE . '">
        <input type="hidden" name="HIVE_TEMP_SUB" value="' . $HIVE_TEMP_SUB . '">
        <input type="hidden" name="HIVE_TEMP_GPIO" value="' . $HIVE_TEMP_GPIO . '">
        <input type="hidden" name="HIVE_TEMP_SLOPE" value="' . $HIVE_TEMP_SLOPE . '">
        <input type="hidden" name="HIVE_TEMP_INTERCEPT" value="' . $HIVE_TEMP_INTERCEPT . '">
        <input type="hidden" name="HIVE_HUMIDITY_SLOPE" value="' . $HIVE_HUMIDITY_SLOPE . '">
        <input type="hidden" name="HIVE_HUMIDITY_INTERCEPT" value="' . $HIVE_HUMIDITY_INTERCEPT . '">';
    } else {
        $TEMPTYPE = test_input($_POST["TEMPTYPE"]);
        $HIVEDEVICE = test_input($_POST["HIVEDEVICE"]);
        $HIVE_TEMP_SUB = test_input($_POST["HIVE_TEMP_SUB"]);
        $HIVE_TEMP_GPIO = test_input($_POST["HIVE_TEMP_GPIO"]);
        $HIVE_TEMP_SLOPE = test_input($_POST["HIVE_TEMP_SLOPE"]);
        $HIVE_TEMP_INTERCEPT = test_input($_POST["HIVE_TEMP_INTERCEPT"]);
        $HIVE_HUMIDITY_SLOPE = test_input($_POST["HIVE_HUMIDITY_SLOPE"]);
        $HIVE_HUMIDITY_INTERCEPT = test_input($_POST["HIVE_HUMIDITY_INTERCEPT"]);
    }

    #Weight Checks
    $ENABLE_HIVE_WEIGHT_CHK = test_input($_POST["ENABLE_HIVE_WEIGHT_CHK"]);
    if ( $ENABLE_HIVE_WEIGHT_CHK == "no" ) {
        # Set values to whatever they were before
        $SCALETYPE = $old['SCALETYPE'];
        $HIVE_WEIGHT_GPIO = $old['HIVE_WEIGHT_GPIO'];
        $HIVE_WEIGHT_SLOPE = $old['HIVE_WEIGHT_SLOPE'];
        $HIVE_WEIGHT_INTERCEPT = $old['HIVE_WEIGHT_INTERCEPT'];
        $WEIGHT_COMPENSATION_ENABLED = $old['WEIGHT_COMPENSATION_ENABLED'] ?? 'no';
        $WEIGHT_TEMP_COEFF = $old['WEIGHT_TEMP_COEFF'] ?? '0';
        $WEIGHT_HUMIDITY_COEFF = $old['WEIGHT_HUMIDITY_COEFF'] ?? '0';
        $WEIGHT_REF_TEMP = $old['WEIGHT_REF_TEMP'] ?? '';
        $WEIGHT_REF_HUMIDITY = $old['WEIGHT_REF_HUMIDITY'] ?? '';

        echo '<input type="hidden" name="SCALETYPE" value="' . $SCALETYPE . '">
        <input type="hidden" name="HIVE_WEIGHT_GPIO" value="' . $HIVE_WEIGHT_GPIO . '">
        <input type="hidden" name="HIVE_WEIGHT_SLOPE" value="' . $HIVE_WEIGHT_SLOPE . '">
        <input type="hidden" name="HIVE_WEIGHT_INTERCEPT" value="' . $HIVE_WEIGHT_INTERCEPT . '">
        <input type="hidden" name="WEIGHT_COMPENSATION_ENABLED" value="' . $WEIGHT_COMPENSATION_ENABLED . '">
        <input type="hidden" name="WEIGHT_TEMP_COEFF" value="' . $WEIGHT_TEMP_COEFF . '">
        <input type="hidden" name="WEIGHT_HUMIDITY_COEFF" value="' . $WEIGHT_HUMIDITY_COEFF . '">
        <input type="hidden" name="WEIGHT_REF_TEMP" value="' . $WEIGHT_REF_TEMP . '">
        <input type="hidden" name="WEIGHT_REF_HUMIDITY" value="' . $WEIGHT_REF_HUMIDITY . '">';

    } else {
        $SCALETYPE = test_input($_POST["SCALETYPE"]);
        $HIVE_WEIGHT_GPIO = test_input($_POST["HIVE_WEIGHT_GPIO"]);
        $HIVE_WEIGHT_SLOPE = test_input($_POST["HIVE_WEIGHT_SLOPE"]);
        $HIVE_WEIGHT_INTERCEPT = test_input($_POST["HIVE_WEIGHT_INTERCEPT"]);
        $WEIGHT_COMPENSATION_ENABLED = test_input($_POST["WEIGHT_COMPENSATION_ENABLED"] ?? 'no');
        $WEIGHT_TEMP_COEFF = test_input($_POST["WEIGHT_TEMP_COEFF"] ?? '0');
        $WEIGHT_HUMIDITY_COEFF = test_input($_POST["WEIGHT_HUMIDITY_COEFF"] ?? '0');
        $WEIGHT_REF_TEMP = test_input($_POST["WEIGHT_REF_TEMP"] ?? '');
        $WEIGHT_REF_HUMIDITY = test_input($_POST["WEIGHT_REF_HUMIDITY"] ?? '');
    }

    
    #LUX 
    $ENABLE_LUX = test_input($_POST["ENABLE_LUX"]);
    if ( $ENABLE_LUX == "no" ) {
        # set values to whatever they were before
        $LUX_SOURCE = $old['LUX_SOURCE'];
        $HIVE_LUX_SLOPE = $old['HIVE_LUX_SLOPE'];
        $HIVE_LUX_INTERCEPT = $old['HIVE_LUX_INTERCEPT'];
        $HIVE_LUX_GPIO = $old['HIVE_LUX_GPIO'];

        echo '<input type="hidden" name="LUX_SOURCE" value="' . $LUX_SOURCE . '">
        <input type="hidden" name="HIVE_LUX_GPIO" value="' . $HIVE_LUX_GPIO . '">
        <input type="hidden" name="HIVE_LUX_SLOPE" value="' . $HIVE_LUX_SLOPE . '">
        <input type="hidden" name="HIVE_LUX_INTERCEPT" value="' . $HIVE_LUX_INTERCEPT . '">';
    } else {
        $LUX_SOURCE = test_input($_POST["LUX_SOURCE"]);
        $HIVE_LUX_SLOPE = test_input($_POST["HIVE_LUX_SLOPE"]);
        $HIVE_LUX_INTERCEPT = test_input($_POST["HIVE_LUX_INTERCEPT"]);
        $HIVE_LUX_GPIO = test_input($_POST["HIVE_LUX_GPIO"]);
    
    }
    
    #Camera/Counter
    $ENABLE_HIVE_CAMERA = test_input($_POST["ENABLE_HIVE_CAMERA"]);
    $ENABLE_BEECOUNTER= test_input($_POST["ENABLE_BEECOUNTER"]);
    $CAMERATYPE = test_input($_POST["CAMERATYPE"]);
    $COUNTERTYPE = test_input($_POST["COUNTERTYPE"]);
    

    #Weather
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

    #Air Quality
    $ENABLE_AIR = test_input($_POST["ENABLE_AIR"]);
    $AIR_TYPE= test_input($_POST["AIR_TYPE"]);
    $AIR_ID = test_input($_POST["AIR_ID"]);
    $AIR_API = test_input($_POST["AIR_API"] ?? '');
    $AIR_LOCAL_URL = test_input($_POST["AIR_LOCAL_URL"] ?? '');
    $ENABLE_AIRNOW = test_input($_POST["ENABLE_AIRNOW"] ?? 'no');
    $KEY_AIRNOW = test_input($_POST["KEY_AIRNOW"] ?? '');
    $AIRNOW_DISTANCE = test_input($_POST["AIRNOW_DISTANCE"] ?? '25');
    
  // Get current version    
    $ver = $conn->prepare("SELECT version FROM hiveconfig");
    $ver->execute();
    $ver = $ver->fetchColumn();
// Increment version
    $version = ++$ver;

    // Update into the DB
    $doit = $conn->prepare("UPDATE hiveconfig SET ENABLE_HIVE_CAMERA=?,ENABLE_HIVE_WEIGHT_CHK=?,ENABLE_HIVE_TEMP_CHK=?,SCALETYPE=?,TEMPTYPE=?,version=?,HIVEDEVICE=?,ENABLE_LUX=?,LUX_SOURCE=?,HIVE_TEMP_GPIO=?,HIVE_WEIGHT_SLOPE=?,HIVE_WEIGHT_INTERCEPT=?,ENABLE_BEECOUNTER=?,CAMERATYPE=?,COUNTERTYPE=?,weather_level=?,key=?,wxstation=?,WXTEMPTYPE=?,WX_TEMPER_DEVICE=?,WX_TEMP_GPIO=?,weather_detail=?,local_wx_type=?,local_wx_url=?, HIVE_LUX_SLOPE=?, HIVE_LUX_INTERCEPT=?, HIVE_TEMP_SLOPE=?, HIVE_TEMP_INTERCEPT=?, WX_TEMP_SLOPE=?, WX_TEMP_INTERCEPT=?, HIVE_HUMIDITY_SLOPE=?, HIVE_HUMIDITY_INTERCEPT=?, WX_HUMIDITY_SLOPE=?, WX_HUMIDITY_INTERCEPT=?, HIVE_LUX_GPIO=?, HIVE_WEIGHT_GPIO=?,HIVE_TEMP_SUB=?,ENABLE_AIR=?,AIR_TYPE=?,AIR_ID=?,AIR_API=?,AIR_LOCAL_URL=?,WEIGHT_COMPENSATION_ENABLED=?,WEIGHT_TEMP_COEFF=?,WEIGHT_HUMIDITY_COEFF=?,WEIGHT_REF_TEMP=?,WEIGHT_REF_HUMIDITY=?,WEATHER_FALLBACK=?,WEATHER_FALLBACK_2=?,WX_MAX_STALE_MINUTES=?,KEY_OPENWEATHERMAP=?,KEY_WEATHERAPI=?,KEY_VISUALCROSSING=?,KEY_PIRATEWEATHER=?,KEY_TOMORROW=?,KEY_AMBEE=?,ENABLE_AIRNOW=?,KEY_AIRNOW=?,AIRNOW_DISTANCE=? WHERE id=1");
    $doit->execute(array($ENABLE_HIVE_CAMERA,$ENABLE_HIVE_WEIGHT_CHK,$ENABLE_HIVE_TEMP_CHK,$SCALETYPE,$TEMPTYPE,$version,$HIVEDEVICE,$ENABLE_LUX,$LUX_SOURCE,$HIVE_TEMP_GPIO,$HIVE_WEIGHT_SLOPE,$HIVE_WEIGHT_INTERCEPT,$ENABLE_BEECOUNTER,$CAMERATYPE,$COUNTERTYPE,$weather_level,$key,$wxstation,$WXTEMPTYPE,$WX_TEMPER_DEVICE,$WX_TEMP_GPIO,$weather_detail,$local_wx_type,$local_wx_url,$HIVE_LUX_SLOPE, $HIVE_LUX_INTERCEPT, $HIVE_TEMP_SLOPE, $HIVE_TEMP_INTERCEPT, $WX_TEMP_SLOPE, $WX_TEMP_INTERCEPT, $HIVE_HUMIDITY_SLOPE, $HIVE_HUMIDITY_INTERCEPT, $WX_HUMIDITY_SLOPE, $WX_HUMIDITY_INTERCEPT, $HIVE_LUX_GPIO, $HIVE_WEIGHT_GPIO, $HIVE_TEMP_SUB, $ENABLE_AIR, $AIR_TYPE, $AIR_ID, $AIR_API, $AIR_LOCAL_URL, $WEIGHT_COMPENSATION_ENABLED, $WEIGHT_TEMP_COEFF, $WEIGHT_HUMIDITY_COEFF, $WEIGHT_REF_TEMP, $WEIGHT_REF_HUMIDITY, $weather_fallback, $weather_fallback_2, $wx_max_stale_minutes, $key_openweathermap, $key_weatherapi, $key_visualcrossing, $key_pirateweather, $key_tomorrow, $key_ambee, $ENABLE_AIRNOW, $KEY_AIRNOW, $AIRNOW_DISTANCE));
    sleep(1);



    // Refresh the fields in the form
    $sth = $conn->prepare("SELECT * FROM hiveconfig");
    $sth->execute();
    $result = $sth->fetch(PDO::FETCH_ASSOC);
    
    // Tell user it saved
    #echo '<div class="alert alert-success alert-dismissable">
     #                           <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>';
    #echo 'Successfully Saved';
    #echo '</div>';
} else {
    // Errors
     echo '<div class="alert alert-danger alert-dismissable"> <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>';
      $err=array_values($v->errors());
      for ($i=0; $i < count($err); $i++){
        echo $err[$i][0]."<br>";
      }
          echo  '</div>';  
              // Refresh the fields in the form
    $sth = $conn->prepare("SELECT * FROM hiveconfig");
    $sth->execute();
    $result = $sth->fetch(PDO::FETCH_ASSOC);  
}
               
    }
        ?>
            <!-- /.row -->
           <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <!-- /.panel-heading -->
                        
                        <div class="panel-body">
                            <!-- Hidden fields to ensure required fields are always present for validation -->
                            <input type="hidden" name="ENABLE_HIVE_CAMERA" value="<?php echo $result['ENABLE_HIVE_CAMERA']; ?>">
                            <input type="hidden" name="ENABLE_HIVE_WEIGHT_CHK" value="<?php echo $result['ENABLE_HIVE_WEIGHT_CHK']; ?>">
                            <input type="hidden" name="ENABLE_HIVE_TEMP_CHK" value="<?php echo $result['ENABLE_HIVE_TEMP_CHK']; ?>">
                            <input type="hidden" name="ENABLE_LUX" value="<?php echo $result['ENABLE_LUX']; ?>">
                            <input type="hidden" name="ENABLE_BEECOUNTER" value="<?php echo $result['ENABLE_BEECOUNTER']; ?>">
                            <input type="hidden" name="ENABLE_AIR" value="<?php echo $result['ENABLE_AIR']; ?>">
                            <input type="hidden" name="WEATHER_LEVEL" value="<?php echo $result['WEATHER_LEVEL']; ?>">
                            <input type="hidden" name="WEATHER_FALLBACK" value="<?php echo htmlspecialchars($result['WEATHER_FALLBACK'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="WEATHER_FALLBACK_2" value="<?php echo htmlspecialchars($result['WEATHER_FALLBACK_2'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="WX_MAX_STALE_MINUTES" value="<?php echo htmlspecialchars($result['WX_MAX_STALE_MINUTES'] ?? '120', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="WEATHER_DETAIL" value="<?php echo $result['WEATHER_DETAIL']; ?>">
                            <input type="hidden" name="KEY" value="<?php echo htmlspecialchars($result['KEY'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="KEY_OPENWEATHERMAP" value="<?php echo htmlspecialchars($result['KEY_OPENWEATHERMAP'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="KEY_WEATHERAPI" value="<?php echo htmlspecialchars($result['KEY_WEATHERAPI'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="KEY_VISUALCROSSING" value="<?php echo htmlspecialchars($result['KEY_VISUALCROSSING'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="KEY_PIRATEWEATHER" value="<?php echo htmlspecialchars($result['KEY_PIRATEWEATHER'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="KEY_TOMORROW" value="<?php echo htmlspecialchars($result['KEY_TOMORROW'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="KEY_AMBEE" value="<?php echo htmlspecialchars($result['KEY_AMBEE'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="WXSTATION" value="<?php echo $result['WXSTATION']; ?>">
                            <input type="hidden" name="CAMERATYPE" value="<?php echo $result['CAMERATYPE']; ?>">
                            <input type="hidden" name="COUNTERTYPE" value="<?php echo $result['COUNTERTYPE']; ?>">
                            <input type="hidden" name="AIR_TYPE" value="<?php echo $result['AIR_TYPE'] ?? ''; ?>">
                            <input type="hidden" name="AIR_ID" value="<?php echo $result['AIR_ID'] ?? ''; ?>">
                            <input type="hidden" name="AIR_API" value="<?php echo $result['AIR_API'] ?? ''; ?>">
                            <input type="hidden" name="AIR_LOCAL_URL" value="<?php echo $result['AIR_LOCAL_URL'] ?? ''; ?>">
                            <input type="hidden" name="ENABLE_AIRNOW" value="<?php echo $result['ENABLE_AIRNOW'] ?? 'no'; ?>">
                            <input type="hidden" name="KEY_AIRNOW" value="<?php echo $result['KEY_AIRNOW'] ?? ''; ?>">
                            <input type="hidden" name="AIRNOW_DISTANCE" value="<?php echo $result['AIRNOW_DISTANCE'] ?? '25'; ?>">
                            <input type="hidden" name="TEMPTYPE" value="<?php echo $result['TEMPTYPE']; ?>">
                            <input type="hidden" name="HIVEDEVICE" value="<?php echo $result['HIVEDEVICE']; ?>">
                            <input type="hidden" name="HIVE_TEMP_SUB" value="<?php echo $result['HIVE_TEMP_SUB']; ?>">
                            <input type="hidden" name="HIVE_TEMP_GPIO" value="<?php echo $result['HIVE_TEMP_GPIO']; ?>">
                            <input type="hidden" name="HIVE_TEMP_SLOPE" value="<?php echo $result['HIVE_TEMP_SLOPE']; ?>">
                            <input type="hidden" name="HIVE_TEMP_INTERCEPT" value="<?php echo $result['HIVE_TEMP_INTERCEPT']; ?>">
                            <input type="hidden" name="HIVE_HUMIDITY_SLOPE" value="<?php echo $result['HIVE_HUMIDITY_SLOPE']; ?>">
                            <input type="hidden" name="HIVE_HUMIDITY_INTERCEPT" value="<?php echo $result['HIVE_HUMIDITY_INTERCEPT']; ?>">
                            <input type="hidden" name="SCALETYPE" value="<?php echo $result['SCALETYPE']; ?>">
                            <input type="hidden" name="HIVE_WEIGHT_GPIO" value="<?php echo $result['HIVE_WEIGHT_GPIO']; ?>">
                            <input type="hidden" name="HIVE_WEIGHT_SLOPE" value="<?php echo $result['HIVE_WEIGHT_SLOPE']; ?>">
                            <input type="hidden" name="HIVE_WEIGHT_INTERCEPT" value="<?php echo $result['HIVE_WEIGHT_INTERCEPT']; ?>">
                            <input type="hidden" name="WEIGHT_COMPENSATION_ENABLED" value="<?php echo htmlspecialchars($result['WEIGHT_COMPENSATION_ENABLED'] ?? 'no', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="WEIGHT_TEMP_COEFF" value="<?php echo htmlspecialchars($result['WEIGHT_TEMP_COEFF'] ?? '0', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="WEIGHT_HUMIDITY_COEFF" value="<?php echo htmlspecialchars($result['WEIGHT_HUMIDITY_COEFF'] ?? '0', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="WEIGHT_REF_TEMP" value="<?php echo htmlspecialchars($result['WEIGHT_REF_TEMP'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="WEIGHT_REF_HUMIDITY" value="<?php echo htmlspecialchars($result['WEIGHT_REF_HUMIDITY'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <input type="hidden" name="LUX_SOURCE" value="<?php echo $result['LUX_SOURCE']; ?>">
                            <input type="hidden" name="HIVE_LUX_GPIO" value="<?php echo $result['HIVE_LUX_GPIO']; ?>">
                            <input type="hidden" name="HIVE_LUX_SLOPE" value="<?php echo $result['HIVE_LUX_SLOPE']; ?>">
                            <input type="hidden" name="HIVE_LUX_INTERCEPT" value="<?php echo $result['HIVE_LUX_INTERCEPT']; ?>">
                            <input type="hidden" name="WXTEMPTYPE" value="<?php echo $result['WXTEMPTYPE']; ?>">
                            <input type="hidden" name="WX_TEMPER_DEVICE" value="<?php echo $result['WX_TEMPER_DEVICE']; ?>">
                            <input type="hidden" name="WX_TEMP_GPIO" value="<?php echo $result['WX_TEMP_GPIO']; ?>">
                            <input type="hidden" name="local_wx_type" value="<?php echo $result['local_wx_type']; ?>">
                            <input type="hidden" name="local_wx_url" value="<?php echo $result['local_wx_url']; ?>">
                            <input type="hidden" name="WX_TEMP_SLOPE" value="<?php echo $result['WX_TEMP_SLOPE']; ?>">
                            <input type="hidden" name="WX_TEMP_INTERCEPT" value="<?php echo $result['WX_TEMP_INTERCEPT']; ?>">
                            <input type="hidden" name="WX_HUMIDITY_SLOPE" value="<?php echo $result['WX_HUMIDITY_SLOPE']; ?>">
                            <input type="hidden" name="WX_HUMIDITY_INTERCEPT" value="<?php echo $result['WX_HUMIDITY_INTERCEPT']; ?>">
                            <div class="table-responsive">
                                <table class="table table-striped table-bordered table-hover" id="dataTables-example">
                                    <thead>
                                        <tr>
                                            <th>Instrument</th>
                                            <th>Device</th>
                                            <th>Options</th>
                                            <th>Zero/Intercept <a href="#" title="Define Zero/Intercept" data-toggle="popover" data-placement="bottom" data-content="Specify the Zero or Intercept value - see hivetool.org for instructions. Setting a value will subtract this value from the sensor raw reading. For example, if your sensor says 1020 and you set this value to 20, then it'll subtract 20 from 1020, giving you 1000. If this feature is not needed, please set this value to 0.
                                                        "><p class="fa fa-question-circle fa-fw"></P></a></th>
                                            <th>Calibration/Slope <a href="#" title="Specify Cali/Slope" data-toggle="popover" data-placement="bottom" data-content="Specify the Calibration or Slope value - see hivetool.org for instructions. If this feature is not used, please set to 1"><p class="fa fa-question-circle fa-fw"></P></a></th>
                                            <?PHP if ( $livevalue == "yes") {
                                                    echo '<th>Live Value (will appear when read) <a class="btn btn-primary" href="/admin/instrumentconfig.php?livevalue=yes" role="button">Re-Read</a><br></th>';
                                            } else {
                                                echo '<th><center>
                                                <a class="btn btn-primary" href="/admin/instrumentconfig.php?livevalue=yes" role="button">Read Sensors</a><br>Takes a few seconds';
                                            } ?>
                                            
                                        </tr>
                                    </thead>
                                    <tbody>
                                       
    <?PHP ###############################################################################################################
          # Hive Temp/Humidity
          ############################################################################################################### ?>
                                        <tr class="odd gradeX">
                                            <td>
                                            <a href="#" title="Temp/Hum Sensors" data-toggle="popover" data-placement="bottom" data-content="Specify which Temp/Humidity Sensor you have installed"><p class="fa fa-question-circle fa-fw"></P></a>
                                                 Temp/Hum Sensors<br>
                                            <select name="ENABLE_HIVE_TEMP_CHK" onchange="this.form.submit()">
                                            <option value="yes" <?php if ($result['ENABLE_HIVE_TEMP_CHK'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                                            <option value="no" <?php if ($result['ENABLE_HIVE_TEMP_CHK'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                                            </select></td>
                                            <td>
                                            <?php if ($result['ENABLE_HIVE_TEMP_CHK'] == "yes") {
                                                echo '
                                                <input type="radio" name="TEMPTYPE" onchange="this.form.submit()" value="temperhum"'; if ($result['TEMPTYPE'] == "temperhum") {echo "checked";} echo '> TemperHum
                                                <br><input type="radio" name="TEMPTYPE" onchange="this.form.submit()" value="temper"'; if ($result['TEMPTYPE'] == "temper") {echo "checked";} echo '> Temper 
                                                <br><input type="radio" name="TEMPTYPE" onchange="this.form.submit()" value="dht22"'; if ($result['TEMPTYPE'] == "dht22") {echo "checked";} echo '> DHT22 
                                                <br><input type="radio" name="TEMPTYPE" onchange="this.form.submit()" value="dht21"'; if ($result['TEMPTYPE'] == "dht21") {echo "checked";} echo '> DHT21
                                                <br><input type="radio" name="TEMPTYPE" onchange="this.form.submit()" value="sht31d"'; if ($result['TEMPTYPE'] == "sht31d") {echo "checked";} echo '> SHT31-D
                                                <br><input type="radio" name="TEMPTYPE" onchange="this.form.submit()" value="bme280"'; if ($result['TEMPTYPE'] == "bme280") {echo "checked";} echo '> BME280
                                                <br><input type="radio" name="TEMPTYPE" onchange="this.form.submit()" value="bme680"'; if ($result['TEMPTYPE'] == "bme680") {echo "checked";} echo '> BME680
                                                <br><input type="radio" name="TEMPTYPE" onchange="this.form.submit()" value="broodminder"'; if ($result['TEMPTYPE'] == "broodminder") {echo "checked";} echo '> BroodMinder T/TH';

                                            }
                                            ?></td>
                                            <td>
                                            <?PHP if ($result['ENABLE_HIVE_TEMP_CHK'] == "yes") {
                                                    if ($result['TEMPTYPE'] == "temperhum") {
                                                        echo '<a href="#" title="Specify Device" data-toggle="popover" data-placement="bottom" data-content="Specify the device you want to use (usually /dev/hidraw1, if you only have one device. Use
                                                        tempered -e from the console to see choices)"><p class="fa fa-question-circle fa-fw"></P></a>Device:
                                                         <input type="text" name="HIVEDEVICE" onchange="this.form.submit()" value="'; echo $result['HIVEDEVICE']; echo '"">';
                                                    }
                                                    if ($result['TEMPTYPE'] == "temper") {
                                                        echo '<a href="#" title="Specify Device" data-toggle="popover" data-placement="bottom" data-content="Specify the device you want to use (usually /dev/hidraw1, if you only have one device. Use
                                                        tempered -e from the console to see choices)"><p class="fa fa-question-circle fa-fw"></P></a>Device:
                                                         <input type="text" name="HIVEDEVICE" onchange="this.form.submit()" value="'; echo $result['HIVEDEVICE']; echo '"">';
                                                         echo '<a href="#" title="Specify Sub Device" data-toggle="popover" data-placement="bottom" data-content="Specificy the sub device, if you have one. This is only used when you have two sensors on one temper device. )"><p class="fa fa-question-circle fa-fw"></P></a>Sub-Device:<input type="text" name="HIVE_TEMP_SUB" onchange="this.form.submit()" value="'; echo $result['HIVE_TEMP_SUB']; echo '"">';

                                                    }
                                                    if ($result['TEMPTYPE'] == "dht22") {
                                                        echo '<a href="#" title="Specify GPIO" data-toggle="popover" data-placement="bottom" data-content="Specify the GPIO you want to use to connect to this sensor"><p class="fa fa-question-circle fa-fw"></P></a>GPIO:
                                                         <input type="text" name="HIVE_TEMP_GPIO" onchange="this.form.submit()" value="'; echo $result['HIVE_TEMP_GPIO']; echo '"">';
                                                    }
                                                    if ($result['TEMPTYPE'] == "dht21") {
                                                        echo '<a href="#" title="Specify GPIO" data-toggle="popover" data-placement="bottom" data-content="Specify the GPIO you want to use to connect to this sensor"><p class="fa fa-question-circle fa-fw"></P></a>GPIO:
                                                         <input type="text" name="HIVE_TEMP_GPIO" onchange="this.form.submit()" value="'; echo $result['HIVE_TEMP_GPIO']; echo '"">';
                                                    }
                                                    if ($result['TEMPTYPE'] == "broodminder") {
                                                        echo '<a href="#" title="Specify Device Address" data-toggle="popover" data-placement="bottom" data-content="Specify which BLE device you want to listen for in the format of 06:09:42:1c:8a, (Full MAC address and lowercase) which is written on device"><p class="fa fa-question-circle fa-fw"></P></a>Device:
                                                         <input type="text" name="HIVEDEVICE" onchange="this.form.submit()" value="'; echo $result['HIVEDEVICE']; echo '"">';
                                                    }
                                            }?>
                                            </td>
                                            <td>
                                            <?PHP if ($result['ENABLE_HIVE_TEMP_CHK'] == "yes") {
                                                    echo '<table>
                                                        <tr><td>Temp </td><td> 
                                                        <input type="text" name="HIVE_TEMP_INTERCEPT" onchange="this.form.submit()" value="'; 
                                                    echo $result['HIVE_TEMP_INTERCEPT']; echo '"> </td></tr>';

                                            echo ' <tr><td>Humidity </td><td><input type="text" name="HIVE_HUMIDITY_INTERCEPT" onchange="this.form.submit()" value="'; 
                                                    echo $result['HIVE_HUMIDITY_INTERCEPT']; echo '"></td></tr>
                                            </table>';
                                            
                                            }?>
                                            
                                            </td>
                                            <td>   <?PHP if ($result['ENABLE_HIVE_TEMP_CHK'] == "yes") {
                                                    echo '<table>
                                                        <tr><td>Temp </td><td> 
                                                        <input type="text" name="HIVE_TEMP_SLOPE" onchange="this.form.submit()" value="'; 
                                                    echo $result['HIVE_TEMP_SLOPE']; echo '"> </td></tr>';

                                            echo ' <tr><td>Humidity </td><td><input type="text" name="HIVE_HUMIDITY_SLOPE" onchange="this.form.submit()" value="'; 
                                                    echo $result['HIVE_HUMIDITY_SLOPE']; echo '"></td></tr>
                                            </table>';
                                            
                                            }?></td>
                                            <td>
                                            <?PHP 
                                            if ($result['ENABLE_HIVE_TEMP_CHK'] == "yes") {
                                                if ( $livevalue == "yes") {
                                                echo '<div id="ReadTemp"></script></div>';
                                                #echo '<iframe src="/admin/livevalue.php?sensor=hivetemp" frameborder="0" allowtransparency="true" scrolling="no" style="border:none;"></iframe>';
                                                } 
                                            }
                                            ?>
                                            </td>
                                        </tr>
    <?PHP ###############################################################################################################
          # Hive Weight
          ############################################################################################################### ?>
                                        <tr class="odd gradeX">
                                            <td>
                                            <a href="#" title="Weight Scale" data-toggle="popover" data-placement="bottom" data-content="Enable/Disable Weight Checking, then specify which board you have installed"><p class="fa fa-question-circle fa-fw"></P></a>
                                                 Weight Scale<br>
                                            
                                            <select name="ENABLE_HIVE_WEIGHT_CHK" onchange="this.form.submit()">
                                            <option value="yes" <?php if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                                            <option value="no" <?php if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                                            </select></td>
                                            <td>
                                            <?php if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "yes") {
                                                echo '
                                                <input type="radio" name="SCALETYPE" onchange="this.form.submit()" value="phidget1046"'; if ($result['SCALETYPE'] == "phidget1046") {echo "checked";} echo '> Phidget 1046
                                                <br><input type="radio" name="SCALETYPE" onchange="this.form.submit()" value="hx711"'; if ($result['SCALETYPE'] == "hx711") {echo "checked";} echo '> HX711
                                                <br><input type="radio" name="SCALETYPE" onchange="this.form.submit()" value="cpw200plus"'; if ($result['SCALETYPE'] == "cpw200plus") {echo "checked";} echo '> CPW 200 Plus
                                                <br><input type="radio" name="SCALETYPE" onchange="this.form.submit()" value="None"'; if ($result['SCALETYPE'] == "None") {echo 'checked';} echo '> None';
                                            }
                                            ?>
                                            </td>
                                            <td>
                                                <?php if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "yes") {
                                                #switch ($result['SCALETYPE']) {
                                                #        case "phidget1046":
                                                            #Future Use
                                                #            break;
                                                #        case "hx711":
                                                #        echo '<a href="#" title="Specify GPIO" data-toggle="popover" data-placement="bottom" data-content="Specify the GPIO you want to use to connect to this sensor"><p class="fa fa-question-circle fa-fw"></P></a>GPIO:
                                                #        <input type="text" name="HIVE_WEIGHT_GPIO" onchange="this.form.submit()" value="'; echo $result['HIVE_WEIGHT_GPIO']; echo '"">';
                                                #            break;
                                                #        case "cpw200plus":
                                                            #Futureuse
                                                #            break;
                                                #    }

                                                } ?>
                                            </td>
                                            
                                            <td>
                                                 <?PHP if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "yes") {
                                                    
                                                    echo '
                                                         <input type="text" name="HIVE_WEIGHT_INTERCEPT" onchange="this.form.submit()" value="'; echo $result['HIVE_WEIGHT_INTERCEPT']; echo '"">'; 
                                            }?>

                                            </td>
                                            <td><?PHP if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "yes") {
                                                    
                                                    echo '<input type="text" name="HIVE_WEIGHT_SLOPE" onchange="this.form.submit()" value="'; echo $result['HIVE_WEIGHT_SLOPE']; echo '"">';
                                            }?>
                    
                                            </td>
                                            <td>
                                            <?PHP 
                                            if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "yes") {
                                                if ( $livevalue == "yes") {
                                                    #Show calibration for hx711
                                                switch ($result['SCALETYPE']) {
                                                        case "phidget1046":
                                                            #Future Use
                                                            break;
                                                        case "hx711":
                                                        echo '<center><a class="btn btn-primary" href="/admin/setup-wizard.php?step=3">Calibrate Wizard</a><br>';
                                                            break;
                                                        case "cpw200plus":
                                                            #Futureuse
                                                            break;
                                                    }
                                                    echo '<div id="ReadWeight"></script></div>';
                
                                                } else {
                                                    switch ($result['SCALETYPE']) {
                                                        case "phidget1046":
                                                            #Future Use
                                                            break;
                                                        case "hx711":
                                                        echo '<center><a class="btn btn-primary" href="/admin/setup-wizard.php?step=3">Calibrate Wizard</a><br>';
                                                            break;
                                                        case "cpw200plus":
                                                            #Futureuse
                                                            break;
                                                        }
                                                }
                                            
                                            
                                            }
                                            ?>
                                            </td>
                                        </tr>

<?PHP ###############################################################################################################
      # Weight Environmental Compensation
      ############################################################################################################### ?>
                                        <?PHP if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "yes") { ?>
                                        <tr class="odd gradeX">
                                            <td>
                                            <a href="#" title="Weight Drift Compensation" data-toggle="popover" data-placement="bottom" data-content="Automatically corrects weight readings for temperature and humidity drift. Coefficients are recalculated weekly from nighttime data when bees are inactive."><p class="fa fa-question-circle fa-fw"></P></a>
                                                 Weight Drift Compensation<br>

                                            <select name="WEIGHT_COMPENSATION_ENABLED" onchange="this.form.submit()">
                                            <option value="yes" <?php if (($result['WEIGHT_COMPENSATION_ENABLED'] ?? 'no') == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                                            <option value="no" <?php if (($result['WEIGHT_COMPENSATION_ENABLED'] ?? 'no') == "no") {echo "selected='selected'";} ?>>Disabled</option>
                                            </select></td>
                                            <td colspan="4">
                                            <?php
                                            $comp_enabled = ($result['WEIGHT_COMPENSATION_ENABLED'] ?? 'no') == "yes";
                                            if ($comp_enabled) {
                                                $last_cal = $result['WEIGHT_LAST_CALIBRATED'] ?? null;
                                                $r2 = $result['WEIGHT_CALIBRATION_R2'] ?? null;
                                                $tc = $result['WEIGHT_TEMP_COEFF'] ?? 0;
                                                $hc = $result['WEIGHT_HUMIDITY_COEFF'] ?? 0;

                                                if ($last_cal) {
                                                    echo '<small><i class="fa fa-check-circle" style="color:green;"></i> <strong>Auto-calibrated:</strong> ' . htmlspecialchars($last_cal) . '</small><br>';
                                                    echo '<small>R&sup2;=' . number_format((float)$r2, 4) . ' &nbsp;&bull;&nbsp; Temp: ' . number_format((float)$tc, 6) . ' lbs/&deg;F &nbsp;&bull;&nbsp; Humidity: ' . number_format((float)$hc, 6) . ' lbs/%RH</small>';
                                                } else {
                                                    echo '<small><i class="fa fa-clock-o" style="color:#e6b800;"></i> <strong>Waiting for data.</strong> Auto-calibration runs weekly at 5am using nighttime readings. Needs 50+ samples over 14 days with 5&deg;F+ temperature variation.</small>';
                                                }
                                            } else {
                                                echo '<small>Enable to automatically correct weight drift caused by temperature and humidity changes. Recalibrates weekly from nighttime data.</small>';
                                            } ?>
                                            </td>
                                            <td></td>
                                        </tr>
                                        <?PHP } ?>

<?PHP ###############################################################################################################
      # Lux
      ############################################################################################################### ?>
                                        <tr class="odd gradeX">
                                            <td>
                                            <a href="#" title="Light Source" data-toggle="popover" data-placement="bottom" data-content="Enable/Disable local light meters, then specify which board you have installed"><p class="fa fa-question-circle fa-fw"></P></a>
                                                 Light Source<br>
                                        
                                            <select name="ENABLE_LUX" onchange="this.form.submit()">
                                            <option value="yes" <?php if ($result['ENABLE_LUX'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                                            <option value="no" <?php if ($result['ENABLE_LUX'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                                            </select></td>
                                            <td>
                                            <?php if ($result['ENABLE_LUX'] == "yes") {
                                                echo '
                                                <input type="radio" name="LUX_SOURCE" onchange="this.form.submit()" value="tsl2591"'; if ($result['LUX_SOURCE'] == "tsl2591") {echo "checked";} echo '> TSL 2591
                                                <br><input type="radio" name="LUX_SOURCE" onchange="this.form.submit()" value="tsl2561"'; if ($result['LUX_SOURCE'] == "tsl2561") {echo "checked";} echo '> TSL 2561
                                                <br><input type="radio" name="LUX_SOURCE" onchange="this.form.submit()" value="wx"'; if ($result['LUX_SOURCE'] == "wx") {echo "checked";} echo '> WX Station';
                                            }
                                            ?>
                                            </td>
                                            <td>
                                                <?php if ($result['ENABLE_LUX'] == "yes") {
                                                #switch ($result['LUX_SOURCE']) {
                                                #        case "tsl2561":
                                                #             echo '<a href="#" title="Specify GPIO" data-toggle="popover" data-placement="bottom" data-content="Specify the GPIO you want to use to connect to this sensor - Default on HiveTool Interface Board is 3"><p class="fa fa-question-circle fa-fw"></P></a>GPIO:
                                                 #        <input type="text" name="HIVE_LUX_GPIO" onchange="this.form.submit()" value="'; echo $result['HIVE_LUX_GPIO']; echo '"">';
                                                 #           break;
                                                 #       case "tsl2591":

                                                 #       echo '<a href="#" title="Specify GPIO" data-toggle="popover" data-placement="bottom" data-content="Specify the GPIO you want to use to connect to this sensor - Default on HiveTool Interface Board is 3"><p class="fa fa-question-circle fa-fw"></P></a>GPIO:
                                                 #        <input type="text" name="HIVE_LUX_GPIO" onchange="this.form.submit()" value="'; echo $result['HIVE_LUX_GPIO']; echo '"">';
                                                 #           break;
                                                 #       case "wx":
                                                            #Futureuse
                                                  #          break;
                                                   # }

                                                } ?>

                                            </td>

                                            <td>  <?PHP if ($result['ENABLE_LUX'] == "yes") {
                                                    echo '
                                                         <input type="text" name="HIVE_LUX_INTERCEPT" onchange="this.form.submit()" value="'; echo $result['HIVE_LUX_INTERCEPT']; echo '"">';
                                            }?></td>
                                            <td>  <?PHP if ($result['ENABLE_LUX'] == "yes") {
                                                    echo '
                                                         <input type="text" name="HIVE_LUX_SLOPE" onchange="this.form.submit()" value="'; echo $result['HIVE_LUX_SLOPE']; echo '"">';
                                            }?></td></center></td>
                                            <td><?PHP 
                                            if ($result['ENABLE_LUX'] == "yes") {
                                                if ( $livevalue == "yes") {
                                                    echo '<div id="ReadLux"></script></div>';
                                                    
                                                }
                                            } 
                                            ?>
                                                    </td>
                                        </tr>
<?PHP ###############################################################################################################
      # BeeCounter
      ############################################################################################################### ?>
                                        <tr class="odd gradeX">
                                            <td>
                                                <a href="#" title="Bee Counter" data-toggle="popover" data-placement="bottom" data-content="Enable Bee Counters and specify the type of Bee Counter you are using"><p class="fa fa-question-circle fa-fw"></P></a>
                                                Bee Counter<br>
                                            
                                            <select name="ENABLE_BEECOUNTER" onchange="this.form.submit()">
                                            <option value="yes" <?php if ($result['ENABLE_BEECOUNTER'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                                            <option value="no" <?php if ($result['ENABLE_BEECOUNTER'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                                            </select></td>

                                            <td>
                                            <?php if ($result['ENABLE_BEECOUNTER'] == "yes") {
                                                echo '
                                                <input type="radio" name="COUNTERTYPE" onchange="this.form.submit()" value="PICAMERA"'; if ($result['COUNTERTYPE'] == "PICAMERA") {echo "checked";} echo '> PI Camera
                                                <br><input type="radio" name="COUNTERTYPE" onchange="this.form.submit()" value="GATES"'; if ($result['COUNTERTYPE'] == "GATES") {echo "checked";} echo '> Gates';}
                                            ?>
                                            </td>
                                            <td></td>
                                            <td></td>
                                            <td></td>
                                            <td></td>
                                        </tr>
<?PHP ###############################################################################################################
      # Camera Type
      ############################################################################################################### ?>
                                        <tr class="odd gradeX">
                                            <td>
                                                <a href="#" title="Camera Type" data-toggle="popover" data-placement="bottom" data-content="Specify the type of camera you are using"><p class="fa fa-question-circle fa-fw"></P></a>
                                                Camera<br>
                                            
                                            <select name="ENABLE_HIVE_CAMERA" onchange="this.form.submit()">
                                            <option value="yes" <?php if ($result['ENABLE_HIVE_CAMERA'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                                            <option value="no" <?php if ($result['ENABLE_HIVE_CAMERA'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                                            </select></td>

                                            <td>
                                            <?php if ($result['ENABLE_HIVE_CAMERA'] == "yes") {
                                                echo '
                                                <input type="radio" name="CAMERATYPE" onchange="this.form.submit()" value="PI"'; if ($result['CAMERATYPE'] == "PI") {echo "checked";} echo '> PI Camera
                                                <br><input type="radio" name="CAMERATYPE" onchange="this.form.submit()" value="USB"'; if ($result['CAMERATYPE'] == "USB") {echo "checked";} echo '> USB Type';
                                            }
                                            ?>
                                            </td>
                                            <td></td>
                                            <td></td>
                                            <td></td>
                                            <td></td>
                                        </tr>
<?PHP ###############################################################################################################
      # WeatherSource
      ############################################################################################################### ?>  
                                <tr class="odd gradeX">
                            <td><a href="#" title="Weather Source" data-toggle="popover" data-placement="bottom" data-content="Specify where you want to get your ambient weather data from."><p class="fa fa-question-circle fa-fw"></P></a>Weather Source<br>

    <!-- ******************************************************************************************** -->

                            
                            <select name="WEATHER_LEVEL" onchange="this.form.submit()">
                            <optgroup label="Cloud APIs (Free)">
                            <option value="openmeteo" <?php if ($result['WEATHER_LEVEL'] == "openmeteo") {echo "selected='selected'";} ?>>Open-Meteo (No Key)</option>
                            <option value="nws" <?php if ($result['WEATHER_LEVEL'] == "nws") {echo "selected='selected'";} ?>>NWS weather.gov (US Only)</option>
                            </optgroup>
                            <optgroup label="Cloud APIs (Key Required)">
                            <option value="openweathermap" <?php if ($result['WEATHER_LEVEL'] == "openweathermap") {echo "selected='selected'";} ?>>OpenWeatherMap</option>
                            <option value="weatherapi" <?php if ($result['WEATHER_LEVEL'] == "weatherapi") {echo "selected='selected'";} ?>>WeatherAPI.com</option>
                            <option value="visualcrossing" <?php if ($result['WEATHER_LEVEL'] == "visualcrossing") {echo "selected='selected'";} ?>>Visual Crossing</option>
                            <option value="pirateweather" <?php if ($result['WEATHER_LEVEL'] == "pirateweather") {echo "selected='selected'";} ?>>Pirate Weather</option>
                            </optgroup>
                            <optgroup label="Personal Weather Stations">
                            <option value="ambientwx" <?php if ($result['WEATHER_LEVEL'] == "ambientwx") {echo "selected='selected'";} ?>>AmbientWeather.net</option>
                            <option value="hive" <?php if ($result['WEATHER_LEVEL'] == "hive") {echo "selected='selected'";} ?>>WX Underground</option>
                            <option value="wf_tempest_local" <?php if ($result['WEATHER_LEVEL'] == "wf_tempest_local") {echo "selected='selected'";} ?>>WF Tempest UDP</option>
                            </optgroup>
                            <optgroup label="Local Hardware">
                            <option value="localws" <?php if ($result['WEATHER_LEVEL'] == "localws") {echo "selected='selected'";} ?>>Local Weather Station</option>
                            <option value="localsensors" <?php if ($result['WEATHER_LEVEL'] == "localsensors") {echo "selected='selected'";} ?>>Local Hive Sensors</option>
                            </optgroup>
                            </select></td>
                            <td>

                                <?php
                                $wx_lat = htmlspecialchars($result['LATITUDE'] ?? '');
                                $wx_lon = htmlspecialchars($result['LONGITUDE'] ?? '');

                                if ($result['WEATHER_LEVEL'] == "openmeteo") {
                                    echo 'Location: <strong>' . $wx_lat . ', ' . $wx_lon . '</strong><br>';
                                    echo '<small>Free &mdash; no API key needed. Data from ECMWF, DWD, NOAA models.<br>';
                                    echo 'Change location in <a href="hiveconfig.php">Hive Config</a>.</small>';
                                }

                                if ($result['WEATHER_LEVEL'] == "nws") {
                                    echo 'Location: <strong>' . $wx_lat . ', ' . $wx_lon . '</strong><br>';
                                    echo '<small>Free &mdash; no API key needed. <strong>US locations only.</strong><br>';
                                    echo 'Station auto-discovered from lat/lon. Change in <a href="hiveconfig.php">Hive Config</a>.</small>';
                                }

                                if ($result['WEATHER_LEVEL'] == "openweathermap") {
                                    echo 'API KEY <br><input type="text" name="KEY_OPENWEATHERMAP" size="40" onchange="this.form.submit()" value="' . htmlspecialchars($result['KEY_OPENWEATHERMAP'] ?? $result['KEY'] ?? '') . '"><br>';
                                    echo '<small>Free: 1,000 calls/day. Get a key at <a href="https://openweathermap.org/api" target="_blank">openweathermap.org</a></small><br>';
                                    echo '<small>Location: ' . $wx_lat . ', ' . $wx_lon . '</small>';
                                }

                                if ($result['WEATHER_LEVEL'] == "weatherapi") {
                                    echo 'API KEY <br><input type="text" name="KEY_WEATHERAPI" size="40" onchange="this.form.submit()" value="' . htmlspecialchars($result['KEY_WEATHERAPI'] ?? $result['KEY'] ?? '') . '"><br>';
                                    echo '<small>Free: 1M calls/month. Get a key at <a href="https://www.weatherapi.com/signup.aspx" target="_blank">weatherapi.com</a></small><br>';
                                    echo '<small>Location: ' . $wx_lat . ', ' . $wx_lon . '</small>';
                                }

                                if ($result['WEATHER_LEVEL'] == "visualcrossing") {
                                    echo 'API KEY <br><input type="text" name="KEY_VISUALCROSSING" size="40" onchange="this.form.submit()" value="' . htmlspecialchars($result['KEY_VISUALCROSSING'] ?? $result['KEY'] ?? '') . '"><br>';
                                    echo '<small>Free: 1,000 calls/day. Includes solar radiation. Get a key at <a href="https://www.visualcrossing.com/sign-up" target="_blank">visualcrossing.com</a></small><br>';
                                    echo '<small>Location: ' . $wx_lat . ', ' . $wx_lon . '</small>';
                                }

                                if ($result['WEATHER_LEVEL'] == "pirateweather") {
                                    echo 'API KEY <br><input type="text" name="KEY_PIRATEWEATHER" size="40" onchange="this.form.submit()" value="' . htmlspecialchars($result['KEY_PIRATEWEATHER'] ?? $result['KEY'] ?? '') . '"><br>';
                                    echo '<small>Free: 20,000 calls/day. Dark Sky compatible. Get a key at <a href="https://pirateweather.net" target="_blank">pirateweather.net</a></small><br>';
                                    echo '<small>Location: ' . $wx_lat . ', ' . $wx_lon . '</small>';
                                }

                                if ($result['WEATHER_LEVEL'] == "hive") {
                                    echo 'STATION ID <br><input type="text" name="WXSTATION" onchange="this.form.submit()" value="'; echo $result['WXSTATION']; echo '">';
                                }

                                if ($result['WEATHER_LEVEL'] == "ambientwx") {

                                    echo 'API KEY <br><input type="text" name="KEY" onchange="this.form.submit()" value="'; echo $result['KEY']; echo '"><BR>';
                                    echo 'STATION MAC <br><input type="text" name="WXSTATION" onchange="this.form.submit()" value="'; echo $result['WXSTATION']; echo '">';
                                }

                                if ($result['WEATHER_LEVEL'] == "wf_tempest_local") {

                                    #echo 'API KEY <br><input type="text" name="KEY" onchange="this.form.submit()" value="'; echo $result['KEY']; echo '"><BR>';
                                    echo 'STATION SERIAL <br><input type="text" name="WXSTATION" onchange="this.form.submit()" value="'; echo $result['WXSTATION']; echo '">';    
                                }

                                if ($result['WEATHER_LEVEL'] == "localws") {
                                    #echo 'Using WS1400.sh for local WX Station';
                                    
                                    echo '
                                    <input type="radio" name="local_wx_type" onchange="this.form.submit()" value="WS1400ip"'; if ($result['local_wx_type'] == "WS1400ip") {echo "checked";} echo '> WS1400ip <br>
                                    <input type="radio" name="local_wx_type" onchange="this.form.submit()" value="ourweather"'; if ($result['local_wx_type'] == "ourweather") {echo "checked";} echo '> OurWeather <br>
                                    </td>';
                                    if ($result['local_wx_type'] == "WS1400ip" || "ourweather") {
                                        echo '<td>IP <input type="text" name="local_wx_url" onchange="this.form.submit()" value="'; echo $result['local_wx_url']; echo '"</td>'; 
                                    }  

                                }
                                    
                               if ($result['WEATHER_LEVEL'] == "localsensors") {
                                    //echo 'Using Locally Connected Sensores for local WX Station';
                                echo '
                                <input type="radio" name="WXTEMPTYPE" onchange="this.form.submit()" value="temperhum"'; if ($result['WXTEMPTYPE'] == "temperhum") {echo "checked";} echo '> Temperhum<br>
                                <input type="radio" name="WXTEMPTYPE" onchange="this.form.submit()" value="dht21"'; if ($result['WXTEMPTYPE'] == "dht21") {echo "checked";} echo '> DHT21<br>
                                    <input type="radio" name="WXTEMPTYPE" onchange="this.form.submit()" value="dht22"'; if ($result['WXTEMPTYPE'] == "dht22") {echo "checked";} echo '> DHT22<br>
                                    <input type="radio" name="WXTEMPTYPE" onchange="this.form.submit()" value="sht31d"'; if ($result['WXTEMPTYPE'] == "sht31d") {echo "checked";} echo '> SHT31-D<br>
                                    <input type="radio" name="WXTEMPTYPE" onchange="this.form.submit()" value="bme280"'; if ($result['WXTEMPTYPE'] == "bme280") {echo "checked";} echo '> BME280 <br>
                                    <input type="radio" name="WXTEMPTYPE" onchange="this.form.submit()" value="bme680"'; if ($result['WXTEMPTYPE'] == "bme680") {echo "checked";} echo '> BME680';
                                    if ($result['WXTEMPTYPE'] == "temperhum") {
                                        echo '</td><td>Device <input type="text" name="WX_TEMPER_DEVICE" onchange="this.form.submit()" value="'; echo $result['WX_TEMPER_DEVICE']; echo '"">';

                                    }
                                    if ($result['WXTEMPTYPE'] == "dht21" || $result['WXTEMPTYPE'] == "dht22") {
                                 echo '</td><td>GPIO <input type="text" name="WX_TEMP_GPIO" onchange="this.form.submit()" value="'; echo $result['WX_TEMP_GPIO']; echo '"></td>';
                                    }
                                }?></td>

                                <td> </td>
                                <td> </td>
                                <td></td>
                        </tr>
<?PHP ###############################################################################################################
      # Weather Fallback
      ############################################################################################################### ?>
                        <tr class="odd gradeX">
                            <td><a href="#" title="Weather Fallback" data-toggle="popover" data-placement="bottom" data-content="If the primary weather source fails, this backup source will be used automatically. Only cloud APIs can be used as fallback."><p class="fa fa-question-circle fa-fw"></P></a>Weather Fallback<br>
                            <select name="WEATHER_FALLBACK" onchange="this.form.submit()">
                            <option value="" <?php if (empty($result['WEATHER_FALLBACK'])) {echo "selected='selected'";} ?>>None (No Fallback)</option>
                            <optgroup label="Cloud APIs (Free)">
                            <option value="openmeteo" <?php if (($result['WEATHER_FALLBACK'] ?? '') == "openmeteo") {echo "selected='selected'";} ?>>Open-Meteo (No Key)</option>
                            <option value="nws" <?php if (($result['WEATHER_FALLBACK'] ?? '') == "nws") {echo "selected='selected'";} ?>>NWS weather.gov (US Only)</option>
                            </optgroup>
                            <optgroup label="Cloud APIs (Key Required)">
                            <option value="openweathermap" <?php if (($result['WEATHER_FALLBACK'] ?? '') == "openweathermap") {echo "selected='selected'";} ?>>OpenWeatherMap</option>
                            <option value="weatherapi" <?php if (($result['WEATHER_FALLBACK'] ?? '') == "weatherapi") {echo "selected='selected'";} ?>>WeatherAPI.com</option>
                            <option value="visualcrossing" <?php if (($result['WEATHER_FALLBACK'] ?? '') == "visualcrossing") {echo "selected='selected'";} ?>>Visual Crossing</option>
                            <option value="pirateweather" <?php if (($result['WEATHER_FALLBACK'] ?? '') == "pirateweather") {echo "selected='selected'";} ?>>Pirate Weather</option>
                            </optgroup>
                            </select></td>
                            <td>
                                <?php
                                $fb = $result['WEATHER_FALLBACK'] ?? '';
                                if (!empty($fb) && $fb == $result['WEATHER_LEVEL']) {
                                    echo '<span style="color:red"><strong>Warning:</strong> Fallback is the same as primary source.</span>';
                                } elseif (!empty($fb)) {
                                    echo '<small>If ' . htmlspecialchars($result['WEATHER_LEVEL']) . ' fails, ' . htmlspecialchars($fb) . ' will be used automatically.</small>';
                                } else {
                                    echo '<small>Recommended: Set Open-Meteo or NWS as fallback for reliability.</small>';
                                }
                                $keyed_providers = ['openweathermap' => 'KEY_OPENWEATHERMAP', 'weatherapi' => 'KEY_WEATHERAPI', 'visualcrossing' => 'KEY_VISUALCROSSING', 'pirateweather' => 'KEY_PIRATEWEATHER'];
                                if (!empty($fb) && isset($keyed_providers[$fb])) {
                                    $fb_key_col = $keyed_providers[$fb];
                                    echo '<br>API KEY: <input type="text" name="' . $fb_key_col . '" size="30" onchange="this.form.submit()" value="' . htmlspecialchars($result[$fb_key_col] ?? '', ENT_QUOTES, 'UTF-8') . '">';
                                }
                                ?>
                            </td>
                            <td> </td>
                            <td> </td>
                            <td></td>
                        </tr>
<?PHP ###############################################################################################################
      # Weather Fallback 2
      ############################################################################################################### ?>
                        <tr class="odd gradeX">
                            <td><a href="#" title="Weather Fallback 2" data-toggle="popover" data-placement="bottom" data-content="Third-tier weather source. Used if both primary and first fallback fail."><p class="fa fa-question-circle fa-fw"></P></a>Weather Fallback 2<br>
                            <select name="WEATHER_FALLBACK_2" onchange="this.form.submit()">
                            <option value="" <?php if (empty($result['WEATHER_FALLBACK_2'] ?? '')) {echo "selected='selected'";} ?>>None</option>
                            <optgroup label="Cloud APIs (Free)">
                            <option value="openmeteo" <?php if (($result['WEATHER_FALLBACK_2'] ?? '') == "openmeteo") {echo "selected='selected'";} ?>>Open-Meteo (No Key)</option>
                            <option value="nws" <?php if (($result['WEATHER_FALLBACK_2'] ?? '') == "nws") {echo "selected='selected'";} ?>>NWS weather.gov (US Only)</option>
                            </optgroup>
                            <optgroup label="Cloud APIs (Key Required)">
                            <option value="openweathermap" <?php if (($result['WEATHER_FALLBACK_2'] ?? '') == "openweathermap") {echo "selected='selected'";} ?>>OpenWeatherMap</option>
                            <option value="weatherapi" <?php if (($result['WEATHER_FALLBACK_2'] ?? '') == "weatherapi") {echo "selected='selected'";} ?>>WeatherAPI.com</option>
                            <option value="visualcrossing" <?php if (($result['WEATHER_FALLBACK_2'] ?? '') == "visualcrossing") {echo "selected='selected'";} ?>>Visual Crossing</option>
                            <option value="pirateweather" <?php if (($result['WEATHER_FALLBACK_2'] ?? '') == "pirateweather") {echo "selected='selected'";} ?>>Pirate Weather</option>
                            </optgroup>
                            </select></td>
                            <td>
                                <?php
                                $fb2 = $result['WEATHER_FALLBACK_2'] ?? '';
                                $fb1 = $result['WEATHER_FALLBACK'] ?? '';
                                if (!empty($fb2) && ($fb2 == $result['WEATHER_LEVEL'] || $fb2 == $fb1)) {
                                    echo '<span style="color:red"><strong>Warning:</strong> Duplicate provider in chain.</span>';
                                } elseif (!empty($fb2)) {
                                    echo '<small>Last resort if both primary and fallback 1 fail.</small>';
                                } else {
                                    echo '<small>Optional third-tier source for maximum reliability.</small>';
                                }
                                if (!empty($fb2) && isset($keyed_providers[$fb2])) {
                                    $fb2_key_col = $keyed_providers[$fb2];
                                    echo '<br>API KEY: <input type="text" name="' . $fb2_key_col . '" size="30" onchange="this.form.submit()" value="' . htmlspecialchars($result[$fb2_key_col] ?? '', ENT_QUOTES, 'UTF-8') . '">';
                                }
                                ?>
                            </td>
                            <td> </td>
                            <td> </td>
                            <td></td>
                        </tr>
<?PHP ###############################################################################################################
      # Weather Staleness Threshold
      ############################################################################################################### ?>
                        <tr class="odd gradeX">
                            <td><a href="#" title="Stale Data Threshold" data-toggle="popover" data-placement="bottom" data-content="Maximum age (in minutes) of a weather observation before it is considered stale and the next provider in the chain is tried. Default: 120 minutes."><p class="fa fa-question-circle fa-fw"></P></a>Stale Data Threshold<br>
                            </td>
                            <td>
                                <input type="number" name="WX_MAX_STALE_MINUTES" min="30" max="360" value="<?php echo htmlspecialchars($result['WX_MAX_STALE_MINUTES'] ?? '120', ENT_QUOTES, 'UTF-8'); ?>"> minutes
                                <br><small>Reject observations older than this and try the next weather source. Default: 120 min.</small>
                            </td>
                            <td> </td>
                            <td> </td>
                            <td></td>
                        </tr>
<?PHP ###############################################################################################################
      # Weather Health Inline
      ############################################################################################################### ?>
                        <tr class="odd gradeX">
                            <td><a href="#" title="Weather Health" data-toggle="popover" data-placement="bottom" data-content="Quick health summary for configured weather providers. Click the link for the full dashboard."><p class="fa fa-question-circle fa-fw"></P></a>Weather Health</td>
                            <td colspan="3">
<?PHP
try {
    $health_check = $conn->prepare("SELECT provider, COUNT(*) as total, SUM(success) as ok, ROUND(AVG(success)*100,0) as rate FROM weather_health WHERE timestamp >= datetime('now', '-1 day', 'localtime') GROUP BY provider ORDER BY total DESC");
    $health_check->execute();
    $health_rows = $health_check->fetchAll(PDO::FETCH_ASSOC);
} catch (Exception $e) {
    $health_rows = [];
}
if (empty($health_rows)) {
    echo '<small class="text-muted">No health data yet.</small> ';
} else {
    foreach ($health_rows as $hr) {
        $r = intval($hr['rate']);
        if ($r >= 95) { $cls = 'success'; }
        elseif ($r >= 80) { $cls = 'warning'; }
        else { $cls = 'danger'; }
        echo '<span class="label label-' . $cls . '">' . htmlspecialchars($hr['provider']) . ': ' . $r . '%</span> ';
    }
    echo '<br>';
}
?>
                                <small><a href="/admin/weather_health.php"><i class="fa fa-bar-chart"></i> Full Weather Health Dashboard</a></small>
                            </td>
                            <td></td>
                        </tr>
<?PHP ###############################################################################################################
      # AirQuality
      ############################################################################################################### ?>
                        <tr class="odd gradeX">
                            <td>
                                <a href="#" title="Air Quality" data-toggle="popover" data-placement="bottom" data-content="Enable Air Quality Checks and specify which air sensors you use or one that is within 10 miles of your hives."><p class="fa fa-question-circle fa-fw"></P></a>
                                Air Quality<br>

                            <select name="ENABLE_AIR" onchange="this.form.submit()">
                            <option value="yes" <?php if ($result['ENABLE_AIR'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                            <option value="no" <?php if ($result['ENABLE_AIR'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                            </select></td>

                            <td>
                            <?php if ($result['ENABLE_AIR'] == "yes"): ?>
                                <input type="radio" name="AIR_TYPE" onchange="this.form.submit()" value="purpleapi"
                                    <?php if ($result['AIR_TYPE'] == "purpleapi" || $result['AIR_TYPE'] == "purple") echo 'checked'; ?>> PurpleAir - API <br>
                                <input type="radio" name="AIR_TYPE" onchange="this.form.submit()" value="purplelocal"
                                    <?php if ($result['AIR_TYPE'] == "purplelocal") echo 'checked'; ?>> PurpleAir - Local
                            <?php endif; ?>
                            </td>
                            <td>
                            <?php if ($result['ENABLE_AIR'] == "yes" && ($result['AIR_TYPE'] == "purpleapi" || $result['AIR_TYPE'] == "purple")): ?>
                              <a href="#" title="Air ID" data-toggle="popover" data-placement="bottom"
                                 data-content="Go to <a href='https://www.purpleair.com/map' target='_blank' title='purpleair'>Purpleair.com</a> to get an ID.">
                                 <i class="fa fa-question-circle fa-fw"></i>
                              </a>
                              STATION ID <br>
                              <input type="text" name="AIR_ID" onchange="this.form.submit()"
                                     value="<?php echo htmlspecialchars($result['AIR_ID'], ENT_QUOTES, 'UTF-8'); ?>">
                              <br>
                              API READ KEY <br>
                              <input type="text" name="AIR_API" onchange="this.form.submit()"
                                     value="<?php echo htmlspecialchars($result['AIR_API'] ?? '', ENT_QUOTES, 'UTF-8'); ?>">
                            <?php elseif ($result['ENABLE_AIR'] == "yes" && $result['AIR_TYPE'] == "purplelocal"): ?>
                              <a href="#" title="Air ID" data-toggle="popover" data-placement="bottom"
                                 data-content="Go to <a href='https://www.purpleair.com/map' target='_blank' title='purpleair'>Purpleair.com</a> to get an ID.">
                                 <i class="fa fa-question-circle fa-fw"></i>
                              </a>
                              STATION ID <br>
                              <input type="text" name="AIR_ID" onchange="this.form.submit()"
                                     value="<?php echo htmlspecialchars($result['AIR_ID'], ENT_QUOTES, 'UTF-8'); ?>">
                              <br>
                              LOCAL URL <br>
                              <input type="text" name="AIR_LOCAL_URL" onchange="this.form.submit()"
                                     value="<?php echo htmlspecialchars($result['AIR_LOCAL_URL'] ?? '', ENT_QUOTES, 'UTF-8'); ?>"
                                     placeholder="http://192.168.1.x/json" title="Must include /json path">
                            <?php endif; ?>
                            </td>
                            <td></td>
                            <td></td>
                            <td></td>
                        </tr>
<?PHP ############################################################################################################### ?>
                        <tr class="odd gradeX">
                            <td>
                                <a href="#" title="EPA AirNow" data-toggle="popover" data-placement="bottom" data-content="Pulls hourly O3 (ozone) and NO2 data from the nearest EPA regulatory monitor via AirNow API. Free, 500 requests/day."><p class="fa fa-question-circle fa-fw"></P></a>
                                EPA AirNow<br>
                                <small class="text-muted">Ozone &amp; NO2</small>
                            </td>
                            <td>
                            <select name="ENABLE_AIRNOW" onchange="this.form.submit()">
                            <option value="yes" <?php if (($result['ENABLE_AIRNOW'] ?? 'no') == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                            <option value="no" <?php if (($result['ENABLE_AIRNOW'] ?? 'no') == "no") {echo "selected='selected'";} ?>>Disabled</option>
                            </select>
                            </td>
                            <td>
                            <?php if (($result['ENABLE_AIRNOW'] ?? 'no') == "yes"): ?>
                              AirNow API Key (<a href="https://docs.airnowapi.org/account/request/" target="_blank">Get free key</a>)<br>
                              <input type="text" name="KEY_AIRNOW" size="40" onchange="this.form.submit()"
                                     value="<?php echo htmlspecialchars($result['KEY_AIRNOW'] ?? '', ENT_QUOTES, 'UTF-8'); ?>"
                                     placeholder="AirNow API Key">
                              <br><br>
                              Search Radius (miles)<br>
                              <input type="number" name="AIRNOW_DISTANCE" min="5" max="100" onchange="this.form.submit()"
                                     value="<?php echo htmlspecialchars($result['AIRNOW_DISTANCE'] ?? '25', ENT_QUOTES, 'UTF-8'); ?>">
                            <?php endif; ?>
                            </td>
                            <td></td>
                            <td></td>
                            <td></td>
                        </tr>
<?PHP ############################################################################################################### ?>
                        <tr class="odd gradeX">
                            <td>
                                <a href="#" title="Pollen Data" data-toggle="popover" data-placement="bottom" data-content="Pollen data is fetched daily from Pollen.com (US, free). Optional fallback providers require API keys."><p class="fa fa-question-circle fa-fw"></P></a>
                                Pollen Data<br>
                                <small class="text-muted">Primary: Pollen.com (US)</small>
                            </td>
                            <td>
                                <small>Optional fallback providers:</small><br>
                                <a href="https://www.tomorrow.io/weather-api/" target="_blank">Tomorrow.io</a><br>
                                <a href="https://www.getambee.com/" target="_blank">Ambee</a>
                            </td>
                            <td>
                              Tomorrow.io API Key<br>
                              <input type="text" name="KEY_TOMORROW" size="40" onchange="this.form.submit()"
                                     value="<?php echo htmlspecialchars($result['KEY_TOMORROW'] ?? '', ENT_QUOTES, 'UTF-8'); ?>"
                                     placeholder="Optional">
                              <br><br>
                              Ambee API Key<br>
                              <input type="text" name="KEY_AMBEE" size="40" onchange="this.form.submit()"
                                     value="<?php echo htmlspecialchars($result['KEY_AMBEE'] ?? '', ENT_QUOTES, 'UTF-8'); ?>"
                                     placeholder="Optional">
                            </td>
                            <td></td>
                            <td></td>
                            <td></td>
                        </tr>
<?PHP ############################################################################################################### ?>

                                    </tbody>
                                </table>


                            </div>
                            <!-- /.table-responsive -->
                        </div>
                        <!-- /.panel-body -->
                    </div>
                    <!-- /.panel -->
                </div>
                <!-- /.col-lg-12 -->
            </div>
                        </div>
                        <!-- /.panel-footer -->
                    </div>
                    <!-- /.panel .chat-panel -->
                </div>
                <!-- /.col-lg-4 -->
            </div>
            <!-- /.row -->

                <div class="modal fade" id="wx" tabindex="-1" role="dialog" aria-labelledby="wx" aria-hidden="true">
                                <div class="modal-dialog">
                                    <div class="modal-content">
                                        <div class="modal-header">
                                            <button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>
                                            <h4 class="modal-title" id="myModalLabel">Weather Underground Help</h4>
                                        </div>
                                        <div class="modal-body">
                                             Specify where you want to get your weather forecast data from, since the local weather source used for ambient does not forecast. Get an API Key from <a href="http://www.wunderground.com/weather/api/">WX Underground</a>.
                                        </div>
                                        <div class="modal-footer">
                                            <button type="button" class="btn btn-default" data-dismiss="modal">Close Help</button>
                                        </div>
                                    </div>
                                    <!-- /.modal-content -->
                                </div>
                                <!-- /.modal-dialog -->
                </div>
                
                                <!-- /.modal -->
        </div>
        <!-- /#page-wrapper -->

    </div>
    <!-- /#wrapper -->

                
    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>

    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>
    
    <!-- Full Screen Popups -->
    <script src="/js/popup.js"></script>  
    
    <script>
    $(document).ready(function(){
    $('[data-toggle="popover"]').popover({html:true}); 
    });
    </script>
    
    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

    <?PHP 
    $FILEVERSION = file_get_contents('./hiveconfig.ver', true);
    if ( $result['VERSION'] > $FILEVERSION ) {
    #echo "Configuration has changed, pulling new config, please wait....\n";
    #echo $FILEVERSION;
    $foo = shell_exec("sudo /home/HiveControl/scripts/data/hiveconfig.sh");
    #echo "New Configuration Loaded\n"; 
    }
    ?>

    <script>
     $(document).ready(function() {
         $("#ReadTemp").load("livevalue.php?sensor=hivetemp");
       $.ajaxSetup({ cache: false });
    });
    </script>
    <script>
     $(document).ready(function() {
         $("#ReadWeight").load("livevalue.php?sensor=hiveweight");
       $.ajaxSetup({ cache: false });
    });
    </script>
   <script>
     $(document).ready(function() {
         $("#ReadLux").load("livevalue.php?sensor=hivelux");
       $.ajaxSetup({ cache: false });
    });
    </script>
</body>

</html>
