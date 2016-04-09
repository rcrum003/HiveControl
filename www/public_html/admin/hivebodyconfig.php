<?PHP

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

if ($_SERVER["REQUEST_METHOD"] == "GET") {
$sth = $conn->prepare("SELECT * from hiveconfig INNER JOIN hiveequipmentweight on hiveconfig.id=hiveequipmentweight.id");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);



}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
// Set Error Fields

$v = new Valitron\Validator($_POST);
$v->rule('in', ['ENABLE_HIVE_WEIGHT'], ['no', 'yes']);

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
                    <h1 class="page-header">Edit Hive Body Configuration</h1>
                </div>
                <!-- /.col-lg-12 -->
            </div>
            <!-- /.row -->
           <?PHP
        if ($_SERVER["REQUEST_METHOD"] == "POST") {
        
if($v->validate()) {

// Get current version    
   $ver = $conn->prepare("SELECT version FROM hiveconfig");
    $ver->execute();
    $ver = $ver->fetchColumn();
// Increment version
    $version = ++$ver;
    sleep(1);
    
// Make sure no bad stuff got through our filters
// Probably not needed, but doesn't hurt
$HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT = test_input($_POST["HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT"]);
$HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT= test_input($_POST["HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT"]);
$HIVE_FEEDER_WEIGHT= test_input($_POST["HIVE_FEEDER_WEIGHT"]);
$HIVE_TOP_INNER_COVER_WEIGHT= test_input($_POST["HIVE_TOP_INNER_COVER_WEIGHT"]);
$HIVE_TOP_TELE_COVER_WEIGHT= test_input($_POST["HIVE_TOP_TELE_COVER_WEIGHT"]);
$HIVE_TOP_MIGRATORY_COVER_WEIGHT= test_input($_POST["HIVE_TOP_MIGRATORY_COVER_WEIGHT"]);
$HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT= test_input($_POST["HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT"]);
$HIVE_BODY_DEEP_FOUNDATION_WEIGHT= test_input($_POST["HIVE_BODY_DEEP_FOUNDATION_WEIGHT"]);
$HIVE_BODY_SHAL_FOUNDATION_WEIGHT= test_input($_POST["HIVE_BODY_SHAL_FOUNDATION_WEIGHT"]);
$HIVE_TOP_WEIGHT= test_input($_POST["HIVE_TOP_WEIGHT"]);
$HIVE_COMPUTER_WEIGHT= test_input($_POST["HIVE_COMPUTER_WEIGHT"]);
$HIVE_MISC_WEIGHT= test_input($_POST["HIVE_MISC_WEIGHT"]);
$NUM_HIVE_BASE_SOLID_BOTTOM_BOARD= test_input($_POST["NUM_HIVE_BASE_SOLID_BOTTOM_BOARD"]);
$NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD= test_input($_POST["NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD"]);
$NUM_HIVE_FEEDER= test_input($_POST["NUM_HIVE_FEEDER"]);
$NUM_HIVE_TOP_INNER_COVER= test_input($_POST["NUM_HIVE_TOP_INNER_COVER"]);
$NUM_HIVE_TOP_TELE_COVER= test_input($_POST["NUM_HIVE_TOP_TELE_COVER"]);
$NUM_HIVE_TOP_MIGRATORY_COVER= test_input($_POST["NUM_HIVE_TOP_MIGRATORY_COVER"]);
$NUM_HIVE_BODY_MEDIUM_FOUNDATION= test_input($_POST["NUM_HIVE_BODY_MEDIUM_FOUNDATION"]);
$NUM_HIVE_BODY_DEEP_FOUNDATION= test_input($_POST["NUM_HIVE_BODY_DEEP_FOUNDATION"]);
$NUM_HIVE_BODY_SHAL_FOUNDATION= test_input($_POST["NUM_HIVE_BODY_SHAL_FOUNDATION"]);

 
    
    // Update Hive Weights into the DB
    $doit = $conn->prepare("UPDATE hiveequipmentweight SET HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT=?,HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT=?,HIVE_FEEDER_WEIGHT=?,HIVE_TOP_INNER_COVER_WEIGHT=?,
    HIVE_TOP_TELE_COVER_WEIGHT=?, HIVE_TOP_MIGRATORY_COVER_WEIGHT=?, HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT=?, HIVE_BODY_DEEP_FOUNDATION_WEIGHT=?, 
    HIVE_BODY_SHAL_FOUNDATION_WEIGHT=?, HIVE_TOP_WEIGHT=?, HIVE_COMPUTER_WEIGHT=?, HIVE_MISC_WEIGHT=? WHERE id=1");
    $doit->execute(array($HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT,$HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT,$HIVE_FEEDER_WEIGHT,$HIVE_TOP_INNER_COVER_WEIGHT,$HIVE_TOP_TELE_COVER_WEIGHT,$HIVE_TOP_MIGRATORY_COVER_WEIGHT,$HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT,$HIVE_BODY_DEEP_FOUNDATION_WEIGHT,$HIVE_BODY_SHAL_FOUNDATION_WEIGHT,$HIVE_TOP_WEIGHT,$HIVE_COMPUTER_WEIGHT,$HIVE_MISC_WEIGHT));
    sleep(1);

    // Update Number of componets 

    $doit2 = $conn->prepare("UPDATE hiveconfig SET version=?, NUM_HIVE_BASE_SOLID_BOTTOM_BOARD=?,
    NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD=?, NUM_HIVE_FEEDER=?, NUM_HIVE_TOP_INNER_COVER=?,  NUM_HIVE_TOP_TELE_COVER=?, NUM_HIVE_TOP_MIGRATORY_COVER=?, 
    NUM_HIVE_BODY_MEDIUM_FOUNDATION=?, NUM_HIVE_BODY_DEEP_FOUNDATION=?, 
    NUM_HIVE_BODY_SHAL_FOUNDATION=? WHERE id=1");
    $doit2->execute(array($version, $NUM_HIVE_BASE_SOLID_BOTTOM_BOARD,$NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD,$NUM_HIVE_FEEDER,$NUM_HIVE_TOP_INNER_COVER,$NUM_HIVE_TOP_TELE_COVER,$NUM_HIVE_TOP_MIGRATORY_COVER,$NUM_HIVE_BODY_MEDIUM_FOUNDATION,$NUM_HIVE_BODY_DEEP_FOUNDATION,$NUM_HIVE_BODY_SHAL_FOUNDATION));
    sleep(1);

    // Refresh the fields in the form
    $sth2 = $conn->prepare("SELECT * from hiveconfig INNER JOIN hiveequipmentweight on hiveconfig.id=hiveequipmentweight.id");
    $sth2->execute();
    $result = $sth2->fetch(PDO::FETCH_ASSOC);
    
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
                        <div class="panel-heading">
                        
                        </div>
                        <!-- /.panel-heading -->
                         <form method="POST" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]);?>">
                        <div class="panel-body">
                            <div class="dataTable_wrapper">
                                <table class="table table-striped table-bordered table-hover" id="dataTables-example">
                                    <thead>
                                        <tr>
                                            <th>Component</th>
                                            <th>Base Weight</th>
                                            <th># Installed on Hive</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                        <tr class="odd gradeX">
                                            <td>Hive Top Weight</td>
                                            <td><input type="text" name="HIVE_TOP_WEIGHT" value="<?PHP echo $result['HIVE_TOP_WEIGHT']; ?>"></td>
                                            <td></td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Telescoping Cover<br><img src="/images/hive/telecover.png" width="175" height="65"></td>
                                            <td><input type="text" name="HIVE_TOP_TELE_COVER_WEIGHT" value="<?PHP echo $result['HIVE_TOP_TELE_COVER_WEIGHT']; ?>"></td>
                                            <td><input type="text" name="NUM_HIVE_TOP_TELE_COVER" value="<?PHP echo $result['NUM_HIVE_TOP_TELE_COVER']; ?>"></td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Migratory Cover<br><img src="/images/hive/migratorytop.png"></td>
                                            <td><input type="text" name="HIVE_TOP_MIGRATORY_COVER_WEIGHT" value="<?PHP echo $result['HIVE_TOP_MIGRATORY_COVER_WEIGHT']; ?>"></td>
                                            <td><input type="text" name="NUM_HIVE_TOP_MIGRATORY_COVER" value="<?PHP echo $result['NUM_HIVE_TOP_MIGRATORY_COVER']; ?>"></td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Inner Cover<br><img src="/images/hive/innercover.png" width="175" height="124"></td>
                                            <td><input type="text" name="HIVE_TOP_INNER_COVER_WEIGHT" value="<?PHP echo $result['HIVE_TOP_INNER_COVER_WEIGHT']; ?>"></td>
                                            <td><input type="text" name="NUM_HIVE_TOP_INNER_COVER" value="<?PHP echo $result['NUM_HIVE_TOP_INNER_COVER']; ?>"></td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Hive Feeder<br><img src="/images/hive/feeder.png" width="175" height="124"></td>
                                            <td><input type="text" name="HIVE_FEEDER_WEIGHT" value="<?PHP echo $result['HIVE_FEEDER_WEIGHT']; ?>"></td>
                                            <td><input type="text" name="NUM_HIVE_FEEDER" value="<?PHP echo $result['NUM_HIVE_FEEDER']; ?>"></td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Shallow - w/Frames<br><img src="/images/hive/shallow.png"></td>
                                            <td><input type="text" name="HIVE_BODY_SHAL_FOUNDATION_WEIGHT" value="<?PHP echo $result['HIVE_BODY_SHAL_FOUNDATION_WEIGHT']; ?>"></td>
                                            <td><input type="text" name="NUM_HIVE_BODY_SHAL_FOUNDATION" value="<?PHP echo $result['NUM_HIVE_BODY_SHAL_FOUNDATION']; ?>"></td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Medium - w/Frames<br><img src="/images/hive/medium-w-frames.png"></td>
                                            <td><input type="text" name="HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT" value="<?PHP echo $result['HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT']; ?>"></td>
                                            <td><input type="text" name="NUM_HIVE_BODY_MEDIUM_FOUNDATION" value="<?PHP echo $result['NUM_HIVE_BODY_MEDIUM_FOUNDATION']; ?>"></td>
                                        </tr>
                                       
                                        <tr class="odd gradeX">
                                            <td>Deep - w/Frames<br><img src="/images/hive/deep.png"></td>
                                            <td><input type="text" name="HIVE_BODY_DEEP_FOUNDATION_WEIGHT" value="<?PHP echo $result['HIVE_BODY_DEEP_FOUNDATION_WEIGHT']; ?>"></td>
                                            <td><input type="text" name="NUM_HIVE_BODY_DEEP_FOUNDATION" value="<?PHP echo $result['NUM_HIVE_BODY_DEEP_FOUNDATION']; ?>"></td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Solid Bottom Board<br><img src="/images/hive/solidbottomboard.png"></td>
                                            <td><input type="text" name="HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT" value="<?PHP echo $result['HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT']; ?>"></td>
                                            <td><input type="text" name="NUM_HIVE_BASE_SOLID_BOTTOM_BOARD" value="<?PHP echo $result['NUM_HIVE_BASE_SOLID_BOTTOM_BOARD']; ?>"></td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Screened Bottom Board<br><img src="/images/hive/screenedbottomboard2.png"></td>
                                            <td><input type="text" name="HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT" value="<?PHP echo $result['HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT']; ?>"></td>
                                            <td><input type="text" name="NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD" value="<?PHP echo $result['NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD']; ?>"></td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Computer Weight</td>
                                            <td><input type="text" name="HIVE_COMPUTER_WEIGHT" value="<?PHP echo $result['HIVE_COMPUTER_WEIGHT']; ?>"></td>
                                            <td></td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Misc Weight</td>
                                            <td><input type="text" name="HIVE_MISC_WEIGHT" value="<?PHP echo $result['HIVE_MISC_WEIGHT']; ?>"></td>
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


    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>

</html>
