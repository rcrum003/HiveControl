<?PHP

#Status Refresh
# Shows status of a message until done
# Pass which system getstatus we are interested in.
# Pass the id

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


$id = test_input($_GET["id"]);
$getstatus = test_input($_GET["getstatus"]);

#Check to see if the neccessary variables exist
if(isset($_GET["id"])) {
    // type exists
    if (empty($_GET["id"])) {
        // Default to nothing if no getstatus is set or empty
        $id = "";
        } else { 

            $id = test_input($_GET["id"]);
            // Not empty, so let's check it's input and run switch

        	$sth = $conn->prepare("SELECT * FROM msgqueue WHERE id=$id");
            $sth->execute();
            $result = $sth->fetch(PDO::FETCH_ASSOC);
            $status = $result['status'];
            $message = $result['message'];
            $msg_result = $result['response'];
            
        	if ( $status == "new" ) {
        		echo "Waiting on Message Queue Engine to start $message job # ".$id." (could be up to one minute)...";
        		header("Refresh:5;");
        		exit;
			} elseif ($status == "processing") {
				echo "Started processing $message job # ".$id." request... this could take a while....";	
				header("Refresh:5;");
				exit;
			} elseif ($status == "error") {
				$message_text = "Erorr for $message job # ".$id." request";
				#echo "$msg_result";
			} elseif ($status == "complete") {
				$message_text = "Successfully completed $message job # ".$id." request";
				#echo "$msg_result";
			}
		}

			}
    else {
    	echo "ID required";
    	exit;
    }
      	
?>
<!DOCTYPE html>
<html lang="en">

<head>
    <style>
    input {
        max-width: 100%;
    } 
    </style>

   <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
  
    <!-- Bootstrap Core CSS -->
    <link href="../bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="../dist/css/sb-admin-2.css" rel="stylesheet">
    
    <script>
    function confirm()
    { alert("Warning!"); // this is the message in ""}
    </script>

    <!-- Custom Fonts -->
    <link href="../bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

</head>
<body>

<h3><?PHP echo "$message_text"; ?> </h3><br>
<br>
<code><?PHP echo nl2br("$msg_result"); ?> </code>
<br>

<!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>

    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>

</html>

