<?PHP
// Live sensor test endpoint
// Returns raw sensor readings for the setup wizard and instrument config page

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}

$sensor = "all";
if (isset($_GET["sensor"]) && !empty($_GET["sensor"])) {
    $sensor = test_input($_GET["sensor"]);
}

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sthlive = $conn->prepare("SELECT * FROM hiveconfig");
$sthlive->execute();
$r = $sthlive->fetch(PDO::FETCH_ASSOC);

switch ($sensor) {
    case "hivetemp":
        $value = shell_exec("/usr/bin/timeout 15 sudo /home/HiveControl/scripts/temp/gettemp.sh 2>&1");
        $valueheader = "Temp F, Humidity%, Dew F, Temp C";
        break;

    case "hiveweight":
        $value = shell_exec("/usr/bin/timeout 15 sudo /home/HiveControl/scripts/weight/getweight.sh 2>&1 | awk '{print $1}'");
        $valueheader = "Gross Weight (lbs)";
        break;

    case "hivelux":
        $value = shell_exec("/usr/bin/timeout 15 sudo /home/HiveControl/scripts/light/getlux.sh 2>&1");
        $valueheader = "LUX";
        break;

    case "weather":
        $value = shell_exec("/usr/bin/timeout 20 sudo /home/HiveControl/scripts/weather/getwx.sh 2>&1 | tail -5");
        $valueheader = "Weather Data";
        break;

    case "camera":
        $value = shell_exec("/usr/bin/timeout 10 sudo /home/HiveControl/scripts/image/takepic.sh 2>&1");
        if ($value === null || trim($value) === '') {
            $value = "Camera capture completed. Check /home/HiveControl/www/public_html/images/ for output.";
        }
        $valueheader = "Camera Test";
        break;

    case "airquality":
        $value = shell_exec("/usr/bin/timeout 15 sudo /home/HiveControl/scripts/air/getair.sh 2>&1 | tail -5");
        $valueheader = "Air Quality Data";
        break;

    default:
        $valueheader = "Unknown sensor type";
        $value = "Specify a valid sensor: hivetemp, hiveweight, hivelux, weather, camera, airquality";
        break;
}

echo $valueheader . "\n";
if ($value !== null) {
    echo trim($value);
} else {
    echo "No response from sensor. Check connection and configuration.";
}
?>
