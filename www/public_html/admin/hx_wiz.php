<!DOCTYPE html>
<html lang="en">

<head>
   <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="Application to manage hive management tasks, and to report on various instruments.">
    <meta name="author" content="Ryan Crum">

    <title>Scale Calibration Wizard</title>

    <!-- Bootstrap Core CSS -->
    <link href="../bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">

    <!-- MetisMenu CSS -->
    <link href="../bower_components/metisMenu/dist/metisMenu.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="../dist/css/sb-admin-2.css" rel="stylesheet">
    
    <!-- Custom Fonts -->
    <link href="../bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesnt work if you view the page via file:// -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->

</head>
<body>
            <div class="row">
                <div class="col-lg-12">
                        <div class="panel-body">
                            

<?PHP
#Weight Calibrate Wizard


//Check input for badness
function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}


$step = test_input($_POST["step"]);
$zero = test_input($_POST["zero"]);
$testweight = test_input($_POST["testweight"]);
$testweight_val = test_input($_POST["testweight_val"]);
$calibration = test_input($_POST["calibration"]);

#Zero = 71,992
#After 5lbs = 108579
#36587 / 5
#Calib  - 7317.4

#Zero Value = 
#108579 Known Weight = 5 
#Value for Known Weight (we removed zero already) = 
#Solving for calibration value 
#Calibration Should be 0 

switch ($step) {
	case '1':
		# Get empty raw value...
		$zero = shell_exec("/usr/bin/timeout 5 /usr/bin/sudo /usr/local/bin/hx711 | tail -1");
		#$zero = "100";
		echo '<form method="POST" action="'; echo htmlspecialchars($_SERVER["PHP_SELF"]); echo'">';
		echo '<input type="hidden" name="zero" value="'.$zero.'">';
		echo '<input type="hidden" name="step" value="2">';
		echo "Zero value is $zero <br>";
		echo "Place Known Weight Value, (lbs please) on scale now<br>";

		echo 'How much weight did you put on? <input type="text" name="testweight" value=""><br>
		<button type="submit" class="btn btn-success">Next </button></form>';
		break;

	case '2':
		# code...
		if(isset($zero) || isset($testweight) ) {
    	// type exists
    		if (empty($zero) || empty($testweight)) {
        	// Default to nothing if no command is set or empty
        		echo "No values set - try wizard again<br>";
        		#echo '<a href="#" onclick="return self.close()"><button type="button" class="btn btn-danger">Close </button></a>';
				echo '<a href="/admin/hx_wiz.php"><button type="submit" class="btn btn-success">Restart Wizard </button></a></form>';
        		exit;
        	} else {
            	// Not empty, so let's do this thing
				echo '<form method="POST" action="'; echo htmlspecialchars($_SERVER["PHP_SELF"]); echo'">';
				echo '<input type="hidden" name="zero" value="'.$zero.'">';
				echo '<input type="hidden" name="testweight" value="'.$testweight.'">';
				echo '<input type="hidden" name="step" value="3">';
				#Get value of known weight
				#$testweight_val = "300";
				echo '
				<table width="200">
                <tr class="odd gradeX">';
				echo "<td>Zero Value</td><td>$zero</td></tr>";
				$testweight_val = shell_exec("/usr/bin/timeout 5 /usr/bin/sudo /usr/local/bin/hx711 $zero | tail -1");
				echo "<tr><td>Known Weight</td><td>$testweight</td></tr>";
				echo "<tr><td>Value for Known Weight (we removed zero already):</td><td>$testweight_val</td></tr>";
				echo "<tr><td>Solving for calibration value</td><td></td></tr>";
				$calibration = $testweight_val/$testweight;
				echo "<tr><td>Calibration Should be:</td><td><td>$calibration</td></tr>";

				#Get weight 
				#$RAW/CALI
				$hiveweight = ($testweight_val/$calibration);

				echo "<tr><td>Reading the scale with the recommended zero and calibration would give us a weight of:</td><td> $hiveweight</td></tr>";
				echo "<tr><td>NOTE: scale values will vary up to .5 lb per the specifications of the load cells (more or less depending on your equipment.</td></tr></table>";

				echo '<input type="hidden" name="calibration" value="'.$calibration.'">';
				#echo '<a href="#" onclick="return self.close()"><button type="button" class="btn btn-danger">Cancel </button></a>';
				echo '<button type="submit" class="btn btn-success">Save to DB </button></form>';
				break;
			}
	}
	case '3':
		# Save to Database...
		include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
		require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';
		// Get current version    
    	$ver = $conn->prepare("SELECT version FROM hiveconfig");
    	$ver->execute();
    	$ver = $ver->fetchColumn();
		// Increment version
    	$version = ++$ver;

    	// Update into the DB
    	$doit = $conn->prepare("UPDATE hiveconfig SET version=?,HIVE_WEIGHT_SLOPE=?,HIVE_WEIGHT_INTERCEPT=? WHERE id=1");
    	$doit->execute(array("$version","$calibration",$zero));
    	sleep(1);

    	echo "<h1>Completed Wizard</h1><br>";
    	#echo '<a href="#" onclick="return self.close()"><button class="btn btn-success">Close </button></a>';

		break;
	case '4':
		# code...
		break;
	default:
		# Start
		echo '<form method="POST" action="'; echo htmlspecialchars($_SERVER["PHP_SELF"]); echo'">';
		echo "Remove all weight from the scale. <br> Scale computer should be turned on for about 2 hrs before next step.<BR>";
		echo '<input type="hidden" name="step" value="1">';
		#echo '<a href="#" onclick="return self.close()"><button type="button" class="btn btn-danger">Cancel </button></a>';
		echo '<button type="submit" class="btn btn-success">Next </button>';
		break;
}

?>
    			</div>
    		</div>
    	</div>



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



