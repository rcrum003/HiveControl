<?PHP

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';


//Check input for badness
function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}


$command = test_input($_GET["command"]);

#Check to see if the neccessary variables exist
if(isset($_GET["command"])) {
    // type exists
    if (empty($_GET["command"])) {
        // Default to nothing if no command is set or empty
        $command = "";
        } else {
            $command = test_input($_GET["command"]);
        }
    } else {
    $command = "";
}


switch ($command) {
    case "clearlogs":
        $sth = $conn->prepare("DELETE from logs");
        $sth->execute();
        $now = date('Y-m-d H:i:s');
        $user_ip = getUserIP();
        loglocal($now, "WEBADMIN", "INFO", "Logs cleared by Admin from source IP $user_ip");
        break;
    }


function loglocal($date, $program, $type, $message) {
  #Stores log entries locally
# This script takes 4 inputs and puts them into the DB
# 1 - Date - 
# 2 - Program
# 3 - Type (Error, Success, Warning)
# 4 - Message (Optional)

        include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
        $sth = $conn->prepare("insert into logs (date,program,type,message) values (\"$date\",\"$program\",\"$type\",\"$message\")");
        $sth->execute();
}

function getUserIP()
{
    $client  = @$_SERVER['HTTP_CLIENT_IP'];
    $forward = @$_SERVER['HTTP_X_FORWARDED_FOR'];
    $remote  = $_SERVER['REMOTE_ADDR'];

    if(filter_var($client, FILTER_VALIDATE_IP))
    {
        $ip = $client;
    }
    elseif(filter_var($forward, FILTER_VALIDATE_IP))
    {
        $ip = $forward;
    }
    else
    {
        $ip = $remote;
    }

    return $ip;
}

if ($_SERVER["REQUEST_METHOD"] == "GET") {
$sth = $conn->prepare("SELECT * FROM logs ORDER by date DESC LIMIT 1000");
$sth->execute();
$result = $sth->fetchall(PDO::FETCH_ASSOC);

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
                    <h1 class="page-header">System Logs</h1>
                </div>
                <!-- /.col-lg-12 -->
            </div>
            <!-- /.row -->
            <!-- /.row -->
           <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <!-- /.panel-heading -->
                        <div class="panel-body">
                            <div class="dataTable_wrapper">
                            <a href="/admin/system.php?command=clearlogs"><button type="button" class="btn btn btn-primary">Clear Logs</button></a>
                                <table width="100%" class="table table-striped table-bordered table-hover" id="dataTables-example">

                                    <thead>
                                        <tr>
                                            <th>Date</th>
                                            <th>Program</th>
                                            <th>Type</th>
                                            <th>Message</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                    <?PHP foreach($result as $r){
                                        echo '<tr><td>'.$r['date'].'</td>';
                                        echo '<td>'.$r['program'].'</td>';
                                        echo '<td>'.$r['type'].'</td>';
                                        echo '<td>'.$r['message'].'</td></tr>';
                                    }
                                    ?>
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

  <!-- DataTables JavaScript -->
    <script src="../bower_components/datatables/media/js/jquery.dataTables.min.js"></script>
    <script src="../bower_components/datatables-plugins/integration/bootstrap/3/dataTables.bootstrap.min.js"></script>
    <script src="../bower_components/datatables-responsive/js/dataTables.responsive.js"></script>

    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

        <script>
    $(document).ready(function() {
        $('#dataTables-example').DataTable({
                responsive: true,
                 "order": [[ 0, "desc" ]]
        });
    });
    </script>

</body>

</html>
