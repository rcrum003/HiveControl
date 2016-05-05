<?PHP

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

if ($_SERVER["REQUEST_METHOD"] == "GET") {
$sth = $conn->prepare("SELECT * FROM hiveconfig");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);



}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
// Set Error Fields

$v = new Valitron\Validator($_POST);
$v->rule('required', ['ENABLE_HIVE_CAMERA', 'ENABLE_HIVE_WEIGHT_CHK', 'ENABLE_HIVE_TEMP_CHK', 'ENABLE_LUX'], 1)->message('{field} is required');
$v->rule('in', ['ENABLE_HIVE_WEIGHT', 'ENABLE_LUX', 'ENABLE_HIVE_CAMERA', 'ENABLE_HIVE_WEIGHT_CHK', 'ENABLE_HIVE_TEMP_CHK', 'ENABLE_BEECOUNTER'], ['no', 'yes']);
$v->rule('integer', ['HIVE_TEMP_GPIO'], 1)->message('{field} must be a integer');
$v->rule('numeric', ['HIVE_WEIGHT_SLOPE', 'HIVE_WEIGHT_INTERCEPT'], 1)->message('{field} must be numeric');
$v->rule('alphaNum', ['SCALETYPE', 'TEMPTYPE', 'LUX_SOURCE', 'COUNTERTYPE', 'CAMERATYPE'], 1)->message('{field} must be alphaNum only');


}
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
        <?PHP
        if ($_SERVER["REQUEST_METHOD"] == "POST") {
        
if($v->validate()) {


    
// Make sure no bad stuff got through our filters
// Probably not needed, but doesn't hurt

    
    $ENABLE_HIVE_WEIGHT_CHK = test_input($_POST["ENABLE_HIVE_WEIGHT_CHK"]);
    $ENABLE_HIVE_TEMP_CHK = test_input($_POST["ENABLE_HIVE_TEMP_CHK"]);    
    $SCALETYPE = test_input($_POST["SCALETYPE"]);
    $TEMPTYPE = test_input($_POST["TEMPTYPE"]);
    $HIVEDEVICE = test_input($_POST["HIVEDEVICE"]);
    $ENABLE_LUX = test_input($_POST["ENABLE_LUX"]);
    $LUX_SOURCE = test_input($_POST["LUX_SOURCE"]);
    $HIVE_TEMP_GPIO = test_input($_POST["HIVE_TEMP_GPIO"]);
    $HIVE_WEIGHT_SLOPE = test_input($_POST["HIVE_WEIGHT_SLOPE"]);
    $HIVE_WEIGHT_INTERCEPT = test_input($_POST["HIVE_WEIGHT_INTERCEPT"]);

    $ENABLE_HIVE_CAMERA = test_input($_POST["ENABLE_HIVE_CAMERA"]);
    $ENABLE_BEECOUNTER= test_input($_POST["ENABLE_BEECOUNTER"]);
    $CAMERATYPE = test_input($_POST["CAMERATYPE"]);
    $COUNTERTYPE = test_input($_POST["COUNTERTYPE"]);

  // Get current version    
    $ver = $conn->prepare("SELECT version FROM hiveconfig");
    $ver->execute();
    $ver = $ver->fetchColumn();
// Increment version
    $version = ++$ver;

    // Update into the DB
    $doit = $conn->prepare("UPDATE hiveconfig SET ENABLE_HIVE_CAMERA=?,ENABLE_HIVE_WEIGHT_CHK=?,ENABLE_HIVE_TEMP_CHK=?,SCALETYPE=?,TEMPTYPE=?,version=?,HIVEDEVICE=?,ENABLE_LUX=?,LUX_SOURCE=?,HIVE_TEMP_GPIO=?,HIVE_WEIGHT_SLOPE=?,HIVE_WEIGHT_INTERCEPT=?,ENABLE_BEECOUNTER=?,CAMERATYPE=?,COUNTERTYPE=? WHERE id=1");
    $doit->execute(array($ENABLE_HIVE_CAMERA,$ENABLE_HIVE_WEIGHT_CHK,$ENABLE_HIVE_TEMP_CHK,$SCALETYPE,$TEMPTYPE,$version,$HIVEDEVICE,$ENABLE_LUX,$LUX_SOURCE,$HIVE_TEMP_GPIO,$HIVE_WEIGHT_SLOPE,$HIVE_WEIGHT_INTERCEPT,$ENABLE_BEECOUNTER,$CAMERATYPE,$COUNTERTYPE));
    sleep(1);



    // Refresh the fields in the form
    $sth = $conn->prepare("SELECT * FROM hiveconfig");
    $sth->execute();
    $result = $sth->fetch(PDO::FETCH_ASSOC);
    
    // Tell user it saved
    echo '<div class="alert alert-success alert-dismissable">
                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>';
    echo 'Successfully Saved';
    echo '</div>';
} else {
    // Errors
     echo '<div class="alert alert-danger">';
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
                        <form method="POST" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]);?>">
                        <div class="panel-body">
                            <div class="dataTable_wrapper">
                                <table class="table table-striped table-bordered table-hover" id="dataTables-example">
                                    <thead>
                                        <tr>
                                            <th>Instrument</th>
                                            <th>On/Off</th>
                                            <th>Device</th>
                                            <th>Options</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                       
                                        <tr class="odd gradeX">
                                            <td>
                                            <a href="#" title="Temp/Hum Sensors" data-toggle="popover" data-placement="bottom" data-content="Specify which Temp/Humidity Sensor you have installed"><p class="fa fa-question-circle fa-fw"></P></a>
                                                 Temp/Hum Sensors</td>
                        
                                            <td>
                                            <select name="ENABLE_HIVE_TEMP_CHK">
                                            <option value="yes" <?php if ($result['ENABLE_HIVE_TEMP_CHK'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                                            <option value="no" <?php if ($result['ENABLE_HIVE_TEMP_CHK'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                                            </select></td>
                                            <td>
                                            <?php if ($result['ENABLE_HIVE_TEMP_CHK'] == "yes") {
                                                echo '
                                                <input type="radio" name="TEMPTYPE" value="temperhum"'; if ($result['TEMPTYPE'] == "temperhum") {echo "checked";} echo '> TemperHum
                                                <br><input type="radio" name="TEMPTYPE" value="dht22"'; if ($result['TEMPTYPE'] == "dht22") {echo "checked";} echo '> DHT22 
                                                <br><input type="radio" name="TEMPTYPE" value="dht21"'; if ($result['TEMPTYPE'] == "dht21") {echo "checked";} echo '> DHT21';
                                            }
                                            ?></td>
                                            <td>
                                            <?PHP if ($result['ENABLE_HIVE_TEMP_CHK'] == "yes") {
                                                    if ($result['TEMPTYPE'] == "temperhum") {
                                                        echo '<br><a href="#" title="Specify Device" data-toggle="popover" data-placement="bottom" data-content="Specify the device you want to use (usually /dev/hidraw1, if you only have one device. Use
                                                        temperhum -e from the console to see choices)"><p class="fa fa-question-circle fa-fw"></P></a>Device:
                                                         <input type="text" name="HIVEDEVICE" value="'; echo $result['HIVEDEVICE']; echo '"">';
                                                    }
                                                    if ($result['TEMPTYPE'] == "dht22") {
                                                        echo '<br><a href="#" title="Specify GPIO" data-toggle="popover" data-placement="bottom" data-content="Specify the GPIO you want to use to connect to this sensor"><p class="fa fa-question-circle fa-fw"></P></a>GPIO:
                                                         <input type="text" name="HIVE_TEMP_GPIO" value="'; echo $result['HIVE_TEMP_GPIO']; echo '"">';
                                                    }
                                                    if ($result['TEMPTYPE'] == "dht21") {
                                                        echo '<br><a href="#" title="Specify GPIO" data-toggle="popover" data-placement="bottom" data-content="Specify the GPIO you want to use to connect to this sensor"><p class="fa fa-question-circle fa-fw"></P></a>GPIO:
                                                         <input type="text" name="HIVE_TEMP_GPIO" value="'; echo $result['HIVE_TEMP_GPIO']; echo '"">';
                                                    }
                                            }?>
                                            </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>
                                            <a href="#" title="Weight Scale" data-toggle="popover" data-placement="bottom" data-content="Enable/Disable Weight Checking, then specify which board you have installed"><p class="fa fa-question-circle fa-fw"></P></a>
                                                 Weight Scale</td>
                                            <td>
                                            <select name="ENABLE_HIVE_WEIGHT_CHK">
                                            <option value="yes" <?php if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                                            <option value="no" <?php if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                                            </select></td>
                                            <td>
                                            <?php if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "yes") {
                                                echo '
                                                <input type="radio" name="SCALETYPE" value="phidget1046"'; if ($result['SCALETYPE'] == "phidget1046") {echo "checked";} echo '> Phidget 1046
                                                <br><input type="radio" name="SCALETYPE" value="hx711"'; if ($result['SCALETYPE'] == "hx711") {echo "checked";} echo '> HX711
                                                <br><input type="radio" name="SCALETYPE" value="cpw200plus"'; if ($result['SCALETYPE'] == "cpw200plus") {echo "checked";} echo '> CPW 200 Plus
                                                <br><input type="radio" name="SCALETYPE" value="None"'; if ($result['SCALETYPE'] == "None") {echo 'checked';} echo '> None';
                                            }
                                            ?>
                                            </td>
                                            <td>
                                                 <?PHP if ($result['ENABLE_HIVE_WEIGHT_CHK'] == "yes") {
                                                    
                                                    echo '<br><a href="#" title="Define Zero/Intercept" data-toggle="popover" data-placement="bottom" data-content="Specify the Zero or Intercept value - see hivetool.org for instructions
                                                        "><p class="fa fa-question-circle fa-fw"></P></a>Zero/Intercept:
                                                         <input type="text" name="HIVE_WEIGHT_INTERCEPT" value="'; echo $result['HIVE_WEIGHT_INTERCEPT']; echo '"">';

                                                    echo '<br><a href="#" title="Specify Cali/Slope" data-toggle="popover" data-placement="bottom" data-content="Specify the Calibration or Slope value - see hivetool.org for instructions"><p class="fa fa-question-circle fa-fw"></P></a>Calibration/Slope:
                                                         <input type="text" name="HIVE_WEIGHT_SLOPE" value="'; echo $result['HIVE_WEIGHT_SLOPE']; echo '""><br>';
                                                         $checkscale = $result['SCALETYPE'];
                                                         if ($checkscale == "hx711") {
                                                        echo '<a href="/admin/hx_wiz.php"><button class="btn btn-success" type="button">Calibration Wizard </button></a>';

                                                         } elseif ( $checkscale == "phidget1046") {
                                                             #echo '<a href="/admin/hx_wiz.php"><button class="btn btn-success" type="button">Calibration Wizard </button></a>';
                                                         }
                                            }?>

                                            </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>
                                            <a href="#" title="Light Source" data-toggle="popover" data-placement="bottom" data-content="Enable/Disable local light meters, then specify which board you have installed"><p class="fa fa-question-circle fa-fw"></P></a>
                                                 Light Source</td>
                                            <td>
                                            <select name="ENABLE_LUX">
                                            <option value="yes" <?php if ($result['ENABLE_LUX'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                                            <option value="no" <?php if ($result['ENABLE_LUX'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                                            </select></td>
                                            <td>
                                            <?php if ($result['ENABLE_LUX'] == "yes") {
                                                echo '
                                                <input type="radio" name="LUX_SOURCE" value="tsl2591"'; if ($result['LUX_SOURCE'] == "tsl2591") {echo "checked";} echo '> TSL 2591
                                                <br><input type="radio" name="LUX_SOURCE" value="tsl2561"'; if ($result['LUX_SOURCE'] == "tsl2561") {echo "checked";} echo '> TSL 2561
                                                <br><input type="radio" name="LUX_SOURCE" value="wx"'; if ($result['LUX_SOURCE'] == "wx") {echo "checked";} echo '> WX Station';
                                            }
                                            ?>
                                            </td>
                                            <td></td>
                                        </tr>

                                        <tr class="odd gradeX">
                                            <td>
                                                <a href="#" title="Bee Counter" data-toggle="popover" data-placement="bottom" data-content="Enable Bee Counters and specify the type of Bee Counter you are using"><p class="fa fa-question-circle fa-fw"></P></a>
                                                Bee Counter</td>
                                            <td>
                                            <select name="ENABLE_BEECOUNTER">
                                            <option value="yes" <?php if ($result['ENABLE_BEECOUNTER'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                                            <option value="no" <?php if ($result['ENABLE_BEECOUNTER'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                                            </select></td>

                                            <td>
                                            <?php if ($result['ENABLE_BEECOUNTER'] == "yes") {
                                                echo '
                                                <input type="radio" name="COUNTERTYPE" value="PICAMERA"'; if ($result['COUNTERTYPE'] == "PICAMERA") {echo "checked";} echo '> PI Camera
                                                <br><input type="radio" name="COUNTERTYPE" value="GATES"'; if ($result['COUNTERTYPE'] == "GATES") {echo "checked";} echo '> Gates';}
                                            ?>
                                            </td>
                                            <td></td>
                                        </tr>

                                        <tr class="odd gradeX">
                                            <td>
                                                <a href="#" title="Camera Type" data-toggle="popover" data-placement="bottom" data-content="Specify the type of camera you are using"><p class="fa fa-question-circle fa-fw"></P></a>
                                                Camera</td>
                                            <td>
                                            <select name="ENABLE_HIVE_CAMERA">
                                            <option value="yes" <?php if ($result['ENABLE_HIVE_CAMERA'] == "yes") {echo "selected='selected'";} ?>>Enabled</option>
                                            <option value="no" <?php if ($result['ENABLE_HIVE_CAMERA'] == "no") {echo "selected='selected'";} ?>>Disabled</option>
                                            </select></td>

                                            <td>
                                            <?php if ($result['ENABLE_HIVE_CAMERA'] == "yes") {
                                                echo '
                                                <input type="radio" name="CAMERATYPE" value="PI"'; if ($result['CAMERATYPE'] == "PI") {echo "checked";} echo '> PI Camera
                                                <br><input type="radio" name="CAMERATYPE" value="USB"'; if ($result['CAMERATYPE'] == "USB") {echo "checked";} echo '> USB Type';
                                            }
                                            ?>
                                            </td>
                                            <td></td>
                                        </tr>
        
    
                                        <tr class="odd gradeX">
                                        <td><button type="submit" class="btn btn-success">Save </button></td>
                                       </tr>
                                
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

<script>
$(document).ready(function(){
    $('[data-toggle="popover"]').popover(); 
});
</script>

    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>

</html>
