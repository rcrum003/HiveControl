

<?PHP

#Script to pass variables in order to get different JSON outputs for highcharts.
# Outputs into JSON

#Take variables in to determine which json output we want.


# Global Database Connection
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

#Get URLs from the URL
#getdata.php?type=hivetempf&period=day&output=json

#type should be the column of data you want
#period should be the amount of data you are looking 
#output should be JSON, CSV or Highcharts

//Check input for badness
function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}


$callback = (string)$_GET['callback'];
if (!$callback) $callback = 'callback';


#Check to see if the neccessary variables exist and are safe
if(isset($_GET["type"])) {
    if (empty($_GET["type"])) {
    	echo "ERROR: Type can't be empty";
        exit;
	    } else {
	    	$type = test_input($_GET["type"]);
	    }
    } else {
	echo "ERROR: Must specify type";
    exit;
}

if(isset($_GET["period"])) {
    // period exists
    if (empty($_GET["period"])) {
    	echo "ERROR: Period can't be empty";
        exit;
	    } else {
	    	$period = test_input($_GET["period"]);
	    }
    } else {
	echo "ERROR: Must specify period";
    exit;
}


if(isset($_GET["chart"])) {
    // period exists
    if (empty($_GET["chart"])) {
        echo "ERROR: Chart can't be empty";
        exit;
        } else {
            $chart = test_input($_GET["chart"]);
        }
    } else {
    echo "ERROR: Must specify a chart";
    exit;
}


# We do the below to restrict what we pass to the database. We don't want to trust the incoming data, so we defined 
# the expected, and if we don't get it, we error out

# Save Period to use in the query
switch ($period) {
    case "today":
        $sqlperiod = "start of day";
        break;
    case "hour":
        $sqlperiod = "-1 hours";
        break;
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
    default:
        echo "ERROR: Invalid period specified";
        exit;
    }

# Do the same thing for type
switch ($type) {
    case "hivetempf":
        $type2 = "hivetempf"; 
       break;
    case "hivetempc":
        $type2 = "hivetempc"; 
       break;
    
    case "hivehum":
        $type2 = "hiveHum"; 
       break;
    
    case "hiveweight":
        $type2 = "hiveweight"; 
       break;
    
    case "hiverawweight":
        $type2 = "hiverawweight"; 
       break;
    
    case "weather_tempf":
        $type2 = "weather_tempf"; 
       break;
    
    case "weather_humidity":
        $type2 = "weather_humidity"; 
       break;
    
    case "weather_dewf":
        $type2 = "weather_dewf"; 
       break;
   
    case "weather_tempc":
        $type2 = "weather_tempc"; 
       break;
    
    case "wind_dir":
        $type2 = "wind_dir"; 
       break;
    
    case "wind_degrees":
        $type2 = "wind_degrees"; 
       break;
    
    case "wind_gust_mph":
        $type2 = "wind_gust_mph"; 
       break;
    
    case "wind_kph":
        $type2 = "wind_kph"; 
       break;
    
    case "wind_gust_kph":
        $type2 = "wind_gust_kph"; 
       break;
    
    case "pressure_mb":
        $type2 = "pressure_mb"; 
       break;
    
    case "pressure_in":
        $type2 = "pressure_in"; 
       break;
    
    case "pressure_tren":
        $type2 = "pressure_trend"; 
       break;
    
    case "weather_dewc":
        $type2 = "weather_dewc"; 
       break;
    
    case "solarradiation":
        $type2 = "solarradiation"; 
       break;
    
    case "uv":
        $type2 = "UV"; 
       break;
    
    case "precip_1hr_in":
        $type2 = "precip_1hr_in"; 
       break;
    
    case "precip_1hr_metric":
        $type2 = "precip_1hr_metric"; 
       break;
    
    case "wind_mph":
        $type2 = "wind_mph"; 
       break;
    
    case "lux":
        $type2 = "lux"; 
       break;
    default:
        echo "Error: Invalid type specified";
        exit;   
    }

# Get Data now depending on what you asked for

switch ($chart) {
        case 'line':
            $sth = $conn->prepare("SELECT strftime('%s',date)*1000 AS datetime, $type2 as value FROM allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime') ORDER BY datetime");
            $sth->execute(); 
            $result = $sth->fetchAll(PDO::FETCH_ASSOC);
            header('Content-Type: text/javascript');
            echo "$callback([";    
            foreach($result as $r){
            echo "[".$r['datetime'].", ".$r['value']."]".", ";}
            echo "\r\n";
            echo "]);";
            break;
        case 'highlow':
            # Shows HighLow for that day for a given data measurement
            $sth = $conn->prepare("SELECT  MAX($type2) as max, MIN($type2) as min, strftime('%s',date)*1000 AS datetime, strftime('%Y-%m-%d', date) AS day FROM allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime') GROUP BY day");
            $sth->execute(); 
            $result = $sth->fetchAll(PDO::FETCH_ASSOC);
            header('Content-Type: text/javascript');
            echo "$callback([";    
            foreach($result as $r){
            echo "[".$r['datetime'].", ".$r['max'].", ".$r['min']."]".", ";
            echo "\r\n";
            }
            echo "]);";
            break;
        default:
            # code...
        echo "nothing to do here";
            break;
    }    
      
    

?>