<?PHP
// Returns a raw (uncalibrated) sensor reading for scale calibration
// Called via AJAX from the setup wizard calibration sub-wizard
// Returns JSON: {"success": true, "raw_value": 123456} or {"success": false, "error": "message"}

header('Content-Type: application/json');

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}

$allowed_types = ['hx711', 'phidget1046'];
$scaletype = '';

if (isset($_GET['scaletype']) && !empty($_GET['scaletype'])) {
    $scaletype = test_input($_GET['scaletype']);
}

if (!in_array($scaletype, $allowed_types)) {
    echo json_encode(['success' => false, 'error' => 'Invalid scale type. Supported: ' . implode(', ', $allowed_types)]);
    exit();
}

// Save the scale type and enable flag to the database so hiveconfig.inc stays in sync
// (user may not have submitted the wizard form yet)
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
$upd = $conn->prepare("UPDATE hiveconfig SET SCALETYPE=?, ENABLE_HIVE_WEIGHT_CHK='yes'");
$upd->execute([$scaletype]);
$upd = null;
$conn = null;

// Regenerate hiveconfig.inc so sensor scripts have current config
shell_exec("sudo /home/HiveControl/scripts/data/dump_hiveconfig_inc.sh 2>/dev/null");

$samples = 1;
if (isset($_GET['samples']) && is_numeric($_GET['samples'])) {
    $samples = max(1, min(10, intval($_GET['samples'])));
}

$readings = [];
$cmd = "/usr/bin/timeout 30 sudo /home/HiveControl/scripts/weight/getrawreading.sh " . escapeshellarg($scaletype) . " 2>/dev/null";

for ($i = 0; $i < $samples; $i++) {
    $output = shell_exec($cmd);
    if ($output === null || trim($output) === '') {
        continue;
    }
    $val = trim($output);
    if (strpos($val, 'ERROR') === 0 || !is_numeric($val)) {
        continue;
    }
    $readings[] = floatval($val);
}

if (empty($readings)) {
    echo json_encode(['success' => false, 'error' => 'No response from sensor. Check that the scale is connected and powered.']);
    exit();
}

$average = array_sum($readings) / count($readings);

echo json_encode([
    'success' => true,
    'raw_value' => round($average, 2),
    'sample_count' => count($readings),
    'samples_requested' => $samples
]);
