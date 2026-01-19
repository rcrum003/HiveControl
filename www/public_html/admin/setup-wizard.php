<?PHP
// Setup Wizard - First Time Installation Guide
// This wizard guides users through the initial configuration

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

$step = isset($_GET['step']) ? (int)$_GET['step'] : 1;
$errors = [];
$success = false;

// Check if setup is already complete
function isSetupComplete($conn) {
    try {
        $sth = $conn->prepare("SELECT HIVENAME, HIVEAPI, CITY, STATE FROM hiveconfig WHERE id=1");
        $sth->execute();
        $result = $sth->fetch(PDO::FETCH_ASSOC);

        if ($result &&
            !empty($result['HIVENAME']) &&
            $result['HIVENAME'] !== 'NOTSET' &&
            !empty($result['HIVEAPI']) &&
            trim($result['HIVEAPI']) !== '' &&
            !empty($result['CITY']) &&
            !empty($result['STATE'])) {
            return true;
        }
        return false;
    } catch (Exception $e) {
        return false;
    }
}

// If already configured and trying to access wizard, redirect to settings
if (isSetupComplete($conn) && !isset($_GET['force'])) {
    header("Location: /admin/hiveconfig.php");
    exit();
}

// Process form submission
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $v = new Valitron\Validator($_POST);
    $v->rule('required', ['HIVENAME', 'HIVEAPI', 'CITY', 'STATE'])->message('{field} is required');
    $v->rule('regex', 'HIVENAME', '/^[a-zA-Z0-9_-]+$/')->message('Hive Name can only contain letters, numbers, dashes, and underscores');
    $v->rule('alphaNum', ['HIVEAPI']);
    $v->rule('lengthmax', ['HIVENAME', 'CITY', 'STATE'], 40);
    $v->rule('lengthmax', ['HIVEAPI'], 70);

    if ($v->validate()) {
        try {
            // Sanitize inputs
            $hivename = htmlspecialchars(trim($_POST['HIVENAME']));
            $hiveapi = htmlspecialchars(trim($_POST['HIVEAPI']));
            $city = htmlspecialchars(trim($_POST['CITY']));
            $state = htmlspecialchars(trim($_POST['STATE']));

            // Update the database with mandatory fields
            $sth = $conn->prepare("UPDATE hiveconfig SET hivename=?, hiveapi=?, city=?, state=? WHERE id=1");
            $sth->execute(array($hivename, $hiveapi, $city, $state));

            $success = true;
        } catch (Exception $e) {
            $errors[] = "Failed to save configuration: " . $e->getMessage();
        }
    } else {
        $errors = array_values($v->errors());
    }
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>HiveControl - Setup Wizard</title>

    <!-- Bootstrap Core CSS -->
    <link href="../bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">
    <link href="../bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">
    <link href="../dist/css/sb-admin-2.css" rel="stylesheet">

    <style>
        body {
            background-color: #f8f8f8 !important;
            background-image: none !important;
            min-height: 100vh;
            padding: 20px 0;
        }
        .wizard-container {
            max-width: 800px;
            margin: 40px auto;
            background: white;
            border-radius: 4px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            overflow: hidden;
            border: 1px solid #e7e7e7;
        }
        .wizard-header {
            background: #e6b800;
            color: white;
            padding: 30px;
            text-align: center;
            border-bottom: 1px solid #d4a600;
        }
        .wizard-header h1 {
            margin: 0;
            font-size: 32px;
            font-weight: bold;
            text-shadow: 0 1px 2px rgba(0,0,0,0.1);
        }
        .wizard-header p {
            margin: 10px 0 0 0;
            font-size: 16px;
            opacity: 0.95;
        }
        .wizard-body {
            padding: 40px;
        }
        .form-group label {
            font-weight: 600;
            color: #333;
        }
        .form-control {
            height: 40px;
            border-radius: 4px;
            border: 1px solid #ccc;
            font-size: 14px;
        }
        .form-control:focus {
            border-color: #e6b800;
            box-shadow: inset 0 1px 1px rgba(0,0,0,.075), 0 0 8px rgba(230, 184, 0, .6);
        }
        .btn-primary {
            background-color: #e6b800;
            border-color: #d4a600;
            padding: 10px 35px;
            font-size: 14px;
            font-weight: 600;
            border-radius: 4px;
        }
        .btn-primary:hover, .btn-primary:focus {
            background-color: #d4a600;
            border-color: #c29900;
        }
        .btn-default {
            background-color: #996633;
            border-color: #885a2d;
            color: white;
            padding: 10px 35px;
            font-size: 14px;
            font-weight: 600;
            border-radius: 4px;
        }
        .btn-default:hover, .btn-default:focus {
            background-color: #885a2d;
            border-color: #774e26;
            color: white;
        }
        .help-text {
            font-size: 13px;
            color: #737373;
            margin-top: 5px;
        }
        .required-label:after {
            content: " *";
            color: #d9534f;
        }
        .alert {
            border-radius: 4px;
        }
        .success-container {
            text-align: center;
            padding: 40px 20px;
        }
        .success-icon {
            font-size: 80px;
            color: #5cb85c;
            margin-bottom: 20px;
        }
        .step-indicator {
            background: #f5f5f5;
            padding: 15px;
            border-bottom: 1px solid #e7e7e7;
            text-align: center;
        }
        .step-indicator span {
            display: inline-block;
            padding: 5px 15px;
            background: #996633;
            color: white;
            border-radius: 4px;
            font-weight: 600;
            font-size: 13px;
        }
        .wizard-header .fa {
            color: white;
            text-shadow: 0 1px 2px rgba(0,0,0,0.1);
        }
    </style>
