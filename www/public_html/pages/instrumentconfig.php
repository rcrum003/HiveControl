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
$v->rule('in', ['ENABLE_HIVE_WEIGHT', 'ENABLE_LUX'], ['no', 'yes']);


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

<head>

    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">

    <title>Hive Control - Edit Instruments</title>

    <!-- Bootstrap Core CSS -->
    <link href="../bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">

    <!-- MetisMenu CSS -->
    <link href="../bower_components/metisMenu/dist/metisMenu.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="../dist/css/sb-admin-2.css" rel="stylesheet">

    <!-- Custom Fonts -->
    <link href="../bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->

</head>

<body>

<?PHP include "../include/db-connect.php" ?>

    <div id="wrapper">

        <!-- Navigation -->
       <?PHP include "../include/navigation.php" ?>
        <!-- /Navigation -->

        <div id="page-wrapper">
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

    $ENABLE_HIVE_CAMERA = test_input($_POST["ENABLE_HIVE_CAMERA"]);
    $ENABLE_HIVE_WEIGHT_CHK = test_input($_POST["ENABLE_HIVE_WEIGHT_CHK"]);
    $ENABLE_HIVE_TEMP_CHK = test_input($_POST["ENABLE_HIVE_TEMP_CHK"]);    
    $SCALETYPE = test_input($_POST["SCALETYPE"]);
    $TEMPTYPE = test_input($_POST["TEMPTYPE"]);
    $HIVEDEVICE = test_input($_POST["HIVEDEVICE"]);
    $ENABLE_LUX = test_input($_POST["ENABLE_LUX"]);
    $LUX_SOURCE = test_input($_POST["LUX_SOURCE"]);

  // Get current version    
    $ver = $conn->prepare("SELECT version FROM hiveconfig");
    $ver->execute();
    $ver = $ver->fetchColumn();
// Increment version
    $version = ++$ver;

    // Update into the DB
    $doit = $conn->prepare("UPDATE hiveconfig SET ENABLE_HIVE_CAMERA=?,ENABLE_HIVE_WEIGHT_CHK=?,ENABLE_HIVE_TEMP_CHK=?,SCALETYPE=?,TEMPTYPE=?,version=?,HIVEDEVICE=?,ENABLE_LUX=?,LUX_SOURCE=? WHERE id=1");
    $doit->execute(array($ENABLE_HIVE_CAMERA,$ENABLE_HIVE_WEIGHT_CHK,$ENABLE_HIVE_TEMP_CHK,$SCALETYPE,$TEMPTYPE,$version,$HIVEDEVICE,$ENABLE_LUX,$LUX_SOURCE));
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
                                                <br><input type="radio" name="TEMPTYPE" value="dht21"'; if ($result['TEMPTYPE'] == "dht22") {echo "checked";} echo '> DHT21';
                                            }
                                            ?></td>
                                            <td>
                                            <?PHP if ($result['ENABLE_HIVE_TEMP_CHK'] == "yes") {
                                                    if ($result['TEMPTYPE'] == "temperhum") {
                                                        echo '<br><a href="#" title="Specify Device" data-toggle="popover" data-placement="bottom" data-content="Specify the device you want to use (usually /dev/hidraw1, if you only have one device. Use
                                                        temperhum -e from the console to see choices)"><p class="fa fa-question-circle fa-fw"></P></a>Device:
                                                         <input type="text" name="HIVEDEVICE" value="'; echo $result['HIVEDEVICE']; echo '"">';
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
                                            <td></td>
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
                                            <td>
                                                <?PHP
                                                if ($result['ENABLE_HIVE_CAMERA'] == "yes") {
                                                echo '
                                                <a href="#" title="Camera Mode" data-toggle="popover" data-placement="bottom" data-content="Specify what operation mode you want to run. In Counter mode, it operates as an In/Out counter. In Webcam, it streams a live picture to the dashboard"><p class="fa fa-question-circle fa-fw"></P></a>Camera Mode
                                                <select name="CAMERAMODE">
                                                <option value="COUNTER"'; if ($result['CAMERAMODE'] == "COUNTER") {echo "selected='selected'";} echo '>Bee Counter</option>
                                                <option value="WEBCAM"'; if ($result['CAMERAMODE'] == "WEBCAM") {echo "selected='selected'";} echo '>Webcam</option>
                                            </select>';
                                            }
                                                ?>

                                            </td>
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
