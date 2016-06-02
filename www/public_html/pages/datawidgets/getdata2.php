

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
#function test_input($data) {
#  $data = trim($data);
#  $data = stripslashes($data);
#  $data = htmlspecialchars($data);
#  return $data;
#}


$callback = (string)$_GET['callback'];
if (!$callback) $callback = 'callback';


#Check to see if the neccessary variables exist and are safe

# We do the below to restrict what we pass to the database. We don't want to trust the incoming data, so we defined 
# the expected, and if we don't get it, we error out

# Save Period to use in the query
switch ($period) {
    case "today":
        $sqlperiod = "start of the day";
        $datalevel = "all";
        break;
    case "hour":
        $sqlperiod = "-1 hours";
        $datalevel = "all";
        break;
    case "day":
        $sqlperiod = "-1 days";
        $datalevel = "all";
        break;
    case "week":
        $sqlperiod = "-7 days";
        #By hour
        $datalevel = "avghour";
        break;
    case "month":
        $sqlperiod = "-1 months";
        $datalevel = "avghour";
        break;
    case "year":
        $sqlperiod =  "-1 years";
        $datalevel = "avghour";
        break;
    case "all":
        $sqlperiod =  "-20 years";
        $datalevel = "avgweek";      
        break;
    default:
        echo "ERROR: Invalid period specified";
        exit;
    }

# Get the data

    #select max(hiveweight) as highweight, min(hiveweight) as lowweight, strftime('%Y-%m-%d-%H', date) as dateshow, strftime('%s',date)*1000 AS datetime from allhivedata GROUP BY dateshow ORDER by datetime ASC
switch ($datalevel) {
    case "avghour":
        if ( $SHOW_METRIC == "on" ) {
        $sth = $conn->prepare("SELECT round((hiveweight * 0.453592),2) as hiveweight, round((hiverawweight * 0.453592),2) as hiverawweight, hivetempc AS hivetempf, hiveHum, weather_tempc as weather_tempf, weather_humidity, precip_1hr_metric as precip_1hr_in, solarradiation, lux, in_count, out_count, wind_kph as wind, pressure_mb as pressure, strftime('%s',date)*1000 AS datetime, strftime('%Y-%m-%d-%H', date) as dateshow FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') GROUP BY dateshow ORDER by datetime ASC");
        } else {
        $sth = $conn->prepare("SELECT hiveweight, hiverawweight, hivetempf, hiveHum, weather_tempf, weather_humidity, precip_1hr_in, solarradiation, lux, in_count, out_count, wind_mph as wind, pressure_in as pressure, strftime('%s',date)*1000 AS datetime, strftime('%Y-%m-%d-%H', date) as dateshow FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') GROUP BY dateshow ORDER by datetime ASC");
        }
        break; 
    case "avgday":

        break;
    case "avgweek":

        break;
    case "all":
        if ( $SHOW_METRIC == "on" ) {
        $sth = $conn->prepare("SELECT round((hiveweight * 0.453592),2) as hiveweight, round((hiverawweight * 0.453592),2) as hiverawweight, hivetempc AS hivetempf, hiveHum, weather_tempc as weather_tempf, weather_humidity, precip_1hr_metric as precip_1hr_in, solarradiation, lux, in_count, out_count, wind_kph as wind, pressure_mb as pressure, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
        } else {
        $sth = $conn->prepare("SELECT hiveweight, hiverawweight, hivetempf, hiveHum, weather_tempf, weather_humidity, precip_1hr_in, solarradiation, lux, in_count, out_count, wind_mph as wind, pressure_in as pressure, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER by datetime ASC");
        }
        break;
    default:
        echo "nothing to do";
        exit;
} #End Switch

$sth->execute();
$result = $sth->fetchAll(PDO::FETCH_ASSOC);


# Get Data now depending on what you asked for

switch ($chart) {
        case 'line':
            $sth = $conn->prepare("SELECT strftime('%s',date)*1000 AS datetime, $type2 as value FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') ORDER BY datetime");
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
            $sth = $conn->prepare("SELECT  MAX($type2) as max, MIN($type2) as min, strftime('%s',date)*1000 AS datetime, strftime('%Y-%m-%d', date) AS day FROM allhivedata WHERE date > datetime('now', 'localtime', '$sqlperiod') GROUP BY day");
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
        #foreach($result as $r){echo "[".$r['datetime'].", "; if ($chart_rounding == "on") echo ceil($r['hivetempf']);else echo $r['hivetempf']; echo "]".", ";}
        #echo "nothing to do here";
            break;
    }    
      
    

?>