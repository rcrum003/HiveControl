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

// Close database before shell_exec — sensor scripts also access the DB and SQLite locks on concurrent access
$sthlive = null;
$conn = null;

// Regenerate hiveconfig.inc from the database so sensor scripts use current config
shell_exec("sudo /home/HiveControl/scripts/data/dump_hiveconfig_inc.sh 2>/dev/null");

switch ($sensor) {
    case "hivetemp":
        $value = shell_exec("/usr/bin/timeout 30 sudo /home/HiveControl/scripts/temp/gettemp.sh 2>/dev/null");
        $valueheader = "Temp F, Humidity%, Dew F, Temp C";
        break;

    case "hiveweight":
        $value = shell_exec("/usr/bin/timeout 60 sudo /home/HiveControl/scripts/weight/getweight.sh 2>/dev/null | awk '{print $1}'");
        $valueheader = "Gross Weight (lbs)";
        break;

    case "hivelux":
        $value = shell_exec("/usr/bin/timeout 20 sudo /home/HiveControl/scripts/light/getlux.sh 2>/dev/null");
        $valueheader = "LUX";
        break;

    case "weather":
        $value = shell_exec("/usr/bin/timeout 20 sudo /home/HiveControl/scripts/weather/getwx.sh 2>/dev/null | tail -5");
        $valueheader = "Weather Data";
        break;

    case "camera":
        $value = shell_exec("/usr/bin/timeout 10 sudo /home/HiveControl/scripts/image/takepic.sh 2>/dev/null");
        if ($value === null || trim($value) === '') {
            $value = "Camera capture completed. Check /home/HiveControl/www/public_html/images/ for output.";
        }
        $valueheader = "Camera Test";
        break;

    case "airquality":
        $value = shell_exec("/usr/bin/timeout 20 sudo /home/HiveControl/scripts/air/getair.sh 2>/dev/null | tail -5");
        $valueheader = "Air Quality Data";
        break;

    case "airnow":
        $value = shell_exec("/usr/bin/timeout 60 sudo /home/HiveControl/scripts/air/getairnow.sh 2>/dev/null | tail -10");
        $valueheader = "EPA AirNow Data";
        break;

    case "pollen":
        $dbpath = "/home/HiveControl/data/hive-data.db";
        try {
            $pdb = new PDO("sqlite:$dbpath");
            $pdb->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            $row = $pdb->query("SELECT date, pollenlevel, pollentypes FROM pollen ORDER BY date DESC LIMIT 1")->fetch(PDO::FETCH_ASSOC);
            if ($row) {
                $levels = ['None', 'Low', 'Moderate', 'High'];
                $lvl = $levels[min((int)$row['pollenlevel'], 3)] ?? $row['pollenlevel'];
                $value = "Date: " . $row['date'] . "\nLevel: " . $lvl . " (" . $row['pollenlevel'] . ")\nTypes: " . ($row['pollentypes'] ?: 'N/A');
            } else {
                $value = "No pollen data in database yet. Run the pollen collection script or wait for the next scheduled run.";
            }
            $pdb = null;
        } catch (Exception $e) {
            $value = "DB error: " . $e->getMessage();
        }
        $valueheader = "Pollen Data";
        break;

    case "blescan":
        $value = shell_exec("/usr/bin/timeout 25 sudo python3 /home/HiveControl/software/broodminder/BM_Scan_bleak.py 15 2>&1");
        $valueheader = "BLE Scan Results";
        break;

    default:
        $valueheader = "Unknown sensor type";
        $value = "Specify a valid sensor: hivetemp, hiveweight, hivelux, weather, camera, airquality, airnow, pollen";
        break;
}

echo $valueheader . "\n";
if ($value !== null) {
    echo trim($value);
} else {
    echo "No response from sensor. Check connection and configuration.";
}
?>
