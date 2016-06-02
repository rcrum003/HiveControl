<?PHP

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

# Get some config parameters

if ($_SERVER["REQUEST_METHOD"] == "GET") {


$sth2 = $conn->prepare("SELECT * FROM notifications");
$sth2->execute();
$notifications = $sth2->fetchall(PDO::FETCH_ASSOC);


}



//Check input for badness
function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}


#Get system time
$shortName = exec('date +%Z');
$longName = timezone_name_from_abbr($shortName);
date_default_timezone_set($longName);

$now = date('Y-m-d H:i:s');


#Check to see if the neccessary variables exist

?>
<!DOCTYPE html>
<html lang="en">


    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

    <div id="wrapper">
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Notifications</h1>
                </div>
                <!-- /.col-lg-12 -->
            </div>
            <!-- /.row -->
            <!-- /.row -->
           <div class="row">
                <div class="col-lg-12">
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
                    <div class="panel panel-default">
                        <!-- /.panel-heading -->
                        <div class="panel-body">
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
                
            </div>
                                

                            </div>
                    </div> <!-- /.notifications -->
                            <!-- /.table-responsive -->
                        </div>
                        <!-- /.panel-body -->
                    </div>
            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-body">
                        
                        
                        </div>
                    </div>
                </div>
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
    </div>
    </div>
    </div>
    </div>

                <div class="modal fade" id="NewModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true">
                                <div class="modal-dialog">
                                    <div class="modal-content">
                                        <div class="modal-header">
                                            <button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>
                                            <h4 class="modal-title" id="myModalLabel">Please Confirm</h4>
                                        </div>
                                        <div class="modal-body">
                                            New Form Here
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
                            <!-- /.modal -->


    </body>
    <!-- /#wrapper -->

    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>

    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>
      <!-- DataTables JavaScript -->
    <script src="../bower_components/datatables/media/js/jquery.dataTables.min.js"></script>
    <script src="../bower_components/datatables-plugins/integration/bootstrap/3/dataTables.bootstrap.min.js"></script>
    <script src="../bower_components/datatables-responsive/js/dataTables.responsive.js"></script>

    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

    <script>
    // tooltip demo
    $('.tooltip-demo').tooltip({
        selector: "[data-toggle=tooltip]",
        container: "body"
    })
    // popover demo
    $("[data-toggle=popover]")
        .popover()
    </script>
<script>
    $(document).ready(function() {
        $('#dataTables-example').DataTable({
                responsive: true,
                 "order": [[ 0, "desc" ]]
        });
    });
    </script>

<script>
 $(document).ready(function() {
     $("#myDiv").load("statusrefresh.php?id=<?PHP echo $msgid; ?>");
   var refreshId = setInterval(function() {
      $("#myDiv").load('statusrefresh.php?id=<?PHP echo $msgid; ?>&rand='+ Math.random());
   }, 9000);
   $.ajaxSetup({ cache: false });
});
</script>
    


</html>
