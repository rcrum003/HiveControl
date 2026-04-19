<?PHP
// Saves calibration slope and intercept values to the database
// Called via AJAX POST from the setup wizard calibration sub-wizard
// Returns JSON: {"success": true} or {"success": false, "error": "message"}

header('Content-Type: application/json');

if ($_SERVER["REQUEST_METHOD"] !== "POST") {
    echo json_encode(['success' => false, 'error' => 'POST required']);
    exit();
}

$slope = isset($_POST['slope']) ? trim($_POST['slope']) : '';
$intercept = isset($_POST['intercept']) ? trim($_POST['intercept']) : '';
$scaletype = isset($_POST['scaletype']) ? trim($_POST['scaletype']) : '';

if (!is_numeric($slope) || !is_numeric($intercept)) {
    echo json_encode(['success' => false, 'error' => 'Slope and intercept must be numeric values']);
    exit();
}

if (floatval($slope) == 0) {
    echo json_encode(['success' => false, 'error' => 'Slope cannot be zero (would cause division by zero)']);
    exit();
}

$allowed_types = ['hx711', 'phidget1046'];

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sth = $conn->prepare("SELECT version FROM hiveconfig LIMIT 1");
$sth->execute();
$row = $sth->fetch(PDO::FETCH_ASSOC);
$version = ($row && isset($row['version'])) ? (int)$row['version'] + 1 : 1;

if (!empty($scaletype) && in_array($scaletype, $allowed_types)) {
    $update = $conn->prepare("UPDATE hiveconfig SET HIVE_WEIGHT_SLOPE=?, HIVE_WEIGHT_INTERCEPT=?, SCALETYPE=?, ENABLE_HIVE_WEIGHT_CHK='yes', version=?");
    $result = $update->execute([$slope, $intercept, $scaletype, $version]);
} else {
    $update = $conn->prepare("UPDATE hiveconfig SET HIVE_WEIGHT_SLOPE=?, HIVE_WEIGHT_INTERCEPT=?, version=?");
    $result = $update->execute([$slope, $intercept, $version]);
}

$sth = null;
$update = null;
$conn = null;

if (!$result) {
    echo json_encode(['success' => false, 'error' => 'Database update failed']);
    exit();
}

// Regenerate hiveconfig.inc so sensor scripts use the new calibration values
shell_exec("sudo /home/HiveControl/scripts/data/dump_hiveconfig_inc.sh 2>/dev/null");

echo json_encode(['success' => true, 'slope' => $slope, 'intercept' => $intercept]);
