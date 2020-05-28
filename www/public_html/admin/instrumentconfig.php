<?PHP
# Version 2020052701

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
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
    $v->rule('numeric', ['HIVE_WEIGHT_SLOPE', 'HIVE_WEIGHT_INTERCEPT', 'HIVE_LUX_SLOPE', 'HIVE_LUX_INTERCEPT', 'HIVE_TEMP_SLOPE', 'HIVE_TEMP_INTERCEPT', 'WX_TEMP_SLOPE','WX_TEMP_INTERCEPT','HIVE_HUMIDITY_SLOPE','HIVE_HUMIDITY_INTERCEPT','WX_HUMIDITY_SLOPE','WX_HUMIDITY_INTERCEPT'], 1)->message('{field} must be numeric');
    $v->rule('alphaNum', ['SCALETYPE', 'TEMPTYPE', 'LUX_SOURCE', 'COUNTERTYPE', 'CAMERATYPE', 'local_wx_type', 'AIR_ID', 'AIR_TYPE'], 1)->message('{field} must be alphaNum only');
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
        
        echo '<input type="hidden" name="SCALETYPE" value="' . $SCALETYPE . '">
        <input type="hidden" name="HIVE_WEIGHT_GPIO" value="' . $HIVE_WEIGHT_GPIO . '">
        <input type="hidden" name="HIVE_WEIGHT_SLOPE" value="' . $HIVE_WEIGHT_SLOPE . '">
        <input type="hidden" name="HIVE_WEIGHT_INTERCEPT" value="' . $HIVE_WEIGHT_INTERCEPT . '">';

    } else {
        $SCALETYPE = test_input($_POST["SCALETYPE"]);
        $HIVE_WEIGHT_GPIO = test_input($_POST["HIVE_WEIGHT_GPIO"]);
        $HIVE_WEIGHT_SLOPE = test_input($_POST["HIVE_WEIGHT_SLOPE"]);
        $HIVE_WEIGHT_INTERCEPT = test_input($_POST["HIVE_WEIGHT_INTERCEPT"]);
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
    $wxstation = test_input($_POST["WXSTATION"]);
    $WXTEMPTYPE = test_input($_POST["WXTEMPTYPE"]);
    $WX_TEMPER_DEVICE = test_input_allow_slash($_POST["WX_TEMPER_DEVICE"]);
    $WX_TEMP_GPIO = test_input($_POST["WX_TEMP_GPIO"]);
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
    
  // Get current version    
    $ver = $conn->prepare("SELECT version FROM hiveconfig");
    $ver->execute();
    $ver = $ver->fetchColumn();
// Increment version
    $version = ++$ver;

    // Update into the DB
    $doit = $conn->prepare("UPDATE hiveconfig SET ENABLE_HIVE_CAMERA=?,ENABLE_HIVE_WEIGHT_CHK=?,ENABLE_HIVE_TEMP_CHK=?,SCALETYPE=?,TEMPTYPE=?,version=?,HIVEDEVICE=?,ENABLE_LUX=?,LUX_SOURCE=?,HIVE_TEMP_GPIO=?,HIVE_WEIGHT_SLOPE=?,HIVE_WEIGHT_INTERCEPT=?,ENABLE_BEECOUNTER=?,CAMERATYPE=?,COUNTERTYPE=?,weather_level=?,key=?,wxstation=?,WXTEMPTYPE=?,WX_TEMPER_DEVICE=?,WX_TEMP_GPIO=?,weather_detail=?,local_wx_type=?,local_wx_url=?, HIVE_LUX_SLOPE=?, HIVE_LUX_INTERCEPT=?, HIVE_TEMP_SLOPE=?, HIVE_TEMP_INTERCEPT=?, WX_TEMP_SLOPE=?, WX_TEMP_INTERCEPT=?, HIVE_HUMIDITY_SLOPE=?, HIVE_HUMIDITY_INTERCEPT=?, WX_HUMIDITY_SLOPE=?, WX_HUMIDITY_INTERCEPT=?, HIVE_LUX_GPIO=?, HIVE_WEIGHT_GPIO=?,HIVE_TEMP_SUB=?,ENABLE_AIR=?,AIR_TYPE=?,AIR_ID=? WHERE id=1");
    $doit->execute(array($ENABLE_HIVE_CAMERA,$ENABLE_HIVE_WEIGHT_CHK,$ENABLE_HIVE_TEMP_CHK,$SCALETYPE,$TEMPTYPE,$version,$HIVEDEVICE,$ENABLE_LUX,$LUX_SOURCE,$HIVE_TEMP_GPIO,$HIVE_WEIGHT_SLOPE,$HIVE_WEIGHT_INTERCEPT,$ENABLE_BEECOUNTER,$CAMERATYPE,$COUNTERTYPE,$weather_level,$key,$wxstation,$WXTEMPTYPE,$WX_TEMPER_DEVICE,$WX_TEMP_GPIO,$weather_detail,$local_wx_type,$local_wx_url,$HIVE_LUX_SLOPE, $HIVE_LUX_INTERCEPT, $HIVE_TEMP_SLOPE, $HIVE_TEMP_INTERCEPT, $WX_TEMP_SLOPE, $WX_TEMP_INTERCEPT, $HIVE_HUMIDITY_SLOPE, $HIVE_HUMIDITY_INTERCEPT, $WX_HUMIDITY_SLOPE, $WX_HUMIDITY_INTERCEPT, $HIVE_LUX_GPIO, $HIVE_WEIGHT_GPIO, $HIVE_TEMP_SUB, $ENABLE_AIR, $AIR_TYPE, $AIR_ID));
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
                            <div class="dataTable_wrapper">
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
                                                        echo '<center><button class="btn btn-primary" data-toggle="modal" type="button" data-target="#CalibModal">Calibrate Wizard</button><br>';
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
                                                        echo '<center><button class="btn btn-primary" data-toggle="modal" type="button" data-target="#CalibModal">Calibrate Wizard</button><br>';
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
                            <!-- <option value="yard" <?php #if ($result['WEATHER_LEVEL'] == "yard") {echo "selected='selected'";} ?>>Yard Controller</option> -->
                            <option value="hive" <?php if ($result['WEATHER_LEVEL'] == "hive") {echo "selected='selected'";} ?>>WX Underground</option>
                            <option value="ambientwx" <?php if ($result['WEATHER_LEVEL'] == "ambientwx") {echo "selected='selected'";} ?>>AmbientWeather.net</option>
                            <option value="localws" <?php if ($result['WEATHER_LEVEL'] == "localws") {echo "selected='selected'";} ?>>Local Weather Station</option>
                            <option value="localsensors" <?php if ($result['WEATHER_LEVEL'] == "localsensors") {echo "selected='selected'";} ?>>Local Hive Sensors</option>
                            <option value="wf_tempest_local" <?php if ($result['WEATHER_LEVEL'] == "wf_tempest_local") {echo "selected='selected'";} ?>>WF Tempest UDP</option>
                            </select></td>
                            <td>

                                <?php if ($result['WEATHER_LEVEL'] == "hive") {
                                    // WX Underground settings
                                    //WEATHER_DETAIL - PWS, CITY
                                    // KEY
                                    // WXSTATION

                                    //echo 'API KEY <br><input type="text" name="KEY" onchange="this.form.submit()" value="'; echo $result['KEY']; echo '"><BR>';
                                    echo 'STATION ID <br><input type="text" name="WXSTATION" onchange="this.form.submit()" value="'; echo $result['WXSTATION']; echo '">';    
 
                                }
                                else {
                                    //echo '<input type="hidden" name="KEY" value="'; echo $result['KEY']; echo '">';
                                    //echo '<input type="hidden" name="WXSTATION" value="'; echo $result['WXSTATION']; echo '">';    
                                }
                                
                                if ($result['WEATHER_LEVEL'] == "ambientwx") {

                                    echo 'API KEY <br><input type="text" name="KEY" onchange="this.form.submit()" value="'; echo $result['KEY']; echo '"><BR>';
                                    echo 'STATION ID <br><input type="text" name="WXSTATION" onchange="this.form.submit()" value="'; echo $result['WXSTATION']; echo '">';    
                                }

                                if ($result['WEATHER_LEVEL'] == "wf_tempest_local") {

                                    #echo 'API KEY <br><input type="text" name="KEY" onchange="this.form.submit()" value="'; echo $result['KEY']; echo '"><BR>';
                                    echo 'STATION SERIAL <br><input type="text" name="WXSTATION" onchange="this.form.submit()" value="'; echo $result['WXSTATION']; echo '">';    
                                }

                                if ($result['WEATHER_LEVEL'] == "localws") {
                                    #echo 'Using WS1400.sh for local WX Station';
                                    #First, set the radio buttons to the proper station
                                    echo '
                                    <input type="radio" name="local_wx_type" onchange="this.form.submit()" value="WS1400ip"'; 
                                    if ($result['local_wx_type'] == "WS1400ip") {echo "checked";} echo '> WS1400ip <br></td>';
                                    if ($result['local_wx_type'] == "ourweather") {echo "checked";} echo '> OurWeather <br></td>';
                                    
                                    #Then, set the particular fields needed
                                    if ($result['local_wx_type'] == "WS1400ip") {echo '<td>IP <input type="text" name="local_wx_url" onchange="this.form.submit()" value="'; echo $result['local_wx_url']; echo '"</td>'; 
                                    if ($result['local_wx_type'] == "ourweather") {echo '<td>IP <input type="text" name="local_wx_url" onchange="this.form.submit()" value="'; echo $result['local_wx_url']; echo '"</td>'; 
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
      # AirQuality
      ############################################################################################################### ?>
                        <tr class="odd gradeX">
                            <td>
                                <a href="#" title="Air Quality" data-toggle="popover" data-placement="bottom" data-content="Enable Air Quailty Checks and specify which air sensors you use or one that is within 10 miles of your hives."><p class="fa fa-question-circle fa-fw"></P></a>
                                Air Quaility<br>
                            
                            <select name="ENABLE_AIR" onchange="this.form.submit()">
                            <option value="yes" <?php if ($result['ENABLE_AIR'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                            <option value="no" <?php if ($result['ENABLE_AIR'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                            </select></td>

                            <td>
                            <?php if ($result['ENABLE_AIR'] == "yes") {
                                echo '
                                <input type="radio" name="AIR_TYPE" onchange="this.form.submit()" value="purple"'; if ($result['AIR_TYPE'] == "purple") {echo "checked";} echo '> PurpleAir';} #Only one at the moment
                            ?>
                            </td>
                            <td>
                            <?PHP if ($result['ENABLE_AIR'] == "yes" && $result['AIR_TYPE'] == "purple") {
                                echo '<a href="#" title="Air ID" data-toggle="popover" data-placement="bottom" data-content="Go to <a href=\'https://www.purpleair.com/map\' target=\'_blank\' title=\'purpleair\'>Purpleair.com</a> to get an ID."><p class="fa fa-question-circle fa-fw"></P></a>';
                                    echo 'STATION ID <br><input type="text" name="AIR_ID" onchange="this.form.submit()" value="'; echo $result['AIR_ID']; echo '">';    
                                } ?>
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

            <div class="modal fade" id="CalibModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true">
                                <div class="modal-dialog">
                                    <div class="modal-content">
                                        <div class="modal-header">
                                            <button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>
                                            <h4 class="modal-title" id="myModalLabel">Scale Calibration Wizard</h4>
                                        </div>
                                        <div class="modal-body">
                                            <iframe src="/admin/hx_wiz.php" width="100%" height="100%" frameborder="0" allowtransparency="true"></iframe>  
                                        </div>
                                        <div class="modal-footer">
                                            <button type="button" class="btn btn-default" data-dismiss="modal">Close Wizard</button>
                                        </div>
                                    </div>
                                    <!-- /.modal-content -->
                                </div>
                                <!-- /.modal-dialog -->
                </div>
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
