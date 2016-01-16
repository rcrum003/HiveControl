

<?PHP

#Script to pass variables in order to get different JSON outputs for highcharts.
# Outputs into JSON

#Take variables in to determine which json output we want.


# Get a bunch of different Temp Values to display on our Temp Detail page

# Global Database Connection
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';


#Get URLs from the URL
#datajob.php?type=&period=

#type should be the column of data you want
#period should be the amount of data you are looking 
#output should be JSON, CSV or HTML

//Check input for badness
function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}



#Check to see if the neccessary variables exist
if(isset($_GET["type"])) {
    // type exists
    if (empty($_GET["type"])) {
    	echo "ERROR: Type can't be empty";
	    } else {
	    	$type = test_input($_GET["type"]);
    		#echo "The Type is $type";
	    }

    } else {
	echo "ERROR: Must specify type";
}

if(isset($_GET["period"])) {
    // type exists
    if (empty($_GET["period"])) {
    	echo "ERROR: Period can't be empty";
	    } else {
	    	$period = test_input($_GET["period"]);
    		#echo "The Period is $period";
	    }
    } else {
	echo "ERROR: Must specify period";
}

if(isset($_GET["output"])) {
    // type exists
    if (empty($_GET["output"])) {
    	echo "ERROR: Data can't be empty";
	    } else {
	    	$output = test_input($_GET["output"]);
    		#echo "The Type is $type";
	    }

    } else {
	echo "ERROR: Must specify output";
}

# Save Period to use in the query
switch ($period) {
    case "day":
        $sqlperiod = "-1 days";
        break;
    case "week":
        $sqlperiod = "-7 days";
        break;
    case "month":
        $sqlperiod = "-1 months";
        break;
    case "year":
        $sqlperiod =  "-1 years";
        break;
    case "all":
        $sqlperiod =  "-20 years";
        break;
    }


switch ($type) {
    case "all":
		$sth = $conn->prepare("SELECT * FROM allhivedata WHERE date > datetime('now', '$sqlperiod') GROUP BY date");
		$sth->execute();
        break;
    case "avg":
        $sth = $conn->prepare("SELECT strftime('%Y-%m',date) AS datetime, ROUND(AVG(hivetempf),2) as hivetempf, ROUND(AVG(hivehum),2) as hivehum,  ROUND(AVG(hiveweight),2) as hiveweight, ROUND(AVG(weather_tempf),2) as weather_tempf, ROUND(AVG(weather_humidity),2) as weather_humidity, ROUND(SUM(precip_today_in),2) as precip_today_in FROM allhivedata WHERE date > datetime('now', '$sqlperiod') GROUP BY datetime");
		$sth->execute();
        break;
    case "temp_chart":
        $sth = $conn->prepare("SELECT strftime('%s',date)*1000 AS datetime, hivetempf FROM allhivedata WHERE date > datetime('now','$sqlperiod') GROUP BY datetime");
		$sth->execute();
		$result = $sth->fetchAll(PDO::FETCH_ASSOC);

		foreach ($result as $r) {
		    $out[] = array((float)$r['datetime'],(float)$r['hivetempf']);
		} 
        break;
    case "temp_data":
        $sth = $conn->prepare("SELECT hiveid, date, hivetempf, hiveHum, weather_tempf, weather_humidity FROM allhivedata WHERE date > datetime('now','$sqlperiod') ORDER BY date");
		$sth->execute();
        break;

    case "weight_chart":
        $sth = $conn->prepare("SELECT strftime('%s',date)*1000 AS datetime, hiveweight, hiverawweight FROM allhivedata WHERE date > datetime('now','$sqlperiod') ORDER BY datetime");
		$sth->execute();
		$result = $sth->fetchAll(PDO::FETCH_ASSOC);

		foreach ($result as $r) {
		    $out[] = array((float)$r['datetime'],(float)$r['hiveweight']);
		} 
        break;
    case "weight_data":
        $sth = $conn->prepare("SELECT hiveid, date, hiveweight, hiverawweight FROM allhivedata WHERE date > datetime('now','$sqlperiod') ORDER BY date");
		$sth->execute();
        break;



   
    }



switch ($output) {
    case "csv":
		export_csv($sth, $type, $period);
		break;
    case "json":
        export_json($sth);
        break;
    case "highcharts":
    	export_highcharts($out);
    	break;
    }



#echo "Period is $sqlperiod";
#echo "Type is $type";
// Get Weather/Hive Data First - from a DB view, this may be very slow

function export_highcharts($out) {

//?type=temp&period=month&output=highcharts'

// Convert time to Unix EPOCH, output one value only.
// Format should be in JSON format, and look like this
// [122869440000000,14.25],
// for each series.
// Make sure you pick the right type to display the value for each
  

echo json_encode($out);


}

function export_json($sth){
	$result = $sth->fetchAll(PDO::FETCH_ASSOC);
	$json=json_encode($result);
	print_r($json);
}

// Used to export via csv
function export_csv($sth, $type, $period){

	// Set headers to make the browser download the results as a csv file
	header("Content-type: text/csv");
	header("Content-Disposition: attachment; filename=hivedata-$type$-period.csv");
	header("Pragma: no-cache");
	header("Expires: 0");

	// Fetch the first row
	$row = $sth->fetch(PDO::FETCH_ASSOC);

	// If no results are found, echo a message and stop
	if ($row == false){
    	echo "No results";
    	exit;
	}

	// Print the titles using the first line
	print_titles($row);
	// Iterate over the results and print each one in a line
	while ($row != false) {
    	// Print the line
  	echo implode(array_values($row), ",") . "\n";
    	// Fetch the next line
  	$row = $sth->fetch(PDO::FETCH_ASSOC);
	}
}

// Prints the column names
function print_titles($row){
    echo implode(array_keys($row), ",") . "\n";
}



?>