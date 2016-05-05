





<?PHP
#Notes on how to create a Weight Calibrate Wizard


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
		<a href="/admin/instrumentconfig.php"><button type="button" class="btn btn-danger">Cancel </button></a><button type="submit" class="btn btn-success">Next </button></form>';
		break;

	case '2':
		# code...
		if(isset($zero) || isset($testweight) ) {
    	// type exists
    		if (empty($zero) || empty($testweight)) {
        	// Default to nothing if no command is set or empty
        		echo "No values set - try wizard again<br>";
        		echo '<a href="/admin/instrumentconfig.php"><button type="button" class="btn btn-danger">Close </button></a>';
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
				echo "Zero Value = $zero <br>";
				$testweight_val = shell_exec("/usr/bin/timeout 5 /usr/bin/sudo /usr/local/bin/hx711 $zero | tail -1");
				echo "Known Weight = $testweight <br>";
				echo "Value for Known Weight (we removed zero already) = $testweight_val<br>";
				echo "Solving for calibration value <BR>";
				$calibration = $testweight_val/$testweight;
				echo "Calibration Should be $calibration <br>";
				echo '<input type="hidden" name="calibration" value="'.$calibration.'">';
				echo '<a href="/admin/instrumentconfig.php"><button type="button" class="btn btn-danger">Cancel </button></a>';
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
    	echo '<a href="/admin/instrumentconfig.php"><button class="btn btn-success">Close </button></a>';

		break;
	case '4':
		# code...
		break;
	default:
		# Start
		echo '<form method="POST" action="'; echo htmlspecialchars($_SERVER["PHP_SELF"]); echo'">';
		echo "Remove all weight from the scale. <br> Scale computer should be turned on for about 2 hrs before next step.<BR>";
		echo '<input type="hidden" name="step" value="1">';
		echo '<button type="submit" class="btn btn-success">Next </button>';
		break;
}

?>






