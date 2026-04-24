<?PHP

include($_SERVER["DOCUMENT_ROOT"] . "/include/security-init.php");
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    return $data;
}

// Load current config
$sth = $conn->prepare("SELECT * FROM hiveconfig INNER JOIN hiveequipmentweight ON hiveconfig.id = hiveequipmentweight.id WHERE hiveconfig.id = 1");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);

$saveMessage = '';
$saveError = '';

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    require_csrf_token();
    $v = new Valitron\Validator($_POST);

    $numericFields = [
        'HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT', 'HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT',
        'HIVE_FEEDER_WEIGHT', 'HIVE_TOP_INNER_COVER_WEIGHT', 'HIVE_TOP_TELE_COVER_WEIGHT',
        'HIVE_TOP_MIGRATORY_COVER_WEIGHT', 'HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT',
        'HIVE_BODY_DEEP_FOUNDATION_WEIGHT', 'HIVE_BODY_SHAL_FOUNDATION_WEIGHT',
        'HIVE_TOP_WEIGHT', 'HIVE_COMPUTER_WEIGHT', 'HIVE_MISC_WEIGHT'
    ];
    $v->rule('numeric', $numericFields, 1)->message('{field} must be numeric');

    $intFields = [
        'NUM_HIVE_BASE_SOLID_BOTTOM_BOARD', 'NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD',
        'NUM_HIVE_FEEDER', 'NUM_HIVE_TOP_INNER_COVER', 'NUM_HIVE_TOP_TELE_COVER',
        'NUM_HIVE_TOP_MIGRATORY_COVER', 'NUM_HIVE_BODY_MEDIUM_FOUNDATION',
        'NUM_HIVE_BODY_DEEP_FOUNDATION', 'NUM_HIVE_BODY_SHAL_FOUNDATION'
    ];
    $v->rule('integer', $intFields)->message('{field} must be a whole number');

    if ($v->validate()) {
        // Get and increment version
        $ver = $conn->prepare("SELECT version FROM hiveconfig WHERE id = 1");
        $ver->execute();
        $version = intval($ver->fetchColumn()) + 1;

        // Sanitize all POST values
        $clean = [];
        foreach ($_POST as $key => $val) {
            $clean[$key] = test_input($val);
        }

        // Update equipment weights
        $doit = $conn->prepare("UPDATE hiveequipmentweight SET
            HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT = ?,
            HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT = ?,
            HIVE_FEEDER_WEIGHT = ?,
            HIVE_TOP_INNER_COVER_WEIGHT = ?,
            HIVE_TOP_TELE_COVER_WEIGHT = ?,
            HIVE_TOP_MIGRATORY_COVER_WEIGHT = ?,
            HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT = ?,
            HIVE_BODY_DEEP_FOUNDATION_WEIGHT = ?,
            HIVE_BODY_SHAL_FOUNDATION_WEIGHT = ?,
            HIVE_TOP_WEIGHT = ?,
            HIVE_COMPUTER_WEIGHT = ?,
            HIVE_MISC_WEIGHT = ?
            WHERE id = 1");
        $doit->execute([
            $clean['HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT'],
            $clean['HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT'],
            $clean['HIVE_FEEDER_WEIGHT'],
            $clean['HIVE_TOP_INNER_COVER_WEIGHT'],
            $clean['HIVE_TOP_TELE_COVER_WEIGHT'],
            $clean['HIVE_TOP_MIGRATORY_COVER_WEIGHT'],
            $clean['HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT'],
            $clean['HIVE_BODY_DEEP_FOUNDATION_WEIGHT'],
            $clean['HIVE_BODY_SHAL_FOUNDATION_WEIGHT'],
            $clean['HIVE_TOP_WEIGHT'],
            $clean['HIVE_COMPUTER_WEIGHT'],
            $clean['HIVE_MISC_WEIGHT']
        ]);

        // Sanitize stack order — only allow known component keys and commas
        $stackOrder = isset($clean['HIVE_STACK_ORDER']) ? $clean['HIVE_STACK_ORDER'] : '';
        $allowedKeys = ['tele_cover','migratory_cover','inner_cover','feeder','shallow','medium','deep','solid_bottom','screened_bottom'];
        $stackParts = array_filter(explode(',', $stackOrder), function($k) use ($allowedKeys) {
            return in_array($k, $allowedKeys, true);
        });
        $stackOrder = implode(',', $stackParts);

        $sensorPos = isset($clean['SENSOR_TEMP_POSITION']) ? intval($clean['SENSOR_TEMP_POSITION']) : -1;
        $sensorLabel = isset($clean['SENSOR_TEMP_LABEL']) ? substr($clean['SENSOR_TEMP_LABEL'], 0, 50) : 'Hive Temp';
        $feederSyrup = isset($clean['FEEDER_HAS_SYRUP']) ? (intval($clean['FEEDER_HAS_SYRUP']) ? 1 : 0) : 0;
        $frameFeederPos = isset($clean['FRAME_FEEDER_POSITION']) ? intval($clean['FRAME_FEEDER_POSITION']) : -1;
        $frameFeederLabel = isset($clean['FRAME_FEEDER_LABEL']) ? substr($clean['FRAME_FEEDER_LABEL'], 0, 50) : 'Frame Feeder';

        // Update hiveconfig with quantities and new fields
        $doit2 = $conn->prepare("UPDATE hiveconfig SET
            version = ?,
            NUM_HIVE_BASE_SOLID_BOTTOM_BOARD = ?,
            NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD = ?,
            NUM_HIVE_FEEDER = ?,
            NUM_HIVE_TOP_INNER_COVER = ?,
            NUM_HIVE_TOP_TELE_COVER = ?,
            NUM_HIVE_TOP_MIGRATORY_COVER = ?,
            NUM_HIVE_BODY_MEDIUM_FOUNDATION = ?,
            NUM_HIVE_BODY_DEEP_FOUNDATION = ?,
            NUM_HIVE_BODY_SHAL_FOUNDATION = ?,
            HIVE_STACK_ORDER = ?,
            SENSOR_TEMP_POSITION = ?,
            SENSOR_TEMP_LABEL = ?,
            FEEDER_HAS_SYRUP = ?,
            FRAME_FEEDER_POSITION = ?,
            FRAME_FEEDER_LABEL = ?
            WHERE id = 1");
        $doit2->execute([
            $version,
            $clean['NUM_HIVE_BASE_SOLID_BOTTOM_BOARD'],
            $clean['NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD'],
            $clean['NUM_HIVE_FEEDER'],
            $clean['NUM_HIVE_TOP_INNER_COVER'],
            $clean['NUM_HIVE_TOP_TELE_COVER'],
            $clean['NUM_HIVE_TOP_MIGRATORY_COVER'],
            $clean['NUM_HIVE_BODY_MEDIUM_FOUNDATION'],
            $clean['NUM_HIVE_BODY_DEEP_FOUNDATION'],
            $clean['NUM_HIVE_BODY_SHAL_FOUNDATION'],
            $stackOrder,
            $sensorPos,
            $sensorLabel,
            $feederSyrup,
            $frameFeederPos,
            $frameFeederLabel
        ]);

        // Refresh result
        $sth2 = $conn->prepare("SELECT * FROM hiveconfig INNER JOIN hiveequipmentweight ON hiveconfig.id = hiveequipmentweight.id WHERE hiveconfig.id = 1");
        $sth2->execute();
        $result = $sth2->fetch(PDO::FETCH_ASSOC);
        $saveMessage = 'Hive configuration saved successfully.';
    } else {
        $errs = array_values($v->errors());
        $errMsgs = [];
        for ($i = 0; $i < count($errs); $i++) {
            $errMsgs[] = htmlspecialchars($errs[$i][0], ENT_QUOTES, 'UTF-8');
        }
        $saveError = implode('<br>', $errMsgs);
    }
}

// Build config JSON for JavaScript
$stackOrder = isset($result['HIVE_STACK_ORDER']) ? $result['HIVE_STACK_ORDER'] : '';
$sensorPos = isset($result['SENSOR_TEMP_POSITION']) ? intval($result['SENSOR_TEMP_POSITION']) : -1;
$sensorLabel = isset($result['SENSOR_TEMP_LABEL']) ? $result['SENSOR_TEMP_LABEL'] : 'Hive Temp';
$feederSyrup = isset($result['FEEDER_HAS_SYRUP']) ? intval($result['FEEDER_HAS_SYRUP']) : 0;
$frameFeederPos = isset($result['FRAME_FEEDER_POSITION']) ? intval($result['FRAME_FEEDER_POSITION']) : -1;
$frameFeederLabel = isset($result['FRAME_FEEDER_LABEL']) ? $result['FRAME_FEEDER_LABEL'] : 'Frame Feeder';

$jsConfig = [
    'stackOrder' => $stackOrder ? explode(',', $stackOrder) : [],
    'quantities' => [
        'NUM_HIVE_BASE_SOLID_BOTTOM_BOARD' => intval($result['NUM_HIVE_BASE_SOLID_BOTTOM_BOARD'] ?? 0),
        'NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD' => intval($result['NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD'] ?? 0),
        'NUM_HIVE_FEEDER' => intval($result['NUM_HIVE_FEEDER'] ?? 0),
        'NUM_HIVE_TOP_INNER_COVER' => intval($result['NUM_HIVE_TOP_INNER_COVER'] ?? 0),
        'NUM_HIVE_TOP_TELE_COVER' => intval($result['NUM_HIVE_TOP_TELE_COVER'] ?? 0),
        'NUM_HIVE_TOP_MIGRATORY_COVER' => intval($result['NUM_HIVE_TOP_MIGRATORY_COVER'] ?? 0),
        'NUM_HIVE_BODY_MEDIUM_FOUNDATION' => intval($result['NUM_HIVE_BODY_MEDIUM_FOUNDATION'] ?? 0),
        'NUM_HIVE_BODY_DEEP_FOUNDATION' => intval($result['NUM_HIVE_BODY_DEEP_FOUNDATION'] ?? 0),
        'NUM_HIVE_BODY_SHAL_FOUNDATION' => intval($result['NUM_HIVE_BODY_SHAL_FOUNDATION'] ?? 0),
    ],
    'weights' => [
        'HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT' => floatval($result['HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT'] ?? 0),
        'HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT' => floatval($result['HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT'] ?? 0),
        'HIVE_FEEDER_WEIGHT' => floatval($result['HIVE_FEEDER_WEIGHT'] ?? 0),
        'HIVE_TOP_INNER_COVER_WEIGHT' => floatval($result['HIVE_TOP_INNER_COVER_WEIGHT'] ?? 0),
        'HIVE_TOP_TELE_COVER_WEIGHT' => floatval($result['HIVE_TOP_TELE_COVER_WEIGHT'] ?? 0),
        'HIVE_TOP_MIGRATORY_COVER_WEIGHT' => floatval($result['HIVE_TOP_MIGRATORY_COVER_WEIGHT'] ?? 0),
        'HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT' => floatval($result['HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT'] ?? 0),
        'HIVE_BODY_DEEP_FOUNDATION_WEIGHT' => floatval($result['HIVE_BODY_DEEP_FOUNDATION_WEIGHT'] ?? 0),
        'HIVE_BODY_SHAL_FOUNDATION_WEIGHT' => floatval($result['HIVE_BODY_SHAL_FOUNDATION_WEIGHT'] ?? 0),
    ],
    'fixedWeights' => [
        'HIVE_TOP_WEIGHT' => floatval($result['HIVE_TOP_WEIGHT'] ?? 0),
        'HIVE_COMPUTER_WEIGHT' => floatval($result['HIVE_COMPUTER_WEIGHT'] ?? 0),
        'HIVE_MISC_WEIGHT' => floatval($result['HIVE_MISC_WEIGHT'] ?? 0),
    ],
    'sensorTempPosition' => $sensorPos,
    'sensorLabel' => $sensorLabel,
    'feederHasSyrup' => $feederSyrup ? true : false,
    'frameFeederPosition' => $frameFeederPos,
    'frameFeederLabel' => $frameFeederLabel,
];
?>
<!DOCTYPE html>
<html lang="en">

    <!-- Header and Navigation -->
    <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

    <link rel="stylesheet" href="/css/hive-diagram.css">
    <link rel="stylesheet" href="../bower_components/jquery-ui/dist/jquery-ui.min.css">

    <div id="wrapper">

        <div class="row">
            <div class="col-lg-12">
                <h1 class="page-header">Hive Body Configuration</h1>
                <p class="text-muted">Build your hive visually. Drag to reorder, click between components to place your temperature sensor, click <i class="fa fa-tint" style="color:#4A90D9"></i> on a body to place a frame feeder.</p>
            </div>
        </div>

        <?php if ($saveMessage): ?>
        <div class="alert alert-success alert-dismissable">
            <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>
            <?php echo htmlspecialchars($saveMessage); ?>
        </div>
        <?php endif; ?>

        <?php if ($saveError): ?>
        <div class="alert alert-danger alert-dismissable">
            <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>
            <?php echo $saveError; ?>
        </div>
        <?php endif; ?>

        <form method="POST" action="<?php echo htmlspecialchars($_SERVER['PHP_SELF']); ?>" id="hive-config-form">
        <?php echo csrf_field(); ?>

        <!-- Hidden fields for computed values -->
        <input type="hidden" name="HIVE_STACK_ORDER" id="field-stack-order" value="<?php echo htmlspecialchars($stackOrder); ?>">
        <input type="hidden" name="SENSOR_TEMP_POSITION" id="field-sensor-pos" value="<?php echo $sensorPos; ?>">
        <input type="hidden" name="SENSOR_TEMP_LABEL" id="field-sensor-label" value="<?php echo htmlspecialchars($sensorLabel); ?>">
        <input type="hidden" name="FEEDER_HAS_SYRUP" id="field-feeder-syrup" value="<?php echo $feederSyrup; ?>">
        <input type="hidden" name="FRAME_FEEDER_POSITION" id="field-frame-feeder-pos" value="<?php echo $frameFeederPos; ?>">
        <input type="hidden" name="FRAME_FEEDER_LABEL" id="field-frame-feeder-label" value="<?php echo htmlspecialchars($frameFeederLabel); ?>">

        <!-- Hidden fields for quantities (computed from stack) -->
        <input type="hidden" name="NUM_HIVE_BASE_SOLID_BOTTOM_BOARD" id="qty-NUM_HIVE_BASE_SOLID_BOTTOM_BOARD" value="<?php echo intval($result['NUM_HIVE_BASE_SOLID_BOTTOM_BOARD'] ?? 0); ?>">
        <input type="hidden" name="NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD" id="qty-NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD" value="<?php echo intval($result['NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD'] ?? 0); ?>">
        <input type="hidden" name="NUM_HIVE_FEEDER" id="qty-NUM_HIVE_FEEDER" value="<?php echo intval($result['NUM_HIVE_FEEDER'] ?? 0); ?>">
        <input type="hidden" name="NUM_HIVE_TOP_INNER_COVER" id="qty-NUM_HIVE_TOP_INNER_COVER" value="<?php echo intval($result['NUM_HIVE_TOP_INNER_COVER'] ?? 0); ?>">
        <input type="hidden" name="NUM_HIVE_TOP_TELE_COVER" id="qty-NUM_HIVE_TOP_TELE_COVER" value="<?php echo intval($result['NUM_HIVE_TOP_TELE_COVER'] ?? 0); ?>">
        <input type="hidden" name="NUM_HIVE_TOP_MIGRATORY_COVER" id="qty-NUM_HIVE_TOP_MIGRATORY_COVER" value="<?php echo intval($result['NUM_HIVE_TOP_MIGRATORY_COVER'] ?? 0); ?>">
        <input type="hidden" name="NUM_HIVE_BODY_MEDIUM_FOUNDATION" id="qty-NUM_HIVE_BODY_MEDIUM_FOUNDATION" value="<?php echo intval($result['NUM_HIVE_BODY_MEDIUM_FOUNDATION'] ?? 0); ?>">
        <input type="hidden" name="NUM_HIVE_BODY_DEEP_FOUNDATION" id="qty-NUM_HIVE_BODY_DEEP_FOUNDATION" value="<?php echo intval($result['NUM_HIVE_BODY_DEEP_FOUNDATION'] ?? 0); ?>">
        <input type="hidden" name="NUM_HIVE_BODY_SHAL_FOUNDATION" id="qty-NUM_HIVE_BODY_SHAL_FOUNDATION" value="<?php echo intval($result['NUM_HIVE_BODY_SHAL_FOUNDATION'] ?? 0); ?>">

        <div class="hive-editor">

            <!-- LEFT: Component Palette -->
            <div class="palette-panel">
                <div class="panel panel-default">
                    <div class="panel-heading"><strong>Component Palette</strong> — click to add</div>
                    <div class="panel-body">

                        <h5 style="margin-top:0;color:#888">Covers</h5>
                        <?php
                        $paletteItems = [
                            ['key' => 'tele_cover', 'label' => 'Telescoping Cover', 'img' => '/images/hive/telecover.png', 'weightKey' => 'HIVE_TOP_TELE_COVER_WEIGHT'],
                            ['key' => 'migratory_cover', 'label' => 'Migratory Cover', 'img' => '/images/hive/migratorytop.png', 'weightKey' => 'HIVE_TOP_MIGRATORY_COVER_WEIGHT'],
                            ['key' => 'inner_cover', 'label' => 'Inner Cover', 'img' => '/images/hive/innercover.png', 'weightKey' => 'HIVE_TOP_INNER_COVER_WEIGHT'],
                        ];
                        foreach ($paletteItems as $item):
                        ?>
                        <div class="palette-card" data-component="<?php echo $item['key']; ?>">
                            <img src="<?php echo $item['img']; ?>" alt="<?php echo htmlspecialchars($item['label']); ?>">
                            <div class="card-info">
                                <h5><?php echo htmlspecialchars($item['label']); ?></h5>
                                <label>Weight (lbs):</label>
                                <input type="number" step="0.1" min="0" class="form-control input-sm weight-input"
                                       name="<?php echo $item['weightKey']; ?>"
                                       data-weight-key="<?php echo $item['weightKey']; ?>"
                                       value="<?php echo htmlspecialchars($result[$item['weightKey']] ?? '0'); ?>">
                            </div>
                            <button type="button" class="btn btn-xs btn-primary btn-add" title="Add to hive"><i class="fa fa-plus"></i></button>
                        </div>
                        <?php endforeach; ?>

                        <h5 style="color:#888">Bodies & Feeders</h5>
                        <?php
                        $bodyItems = [
                            ['key' => 'feeder', 'label' => 'Hive Feeder', 'img' => '/images/hive/feeder.png', 'weightKey' => 'HIVE_FEEDER_WEIGHT'],
                            ['key' => 'shallow', 'label' => 'Shallow Super', 'img' => '/images/hive/shallow.png', 'weightKey' => 'HIVE_BODY_SHAL_FOUNDATION_WEIGHT'],
                            ['key' => 'medium', 'label' => 'Medium Super', 'img' => '/images/hive/medium-w-frames.png', 'weightKey' => 'HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT'],
                            ['key' => 'deep', 'label' => 'Deep Body', 'img' => '/images/hive/deep.png', 'weightKey' => 'HIVE_BODY_DEEP_FOUNDATION_WEIGHT'],
                        ];
                        foreach ($bodyItems as $item):
                        ?>
                        <div class="palette-card" data-component="<?php echo $item['key']; ?>">
                            <img src="<?php echo $item['img']; ?>" alt="<?php echo htmlspecialchars($item['label']); ?>">
                            <div class="card-info">
                                <h5><?php echo htmlspecialchars($item['label']); ?></h5>
                                <label>Weight (lbs):</label>
                                <input type="number" step="0.1" min="0" class="form-control input-sm weight-input"
                                       name="<?php echo $item['weightKey']; ?>"
                                       data-weight-key="<?php echo $item['weightKey']; ?>"
                                       value="<?php echo htmlspecialchars($result[$item['weightKey']] ?? '0'); ?>">
                            </div>
                            <button type="button" class="btn btn-xs btn-primary btn-add" title="Add to hive"><i class="fa fa-plus"></i></button>
                        </div>
                        <?php endforeach; ?>

                        <h5 style="color:#888">Bottom Boards</h5>
                        <?php
                        $bottomItems = [
                            ['key' => 'solid_bottom', 'label' => 'Solid Bottom Board', 'img' => '/images/hive/solidbottomboard.png', 'weightKey' => 'HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT'],
                            ['key' => 'screened_bottom', 'label' => 'Screened Bottom Board', 'img' => '/images/hive/screenedbottomboard2.png', 'weightKey' => 'HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT'],
                        ];
                        foreach ($bottomItems as $item):
                        ?>
                        <div class="palette-card" data-component="<?php echo $item['key']; ?>">
                            <img src="<?php echo $item['img']; ?>" alt="<?php echo htmlspecialchars($item['label']); ?>">
                            <div class="card-info">
                                <h5><?php echo htmlspecialchars($item['label']); ?></h5>
                                <label>Weight (lbs):</label>
                                <input type="number" step="0.1" min="0" class="form-control input-sm weight-input"
                                       name="<?php echo $item['weightKey']; ?>"
                                       data-weight-key="<?php echo $item['weightKey']; ?>"
                                       value="<?php echo htmlspecialchars($result[$item['weightKey']] ?? '0'); ?>">
                            </div>
                            <button type="button" class="btn btn-xs btn-primary btn-add" title="Add to hive"><i class="fa fa-plus"></i></button>
                        </div>
                        <?php endforeach; ?>

                    </div>
                </div>

                <!-- Fixed Weights -->
                <div class="panel panel-default">
                    <div class="panel-heading"><strong>Other Weights</strong> — not part of the stack</div>
                    <div class="panel-body">
                        <div class="fixed-weights">
                            <div class="fw-item">
                                <label>Hive Top Weight (lbs)</label>
                                <input type="number" step="0.1" min="0" class="form-control input-sm fixed-weight-input"
                                       name="HIVE_TOP_WEIGHT" data-fixed-key="HIVE_TOP_WEIGHT"
                                       value="<?php echo htmlspecialchars($result['HIVE_TOP_WEIGHT'] ?? '0'); ?>">
                            </div>
                            <div class="fw-item">
                                <label>Computer Weight (lbs)</label>
                                <input type="number" step="0.1" min="0" class="form-control input-sm fixed-weight-input"
                                       name="HIVE_COMPUTER_WEIGHT" data-fixed-key="HIVE_COMPUTER_WEIGHT"
                                       value="<?php echo htmlspecialchars($result['HIVE_COMPUTER_WEIGHT'] ?? '0'); ?>">
                            </div>
                            <div class="fw-item">
                                <label>Misc Weight (lbs)</label>
                                <input type="number" step="0.1" min="0" class="form-control input-sm fixed-weight-input"
                                       name="HIVE_MISC_WEIGHT" data-fixed-key="HIVE_MISC_WEIGHT"
                                       value="<?php echo htmlspecialchars($result['HIVE_MISC_WEIGHT'] ?? '0'); ?>">
                            </div>
                        </div>
                    </div>
                </div>

                <!-- Sensors & Feeders -->
                <div class="panel panel-default">
                    <div class="panel-heading"><strong>Sensors & Feeders</strong></div>
                    <div class="panel-body">
                        <h5 style="margin-top:0;color:#888">Temperature Sensor</h5>
                        <p class="text-muted" style="font-size:12px">Click between components in the stack to place your sensor. Click again to remove.</p>
                        <label>Sensor Label:</label>
                        <input type="text" class="form-control input-sm" id="sensor-label-input" maxlength="50"
                               value="<?php echo htmlspecialchars($sensorLabel); ?>"
                               placeholder="e.g. Hive Temp, Brood Temp">
                        <div style="margin-top:8px">
                            <label>
                                <input type="checkbox" id="feeder-syrup-check" <?php if ($feederSyrup) echo 'checked'; ?>>
                                Feeder contains syrup
                            </label>
                        </div>

                        <hr style="margin:12px 0">
                        <h5 style="margin-top:0;color:#888">Frame Feeder</h5>
                        <p class="text-muted" style="font-size:12px">Click the <i class="fa fa-tint" style="color:#4A90D9"></i> button on a body component to place your in-hive frame feeder. Click again to remove.</p>
                        <label>Feeder Label:</label>
                        <input type="text" class="form-control input-sm" id="frame-feeder-label-input" maxlength="50"
                               value="<?php echo htmlspecialchars($frameFeederLabel); ?>"
                               placeholder="e.g. Frame Feeder, Division Board Feeder">
                    </div>
                </div>
            </div>

            <!-- RIGHT: Your Hive Stack -->
            <div class="stack-panel">
                <div class="panel panel-default">
                    <div class="panel-heading">
                        <strong>Your Hive</strong>
                        <span class="pull-right text-muted" style="font-size:12px"><i class="fa fa-arrows-v"></i> Drag to reorder</span>
                    </div>
                    <div class="panel-body">
                        <!-- Interactive stack list -->
                        <ul class="hive-stack-list" id="hive-stack-list"></ul>

                        <!-- SVG diagram preview -->
                        <div id="hive-stack-diagram" class="hive-diagram-wrap" style="margin-top:16px"></div>

                        <!-- Tare weight -->
                        <div class="tare-summary">
                            Total Tare Weight: <span class="tare-value" id="tare-weight">0.00</span> lbs
                        </div>
                    </div>
                </div>

                <div class="text-center" style="margin-top:10px">
                    <button type="submit" class="btn btn-lg btn-success"><i class="fa fa-save"></i> Save Configuration</button>
                </div>
            </div>

        </div>
        </form>

    </div>
    <!-- /#wrapper -->

    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>
    <!-- jQuery UI for sortable -->
    <script src="../bower_components/jquery-ui/dist/jquery-ui.min.js"></script>
    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>
    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>
    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>
    <!-- Hive Diagram Library -->
    <script src="/js/hive-diagram.js"></script>

    <script>
    $(function () {
        var config = <?php echo json_encode($jsConfig, JSON_HEX_TAG | JSON_HEX_AMP | JSON_HEX_APOS | JSON_HEX_QUOT); ?>;

        config.diagramContainer = '#hive-stack-diagram';
        config.listContainer = '#hive-stack-list';
        config.tareContainer = '#tare-weight';

        config.onChange = function (ctrl) {
            // Sync hidden form fields
            $('#field-stack-order').val(ctrl.getStackOrder());
            $('#field-sensor-pos').val(ctrl.sensorPosition);
            $('#field-sensor-label').val(ctrl.sensorLabel);
            $('#field-feeder-syrup').val(ctrl.feederHasSyrup ? 1 : 0);
            $('#field-frame-feeder-pos').val(ctrl.frameFeederPosition);
            $('#field-frame-feeder-label').val(ctrl.frameFeederLabel);

            var qtys = ctrl.getQuantities();
            for (var qkey in qtys) {
                $('#qty-' + qkey).val(qtys[qkey]);
            }
        };

        var editor = new HiveDiagram.EditorController(config);
        editor.refresh();
        editor.initSortable();

        // Palette: add component
        $('.palette-card .btn-add').on('click', function (e) {
            e.preventDefault();
            e.stopPropagation();
            var compType = $(this).closest('.palette-card').data('component');
            editor.addComponent(compType);
        });

        // Weight inputs: update tare
        $('.weight-input').on('change', function () {
            var key = $(this).data('weight-key');
            var val = $(this).val();
            editor.updateWeight(key, val);
        });

        // Fixed weight inputs
        $('.fixed-weight-input').on('change', function () {
            var key = $(this).data('fixed-key');
            var val = $(this).val();
            editor.updateFixedWeight(key, val);
        });

        // Sensor label
        $('#sensor-label-input').on('change', function () {
            editor.sensorLabel = $(this).val();
            editor.refresh();
        });

        // Feeder syrup
        $('#feeder-syrup-check').on('change', function () {
            editor.setFeederSyrup($(this).is(':checked'));
        });

        // Frame feeder label
        $('#frame-feeder-label-input').on('change', function () {
            editor.frameFeederLabel = $(this).val();
            editor.refresh();
        });
    });
    </script>

</body>
</html>
