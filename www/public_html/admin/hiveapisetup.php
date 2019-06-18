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

function test_input_allow_slash($data) {
  $data = trim($data);
  $data = htmlspecialchars($data);
  return $data;
}



if ($_SERVER["REQUEST_METHOD"] == "GET") {
echo "Nothing to See Here";
exit;

}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
// Set Error Fields

    $v = new Valitron\Validator($_POST);
    $v->rule('alphaNum', ['HIVEAPI'], 1)->message('{field} must be alphaNum only');
    $v->rule('lengthmax', ['HIVEAPI'], 100);
        
    if($v->validate()) {
    
    $HIVEAPI = test_input($_POST["HIVEAPI"]);


    // Get current version    
    $ver = $conn->prepare("SELECT version FROM hiveconfig");
    $ver->execute();
    $ver = $ver->fetchColumn();
    // Increment version
    $version = ++$ver;

    // Update into the DB
    $doit = $conn->prepare("UPDATE hiveconfig SET HIVEAPI=? WHERE id=1");
    $doit->execute(array($HIVEAPI));
    sleep(1);
    
    // Tell user it saved
    echo '<div class="alert alert-success alert-dismissable"> Successfully Saved </button></div>';
} else {
    // Errors
     echo '<div class="alert alert-danger alert-dismissable"> <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>';
      $err=array_values($v->errors());
      for ($i=0; $i < count($err); $i++){
        echo $err[$i][0]."<br>";
      }
          echo  '</div>';  
           
}
               
    }
?>
       

                
    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>

    
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

    
</body>

</html>
