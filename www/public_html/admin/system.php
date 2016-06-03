<?PHP

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

# Get some config parameters


//Check input for badness
function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
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

function loglocal($date, $program, $type, $message) {
  #Stores log entries locally
# This script takes 4 inputs and puts them into the DB
# 1 - Date - 
# 2 - Program
# 3 - Type (Error, Success, Warning)
# 4 - Message (Optional)

        include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
       
        $sth99 = $conn->prepare("insert into logs (date,program,type,message) values (\"$date\",\"$program\",\"$type\",\"$message\")");
        $sth99->execute();
       unset($sth99); 
}

function getlog($conn)
{
    # Pulls latest log file

    $sth1 = $conn->prepare("SELECT * FROM logs ORDER by date DESC LIMIT 1000");
    $sth1->execute();
    $result = $sth1->fetchall(PDO::FETCH_ASSOC);
    unset($sth1); 

    echo '<table width="100%" class="table table-striped table-bordered table-hover" id="dataTables-example">
    <thead>
            <tr>
            <th>Date</th>
            <th>Program</th>
            <th>Type</th>
            <th>Message</th>
            </tr>
            </thead>
            <tbody>';
                 foreach($result as $r){
                    echo '<tr><td>'.$r['date'].'</td>';
                    echo '<td>'.$r['program'].'</td>';
                    echo '<td>'.$r['type'].'</td>';
                    echo '<td>'.$r['message'].'</td></tr>';
                        }
                    echo '</tbody></table>';
}

$command = test_input($_GET["command"]);
$confirm = test_input($_GET["confirm"]);
$table = test_input($_GET["table"]);

#Get system time
$shortName = exec('date +%Z');
$longName = timezone_name_from_abbr($shortName);
date_default_timezone_set($longName);

$now = date('Y-m-d H:i:s');

#Check to see if our main script is set to run
$sth15 = $conn->prepare("SELECT RUN FROM hiveconfig");
$sth15->execute();
$RUN = $sth15->fetchColumn();
unset($sth15);