</head>
<body>
    <div class="wizard-container">
        <div class="wizard-header">
            <i class="fa fa-home" style="font-size: 50px;"></i>
            <h1>Welcome to HiveControl!</h1>
            <p>Let's get your hive monitoring system set up</p>
        </div>

        <div class="step-indicator">
            <span>Step 1 of 1: Essential Configuration</span>
        </div>

        <div class="wizard-body">
            <?php if ($success): ?>
                <div class="success-container">
                    <div class="success-icon">
                        <i class="fa fa-check-circle"></i>
                    </div>
                    <h2>Setup Complete!</h2>
                    <p class="lead">Your HiveControl system is now configured with the essential settings.</p>
                    <p>You can now access your dashboard or configure additional settings.</p>
                    <div style="margin-top: 30px;">
                        <a href="/pages/index.php" class="btn btn-primary btn-lg" style="margin-right: 10px;">
                            <i class="fa fa-dashboard"></i> Go to Dashboard
                        </a>
                        <a href="/admin/instrumentconfig.php" class="btn btn-default btn-lg">
                            <i class="fa fa-cog"></i> Configure Instruments
                        </a>
                    </div>
                </div>
            <?php else: ?>
                <?php if (!empty($errors)): ?>
                    <div class="alert alert-danger alert-dismissable">
                        <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>
                        <strong>Please fix the following errors:</strong>
                        <ul style="margin: 10px 0 0 0;">
                            <?php foreach ($errors as $error): ?>
                                <li><?php echo is_array($error) ? $error[0] : $error; ?></li>
                            <?php endforeach; ?>
                        </ul>
                    </div>
                <?php endif; ?>

                <div class="alert alert-info">
                    <i class="fa fa-info-circle"></i> <strong>Required Fields</strong> - Please complete all mandatory fields marked with <span style="color: #d9534f;">*</span>
                </div>

                <form method="POST" action="">
                    <div class="form-group">
                        <label class="required-label" for="HIVENAME">Hive Name</label>
                        <input type="text" class="form-control" id="HIVENAME" name="HIVENAME"
                               value="<?php echo isset($_POST['HIVENAME']) ? htmlspecialchars($_POST['HIVENAME']) : ''; ?>"
                               placeholder="e.g., Hive-01, Backyard-Hive" required>
                        <p class="help-text">Give your hive a unique, identifiable name</p>
                    </div>

                    <div class="form-group">
                        <label class="required-label" for="HIVEAPI">Hive API Key</label>
                        <input type="text" class="form-control" id="HIVEAPI" name="HIVEAPI"
                               value="<?php echo isset($_POST['HIVEAPI']) ? htmlspecialchars($_POST['HIVEAPI']) : ''; ?>"
                               placeholder="Enter your API key from HiveControl.org" required>
                        <p class="help-text">
                            Register at <a href="https://www.hivecontrol.org/" target="_blank">HiveControl.org</a>,
                            go to "Your Settings" → "API" to create a key. You can use one key for all hives.
                        </p>
                    </div>

                    <div class="form-group">
                        <label class="required-label" for="CITY">City</label>
                        <input type="text" class="form-control" id="CITY" name="CITY"
                               value="<?php echo isset($_POST['CITY']) ? htmlspecialchars($_POST['CITY']) : ''; ?>"
                               placeholder="e.g., Portland" required>
                        <p class="help-text">Closest city to your hive for weather data</p>
                    </div>

                    <div class="form-group">
                        <label class="required-label" for="STATE">State</label>
                        <input type="text" class="form-control" id="STATE" name="STATE"
                               value="<?php echo isset($_POST['STATE']) ? htmlspecialchars($_POST['STATE']) : ''; ?>"
                               placeholder="e.g., Oregon" required>
                        <p class="help-text">State or province where your hive is located</p>
                    </div>

                    <div style="margin-top: 30px; text-align: center;">
                        <button type="submit" class="btn btn-primary btn-lg">
                            <i class="fa fa-check"></i> Complete Setup
                        </button>
                    </div>

                    <div style="margin-top: 20px; text-align: center; color: #999;">
                        <small>You can configure additional settings like instruments and sensors after completing this setup</small>
                    </div>
                </form>
            <?php endif; ?>
        </div>
    </div>

    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>
    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>
</body>
</html>
