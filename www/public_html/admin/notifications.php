<?PHP

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

if ($_SERVER["REQUEST_METHOD"] == "GET") {


$sth2 = $conn->prepare("SELECT * FROM notifications");
$sth2->execute();
$notifications = $sth2->fetchall(PDO::FETCH_ASSOC);


}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
// Set Error Fields

$v = new Valitron\Validator($_POST);
$v->rule('required', ['ENABLE_HIVE_CAMERA', 'ENABLE_HIVE_WEIGHT_CHK', 'ENABLE_HIVE_TEMP_CHK', 'ENABLE_LUX'], 1)->message('{field} is required');
$v->rule('slug', ['WXTEMPTYPE']);
$v->rule('in', ['ENABLE_HIVE_WEIGHT', 'ENABLE_LUX', 'ENABLE_HIVE_CAMERA', 'ENABLE_HIVE_WEIGHT_CHK', 'ENABLE_HIVE_TEMP_CHK', 'ENABLE_BEECOUNTER'], ['no', 'yes']);
$v->rule('integer', ['HIVE_TEMP_GPIO'], 1)->message('{field} must be a integer');
$v->rule('numeric', ['HIVE_WEIGHT_SLOPE', 'HIVE_WEIGHT_INTERCEPT'], 1)->message('{field} must be numeric');
$v->rule('alphaNum', ['SCALETYPE', 'TEMPTYPE', 'LUX_SOURCE', 'COUNTERTYPE', 'CAMERATYPE', 'local_wx_type'], 1)->message('{field} must be alphaNum only');
$v->rule('lengthmax', ['WX_TEMP_GPIO'], 2);


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
                    <h1 class="page-header">Notifications </h1>
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
    $weather_level = test_input($_POST["WEATHER_LEVEL"]);
    $weather_detail = test_input($_POST["WEATHER_DETAIL"]);
    $key = test_input($_POST["KEY"]);
    $wxstation = test_input($_POST["WXSTATION"]);
    $WXTEMPTYPE = test_input($_POST["WXTEMPTYPE"]);
    $WX_TEMPER_DEVICE = test_input_allow_slash($_POST["WX_TEMPER_DEVICE"]);
    $WX_TEMP_GPIO = test_input($_POST["WX_TEMP_GPIO"]);
    $local_wx_type = test_input($_POST["local_wx_type"]);
    $local_wx_url = test_input($_POST["local_wx_url"]);

  // Get current version    
    $ver = $conn->prepare("SELECT version FROM hiveconfig");
    $ver->execute();
    $ver = $ver->fetchColumn();
// Increment version
    $version = ++$ver;

    // Update into the DB
    $doit = $conn->prepare("UPDATE hiveconfig SET ENABLE_HIVE_CAMERA=?,ENABLE_HIVE_WEIGHT_CHK=?,ENABLE_HIVE_TEMP_CHK=?,SCALETYPE=?,TEMPTYPE=?,version=?,HIVEDEVICE=?,ENABLE_LUX=?,LUX_SOURCE=?,HIVE_TEMP_GPIO=?,HIVE_WEIGHT_SLOPE=?,HIVE_WEIGHT_INTERCEPT=?,ENABLE_BEECOUNTER=?,CAMERATYPE=?,COUNTERTYPE=?,weather_level=?,key=?,wxstation=?,WXTEMPTYPE=?,WX_TEMPER_DEVICE=?,WX_TEMP_GPIO=?,weather_detail=?,local_wx_type=?,local_wx_url=? WHERE id=1");
    $doit->execute(array($ENABLE_HIVE_CAMERA,$ENABLE_HIVE_WEIGHT_CHK,$ENABLE_HIVE_TEMP_CHK,$SCALETYPE,$TEMPTYPE,$version,$HIVEDEVICE,$ENABLE_LUX,$LUX_SOURCE,$HIVE_TEMP_GPIO,$HIVE_WEIGHT_SLOPE,$HIVE_WEIGHT_INTERCEPT,$ENABLE_BEECOUNTER,$CAMERATYPE,$COUNTERTYPE,$weather_level,$key,$wxstation,$WXTEMPTYPE,$WX_TEMPER_DEVICE,$WX_TEMP_GPIO,$weather_detail,$local_wx_type,$local_wx_url));
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
                            <!-- Nav tabs -->
                            <ul class="nav nav-tabs">
                                <li class="active"><a href="#notifications" data-toggle="tab">Notifications</a>
                                </li>
                                <li><a href="#settings" data-toggle="tab">Settings</a>
                                </li>
                            </ul>

                            <!-- Tab panes -->
                            <div class="tab-content">
                                <div class="tab-pane fade in active" id="notifications">
                                    <P>
                                    <div class="dataTable_wrapper">
                                <table class="table table-striped table-bordered table-hover" id="dataTables-example">
                                    <thead>
                                        <tr>
                                            <th>ID</th>
                                            <th>Name</th>
                                            <th>Measure</th>
                                            <th>Threshold Type</th>
                                            <th>Threshold Value</th>
                                            <th>Time Period</th>
                                            <th>Status</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                       <?PHP foreach($notifications as $r){
                                        echo '<tr><td>'.$r['id'].'</td>';
                                        echo '<td>'.$r['name'].'</td>';
                                        echo '<td>'.$r['measure'].'</td>';
                                        echo '<td>'.$r['threshold_type'].'</td>';
                                        echo '<td>'.$r['threshold_value'].'</td>';
                                        echo '<td>'.$r['time_period'].'</td>';
                                        echo '<td>'.$r['status'].'</td></tr>';
                                    }
                                    ?>      
                                    </tbody>
                                </table>
                                <button type="button" class="btn btn-outline btn-default btn-lg" data-toggle="modal" data-target="#NewModal">
                                    <div class="text-center">New</div>
                                </button>

                            </div><!-- /.table-responsive -->
                                </div> <!-- /.notifications -->

                                <div class="tab-pane fade" id="settings">
                                    <p>
                                    <div class="dataTable_wrapper">
                                <table class="table table-striped table-bordered table-hover" id="dataTables-example">
                                    <thead>
                                        <tr>
                                            <th>Setting</th>
                                            <th>Value</th>
                                            <th>Options</th>
                                            
                                        </tr>
                                    </thead>
                                    <tbody>
                                       
                                        <tr class="odd gradeX">
                                            <td>1</td>
                                            <td>2</td>
                                            <td>3</td>
                                            
                                        <tr>        
                                    </tbody>
                                </table>
                            </div> <!-- /.table-responsive -->

                                </div> <!-- /.settings -->
                        
                    </div> <!-- /tab -->
                    
                </div> <!-- /.panel body-->
            </div> <!-- /.panel -->
            
        </div> <!-- /class 12 -->
        
    </div> <!-- /#row -->
</div> <!-- /#page-wrapper -->
        <div class="modal fade" id="NewModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true">
                                <div class="modal-dialog">
                                    <div class="modal-content">
                                        <div class="modal-header">
                                            <button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>
                                            <h4 class="modal-title" id="myModalLabel">New Notification</h4>
                                        </div>
                                        <div class="modal-body">
                                            New Code Here
                                        </div>
                                        <div class="modal-footer">
                                            <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                                            <a href="/admin/system.php?command=cleardata&confirm=yes"><button type="button" class="btn btn-danger">Save</button></a>
                                        </div>
                                    </div>
                                    <!-- /.modal-content -->
                                </div>
                                <!-- /.modal-dialog -->
            </div>

</body>
                
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
    $('[data-toggle="popover"]').popover(); 
    });
    </script>
    
    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>



</html>