#Check to see if the neccessary variables exist
if(isset($_GET["command"])) {
    // command exists
    if (empty($_GET["command"])) {
        // Default to nothing if no command is set or empty
        $command = "none";
        } else {
            // Not empty, so let's run switch

        switch ($command) {
            case "upgrade":

                #Check to see is a messgae for upgrade already exists
                #New, processing, complete, error are the expected statuses
                
                $sth2 = $conn->prepare("SELECT * FROM msgqueue WHERE message='upgrade' ORDER BY id DESC LIMIT 1");
                $sth2->execute();
                $result = $sth2->fetch(PDO::FETCH_ASSOC);
                $status = $result['status'];
                unset($sth2);
                $upgrade_response = $result['response'];
                

                if ( $status == "new" ) {
                    $msgid = $result['id'];
                    break;
                } 
                if ( $status == "processing" ) {
                    $msgid = $result['id'];
                    break;
                } 
                if ( $status == "error" ) {
                    $msgid = $result['id'];
                    $error = $upgrade_response;
                }
                $message = "upgrade";
                $status = "new";
                $date = date('Y-m-d H:i:s');
                $sth4 = $conn->prepare("insert into msgqueue (date,message,status) values (\"$date\",\"$message\",\"$status\")");
                $sth4->execute();
                unset($sth4);
                $now = date('Y-m-d H:i:s');

                #Get the Message ID to pass to our refresh script
                $sth5 = $conn->prepare("SELECT * FROM msgqueue WHERE message='upgrade' ORDER BY id DESC LIMIT 1");
                $sth5->execute();
                $result = $sth5->fetch(PDO::FETCH_ASSOC);
                unset($sth5);
                $msgid = $result['id'];
                break;
            
            case "cleardata":

                if ($confirm == "yes") {
                    # well, you confirmed it, deleting all the data
                    $sth6 = $conn->prepare("DELETE from allhivedata");
                    $sth6->execute();
                    $sth7 = $conn->prepare("DELETE from hivedata");
                    $sth7->execute();
                    $sth8 = $conn->prepare("DELETE from weather");
                    $sth8->execute();
                    $sth9 = $conn->prepare("DELETE from gdd");
                    $sth9->execute();
                    $sth10 = $conn->prepare("DELETE from logs");
                    $sth10->execute();
                    $sth11 = $conn->prepare("DELETE from msgqueue");
                    $sth11->execute();
                    $sth12 = $conn->prepare("DELETE from pollen");
                    $sth12->execute();
                    $user_ip = getUserIP();
                    loglocal($now, "RESET", "INFO", "All Data Erased by Admin from source IP $user_ip");
                }
                    
                break;

            case "clearlogs":

                if ( $confirm == "yes" ) {
                    # Confirmed we want to delete
                    $sth13 = $conn->prepare("DELETE from logs");
                    $sth13->execute();
                    unset($sth13);
                    $user_ip = getUserIP();
                    loglocal($now, "LOGS", "INFO", "Logs cleared by Admin from source IP $user_ip");
                    break;
                }
                break;

            case "removezero":

                if ( $confirm == "yes" ) {
                    # Confirmed we want to remove zeros
                    if ( isset($table)) {
                        # need to tell us what table you want remove the zeros for
                    $sth14 = $conn->prepare("DELETE from allhivedata WHERE $table='0'");
                    $sth14->execute();
                    $user_ip = getUserIP();
                    
                    loglocal($now, "REMOVEZERO", "INFO", "Successfully removed zero data for $table by Admin from source IP $user_ip");
                    break;
                    }
                    break;
                }
                break;

            case "pause":
                # pauses current conditions script
                $ver = $conn->prepare("SELECT version FROM hiveconfig");
                $ver->execute();
                $ver = $ver->fetchColumn();
                // Increment version
                $version = ++$ver;
                #$version = "10";
                unset($ver);
                if ( $RUN == "no") {
                    #Set to yes
                    $pause_type = "started";
                    $pause_run = "yes";
                }
                if ( $RUN == "yes") {
                    #set to no
                    $pause_type = "paused";
                    $pause_run = "no";
                }
                  $doit = $conn->prepare("UPDATE hiveconfig SET version=?,RUN=? WHERE id=1");
                  $doit->execute(array($version,$pause_run));
                  unset($doit);
                  sleep(2);
                  loglocal($now, "MAIN", "INFO", "Successfully $pause_type Data Collection by Admin from source IP $user_ip");
                  header('Refresh: 1; url=/admin/system.php');
                break;
        }
        }
    } else {
        $command = "none";
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
                    <h1 class="page-header">System Commands</h1>
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

                <a href="/admin/system.php?command=upgrade">
                <button type="button" class="btn btn-outline btn-default btn-lg">
                                <img src="../images/disk.png" width="75" height="75">
                                    <div class="text-center">Update Code</div>
                </a></button>
                <?PHP if ($command == "clearlogs" || $command == "upgrade" || $command == "cleardata" || $command == "removezero" || $command == "pause" || $command == "server_status") {
                    echo '<a href="/admin/system.php">
                    <button type="button" class="btn btn-outline btn-default btn-lg">
                                <img src="../images/delete.png" width="75" height="75">
                                    <div class="text-center">Show Logs</div>
                        </button></a>'; }
                        else {
                        echo '<button type="button" class="btn btn-outline btn-default btn-lg" data-toggle="modal" data-target="#ClearLogsModal">
                                <img src="../images/delete.png" width="75" height="75">
                                    <div class="text-center">Clear Logs</div>
                        </button>';
                    }
                ?>
                
                <button type="button" class="btn btn-outline btn-default btn-lg" data-toggle="modal" data-target="#ResetDataModal">
                                <img src="../images/emptydb.png" width="75" height="75">
                                    <div class="text-center">Reset All Data</div>
                </button>

                
                <button type="button" class="btn btn-outline btn-default btn-lg" data-toggle="modal" data-target="#ZeroModal">
                                <img src="../images/zerochart.png" width="75" height="75">
                                    <div class="text-center">Remove Zero</div>
                </button>
                <a href="/admin/system.php?command=server_status">
                <button type="button" class="btn btn-outline btn-default btn-lg">
                                <img src="../images/raspberry.png" width="75" height="75">
                                    <div class="text-center">Server Status</div>
                </a></button>
                <a href="/admin/system.php?command=pause">
                    <?PHP if($RUN == "yes") { 
                        #we are running
                        echo '<button type="button" class="btn btn-outline btn-default btn-lg">
                        <img src="../images/pause.png" width="75" height="75">
                        <div class="text-center">Pause</div>';
                    }
                        elseif($RUN=="no") {
                            # we arent running..
                            echo '<button type="button" class="btn btn-danger btn-lg">
                            <img src="../images/play.png" width="75" height="75">
                            <div class="text-center">Start</div>';
                        } ?>

                </a></button>
                
            </div>
                                

                            </div>
                            <!-- /.table-responsive -->
                        </div>
                        <!-- /.panel-body -->
                    </div>
            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-body">
                        <?PHP 
                        if(isset($error)) { 
                            // echo "Last Error Message was: $error";
                            echo '<div class="alert alert-danger alert-dismissable">
                            <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>'; 
                            echo '<b>Upgrade Request '.$msgid.' Errored Out </b><br>'; echo $error; echo '</div>';
                            } 
                        if ($command == "clearlogs" || $command == "cleardata" || $command == "removezero" || $command == "pause" || $command == "none") {
                            getlog($conn);
                        }
                        if ($command == "upgrade") {
                            echo '<div id="myDiv"></script></div>';
                        }
                        if ($command == "server_status") {
                            echo '<pre>';
                            require $_SERVER["DOCUMENT_ROOT"] . '/admin/server_status.txt';
                            echo '</pre>';
                                        }
                            ?>
                        
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

                <div class="modal fade" id="ResetDataModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true">
                                <div class="modal-dialog">
                                    <div class="modal-content">
                                        <div class="modal-header">
                                            <button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>
                                            <h4 class="modal-title" id="myModalLabel">Please Confirm</h4>
                                        </div>
                                        <div class="modal-body">
                                            Are you sure you want to delete ALL data? This data is non-recoverable, if you confirm. 
                                        </div>
                                        <div class="modal-footer">
                                            <button type="button" class="btn btn-default" data-dismiss="modal">No, get me out of here</button>
                                            <a href="/admin/system.php?command=cleardata&confirm=yes"><button type="button" class="btn btn-danger">Yes, DELETE it all!</button></a>
                                        </div>
                                    </div>
                                    <!-- /.modal-content -->
                                </div>
                                <!-- /.modal-dialog -->
                </div>

                <div class="modal fade" id="ClearLogsModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true">
                                <div class="modal-dialog">
                                    <div class="modal-content">
                                        <div class="modal-header">
                                            <button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>
                                            <h4 class="modal-title" id="myModalLabel">Please Confirm</h4>
                                        </div>
                                        <div class="modal-body">
                                            Are you sure you want to clear the logs? 
                                        </div>
                                        <div class="modal-footer">
                                            <button type="button" class="btn btn-default" data-dismiss="modal">No, get me out of here</button>
                                            <a href="/admin/system.php?command=clearlogs&confirm=yes"><button type="button" class="btn btn-danger">Yes, clear it all!</button></a>
                                        </div>
                                    </div>
                                    <!-- /.modal-content -->
                                </div>
                                <!-- /.modal-dialog -->
                </div>

                <div class="modal fade" id="ZeroModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true">
                                <div class="modal-dialog">
                                    <div class="modal-content">
                                        <div class="modal-header">
                                            <button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>
                                            <h4 class="modal-title" id="myModalLabel">Please Select a Trend to Remove Zeros</h4>
                                        </div>
                                        <div class="modal-body">
                                            Warning: we remove zero data by deleting the whole record. So if weight = 0, we will remove the whole entry where weight = 0. If you select a data point that is zero throughout the whole database, every record in your database will be deleted. Use at your own risk.
                                            <p><br><br><b>
                                            Clicking on any of the following will remove any record where it = 0:
                                            </b><p>
                                            <div class="table-responsive">
                                            <table class="table">
                                                <tbody>
                                                    <tr>
                                                        <td><a href="/admin/system.php?command=removezero&confirm=yes&table=hivetempf">Hive Temp</a></td>
                                                        <td><a href="/admin/system.php?command=removezero&confirm=yes&table=hiveHum">Hive Humidity</a></td>
                                                 
                                                    </tr>
                                                    <tr>
                                                       <td><a href="/admin/system.php?command=removezero&confirm=yes&table=hiveweight">Hive Weight</a></td>
                                                        <td><a href="/admin/system.php?command=removezero&confirm=yes&table=IN_COUNT">Flight</a></td>
                                                    
                                                    </tr>
                                                </tbody>
                                            </table>
                            </div>
                                        </div>
                                        <div class="modal-footer">
                                            <button type="button" class="btn btn-default" data-dismiss="modal">Opps, I don't want to do that.</button>
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
