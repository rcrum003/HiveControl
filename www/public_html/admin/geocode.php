<?php
header('Content-Type: application/json');
header('X-Content-Type-Options: nosniff');

$city = isset($_GET['city']) ? trim($_GET['city']) : '';
$state = isset($_GET['state']) ? trim($_GET['state']) : '';

if (empty($city) || empty($state)) {
    echo json_encode(['error' => 'City and state are required']);
    exit;
}

$query = urlencode($city . ', ' . $state . ', USA');
$url = 'https://nominatim.openstreetmap.org/search?format=json&q=' . $query . '&limit=1';

$opts = [
    'http' => [
        'method' => 'GET',
        'header' => "User-Agent: HiveControl/2.16 (beehive monitoring)\r\nAccept: application/json\r\n",
        'timeout' => 10
    ]
];
$context = stream_context_create($opts);
$response = @file_get_contents($url, false, $context);

if ($response === false) {
    echo json_encode(['error' => 'Geocoding request failed']);
    exit;
}

echo $response;
