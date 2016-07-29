
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


$sensor = test_input($_GET["sensor"]);

#Check to see if the neccessary variables exist

if(isset($_GET["sensor"])) {
    // exists
    if (empty($_GET["sensor"])) {
        // Default to Day if no period is set or empty
        $sensor = "all";
        } else {
            $sensor = test_input($_GET["sensor"]);
        }
    } else {
    $sensor = "all";
}


#Script to get all the values

########################################################
# Get Config to see what we have setup
########################################################
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

 $sthlive = $conn->prepare("SELECT * FROM hiveconfig");
 $sthlive->execute();
 $r = $sthlive->fetch(PDO::FETCH_ASSOC);


switch ($sensor) {
    case "hivetemp":
		########################################################
		# Hive Temp
		########################################################
        $value = shell_exec("sudo /home/HiveControl/scripts/temp/gettemp.sh");
        $valueheader = " Temp F, Humidity%, Dew F, Temp C  ";
		break;
    	
    case "hiveweight":
        ########################################################
		# Hive Weight
		########################################################
        $value = shell_exec("sudo /home/HiveControl/scripts/weight/getweight.sh |awk '{print $1}'");
        $valueheader = " Gross Weight ";
        break;

    case "hivelux":
        ########################################################
		# LUX
		########################################################
        $value = shell_exec("sudo /home/HiveControl/scripts/light/getlux.sh");
        $valueheader = " LUX  ";
		break;
    }

echo $valueheader . "<BR>";
echo $value;


?>


