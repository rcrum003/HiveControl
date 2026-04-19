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

// Regenerate hiveconfig.inc so sensor scripts have current config
shell_exec("sudo /home/HiveControl/scripts/data/dump_hiveconfig_inc.sh 2>/dev/null");

$output = shell_exec("/usr/bin/timeout 45 sudo /home/HiveControl/scripts/weight/getrawreading.sh " . escapeshellarg($scaletype) . " 2>/dev/null");

if ($output === null || trim($output) === '') {
    echo json_encode(['success' => false, 'error' => 'No response from sensor. Check that the scale is connected and powered.']);
    exit();
}

$output = trim($output);

if (strpos($output, 'ERROR') === 0) {
    echo json_encode(['success' => false, 'error' => $output]);
    exit();
}

if (!is_numeric($output)) {
    echo json_encode(['success' => false, 'error' => 'Unexpected sensor output: ' . substr($output, 0, 100)]);
    exit();
}

echo json_encode(['success' => true, 'raw_value' => $output]);
