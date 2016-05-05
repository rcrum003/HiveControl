<?PHP

//Pass variables to determine the type of view

// Functions
// period - specify what timeperiods you want to see

//Check input for badness
function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}


$command = test_input($_GET["command"]);

# Commands currently supported
# clearlogs = Clear Logs
# reset = Reset all data in hive
# default = sets all settings back to default
# stop = Stops all collections
# cron = Updates cron
# update = Updates HiveControl to latest version


#Check to see if the neccessary variables exist

if(isset($_GET["command"])) {
    // type exists
    if (empty($_GET["command"])) {
        // Default to Day if no period is set or empty
        $command = "display";
        } else {
            $command = test_input($_GET["command"]);
        }
    } else {
    $command = "display";
}

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
$now = date('Y-m-d H:i:s');
$user_ip = getUserIP();


switch ($command) {
    case "clearlogs":
        #$sth = $conn->prepare("DELETE from logs");
        #$sth->execute();
        loglocal($now, "WEBADMIN", "INFO", "Logs Cleared by Admin at $user_ip");

        break;
    case "reset":
        $sth = $conn->prepare("DELETE from allhivedata");
        $sth->execute();
        $sth = $conn->prepare("DELETE from hivedata");
        $sth->execute();
        $sth = $conn->prepare("DELETE from weather");
        $sth->execute();
        $sth = $conn->prepare("DELETE from logs");
        $sth->execute();
        loglocal($now, "WEBADMIN", "INFO", "All Data Erased by Admin");
        break;
    case "stop":
        $sqlperiod = "-1 months";
        break;
    case "cron":
        $sqlperiod =  "-1 years";
        break;
    case "update":
        $sqlperiod =  "-20 years";
        break;
    case "display":
        $sth = $conn->prepare("SELECT upgrade_available, HCVersion from hiveconfig");
        $sth->execute();
        $result = $sth->fetch(PDO::FETCH_ASSOC);
        $upgrade = $result['upgrade_available'];
        $HCVersion = $result['HCVersion'];

        
        break;
    }


function loglocal($date, $program, $type, $message) {
  #Stores log entries locally
# This script takes 4 inputs and puts them into the DB
# 1 - Date - 2016-03-13 00:22:36
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
?>

<!DOCTYPE html>
<html lang="en">


    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

    <div id="wrapper">        
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Admin Functions</h1>
                </div>
                <!-- /.col-lg-12 -->
            </div>
            <!-- /.row -->

            <!-- /.row -->
           <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                      <div class="panel-body">


                      <?PHP echo "Avail $upgrade  /"; 
                      echo "You have $HCVersion "; ?>



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


