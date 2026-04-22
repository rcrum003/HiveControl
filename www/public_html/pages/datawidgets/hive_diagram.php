<?php
/**
 * Dashboard Hive Diagram Widget
 * Renders a read-only SVG hive body visualization from the current configuration.
 * Included inside index.php — expects $conn (PDO) to be available.
 */

// Fetch hive body config
$hiveBodySth = $conn->prepare("SELECT
    hc.HIVE_STACK_ORDER,
    hc.SENSOR_TEMP_POSITION,
    hc.SENSOR_TEMP_LABEL,
    hc.FEEDER_HAS_SYRUP,
    hc.NUM_HIVE_BASE_SOLID_BOTTOM_BOARD,
    hc.NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD,
    hc.NUM_HIVE_FEEDER,
    hc.NUM_HIVE_TOP_INNER_COVER,
    hc.NUM_HIVE_TOP_TELE_COVER,
    hc.NUM_HIVE_TOP_MIGRATORY_COVER,
    hc.NUM_HIVE_BODY_MEDIUM_FOUNDATION,
    hc.NUM_HIVE_BODY_DEEP_FOUNDATION,
    hc.NUM_HIVE_BODY_SHAL_FOUNDATION,
    hw.HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT,
    hw.HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT,
    hw.HIVE_FEEDER_WEIGHT,
    hw.HIVE_TOP_INNER_COVER_WEIGHT,
    hw.HIVE_TOP_TELE_COVER_WEIGHT,
    hw.HIVE_TOP_MIGRATORY_COVER_WEIGHT,
    hw.HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT,
    hw.HIVE_BODY_DEEP_FOUNDATION_WEIGHT,
    hw.HIVE_BODY_SHAL_FOUNDATION_WEIGHT
FROM hiveconfig hc
INNER JOIN hiveequipmentweight hw ON hc.id = hw.id
WHERE hc.id = 1");
$hiveBodySth->execute();
$hiveBody = $hiveBodySth->fetch(PDO::FETCH_ASSOC);

// Check if any components are configured
$hasComponents = false;
if ($hiveBody) {
    if (!empty($hiveBody['HIVE_STACK_ORDER'])) {
        $hasComponents = true;
    } else {
        $numKeys = [
            'NUM_HIVE_BASE_SOLID_BOTTOM_BOARD', 'NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD',
            'NUM_HIVE_FEEDER', 'NUM_HIVE_TOP_INNER_COVER', 'NUM_HIVE_TOP_TELE_COVER',
            'NUM_HIVE_TOP_MIGRATORY_COVER', 'NUM_HIVE_BODY_MEDIUM_FOUNDATION',
            'NUM_HIVE_BODY_DEEP_FOUNDATION', 'NUM_HIVE_BODY_SHAL_FOUNDATION'
        ];
        foreach ($numKeys as $nk) {
            if (isset($hiveBody[$nk]) && intval($hiveBody[$nk]) > 0) {
                $hasComponents = true;
                break;
            }
        }
    }
}

if (!$hasComponents) {
    // No components configured — show placeholder
    echo '<div class="text-center" style="padding:20px;color:#999;">
        <i class="fa fa-cubes fa-2x" style="margin-bottom:8px"></i><br>
        No hive body configured.<br>
        <a href="/admin/hivebodyconfig.php" class="btn btn-xs btn-default" style="margin-top:8px">Configure Hive</a>
    </div>';
    return;
}

// Get current sensor reading for the diagram
$sensorValue = '';
if (isset($hivetempf) && $hivetempf !== 'null' && $hivetempf !== '' && $hivetempf !== 'NA') {
    $unit = (isset($SHOW_METRIC) && $SHOW_METRIC === 'on') ? 'C' : 'F';
    $sensorValue = $hivetempf . '°' . $unit;
}

// Build JS config for dashboard rendering
$dashConfig = [
    'stackOrder' => !empty($hiveBody['HIVE_STACK_ORDER']) ? explode(',', $hiveBody['HIVE_STACK_ORDER']) : [],
    'quantities' => [
        'NUM_HIVE_BASE_SOLID_BOTTOM_BOARD' => intval($hiveBody['NUM_HIVE_BASE_SOLID_BOTTOM_BOARD'] ?? 0),
        'NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD' => intval($hiveBody['NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD'] ?? 0),
        'NUM_HIVE_FEEDER' => intval($hiveBody['NUM_HIVE_FEEDER'] ?? 0),
        'NUM_HIVE_TOP_INNER_COVER' => intval($hiveBody['NUM_HIVE_TOP_INNER_COVER'] ?? 0),
        'NUM_HIVE_TOP_TELE_COVER' => intval($hiveBody['NUM_HIVE_TOP_TELE_COVER'] ?? 0),
        'NUM_HIVE_TOP_MIGRATORY_COVER' => intval($hiveBody['NUM_HIVE_TOP_MIGRATORY_COVER'] ?? 0),
        'NUM_HIVE_BODY_MEDIUM_FOUNDATION' => intval($hiveBody['NUM_HIVE_BODY_MEDIUM_FOUNDATION'] ?? 0),
        'NUM_HIVE_BODY_DEEP_FOUNDATION' => intval($hiveBody['NUM_HIVE_BODY_DEEP_FOUNDATION'] ?? 0),
        'NUM_HIVE_BODY_SHAL_FOUNDATION' => intval($hiveBody['NUM_HIVE_BODY_SHAL_FOUNDATION'] ?? 0),
    ],
    'weights' => [
        'HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT' => floatval($hiveBody['HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT'] ?? 0),
        'HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT' => floatval($hiveBody['HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT'] ?? 0),
        'HIVE_FEEDER_WEIGHT' => floatval($hiveBody['HIVE_FEEDER_WEIGHT'] ?? 0),
        'HIVE_TOP_INNER_COVER_WEIGHT' => floatval($hiveBody['HIVE_TOP_INNER_COVER_WEIGHT'] ?? 0),
        'HIVE_TOP_TELE_COVER_WEIGHT' => floatval($hiveBody['HIVE_TOP_TELE_COVER_WEIGHT'] ?? 0),
        'HIVE_TOP_MIGRATORY_COVER_WEIGHT' => floatval($hiveBody['HIVE_TOP_MIGRATORY_COVER_WEIGHT'] ?? 0),
        'HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT' => floatval($hiveBody['HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT'] ?? 0),
        'HIVE_BODY_DEEP_FOUNDATION_WEIGHT' => floatval($hiveBody['HIVE_BODY_DEEP_FOUNDATION_WEIGHT'] ?? 0),
        'HIVE_BODY_SHAL_FOUNDATION_WEIGHT' => floatval($hiveBody['HIVE_BODY_SHAL_FOUNDATION_WEIGHT'] ?? 0),
    ],
    'sensorTempPosition' => isset($hiveBody['SENSOR_TEMP_POSITION']) ? intval($hiveBody['SENSOR_TEMP_POSITION']) : -1,
    'sensorValue' => $sensorValue,
    'sensorLabel' => isset($hiveBody['SENSOR_TEMP_LABEL']) ? $hiveBody['SENSOR_TEMP_LABEL'] : 'Hive Temp',
    'feederHasSyrup' => isset($hiveBody['FEEDER_HAS_SYRUP']) ? (intval($hiveBody['FEEDER_HAS_SYRUP']) ? true : false) : false,
];
?>

<div class="dashboard-hive-diagram">
    <div id="dashboard-hive-svg" class="hive-diagram-wrap"></div>
    <a href="/admin/hivebodyconfig.php" class="hive-link-overlay" title="Edit hive configuration"></a>
    <div class="diagram-footer">Click to edit configuration</div>
</div>

<script>
window._hiveDiagramDashCfg = <?php echo json_encode($dashConfig, JSON_HEX_TAG | JSON_HEX_AMP | JSON_HEX_APOS | JSON_HEX_QUOT); ?>;
</script>
