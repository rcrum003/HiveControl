<?PHP
// HiveControl Help Documentation
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Get the help topic from URL parameter
$topic = isset($_GET['topic']) ? htmlspecialchars($_GET['topic']) : 'overview';
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>HiveControl - Help & Documentation</title>

    <!-- Bootstrap Core CSS -->
    <link href="../bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">
    <link href="../bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">
    <link href="../dist/css/sb-admin-2.css" rel="stylesheet">

    <style>
        .help-sidebar {
            background: #f5f5f5;
            border-right: 1px solid #e7e7e7;
            padding: 20px;
            min-height: calc(100vh - 50px);
        }
        .help-sidebar h4 {
            color: #996633;
            font-weight: 600;
            margin-bottom: 15px;
        }
        .help-sidebar .list-group-item {
            border: none;
            background: transparent;
            padding: 8px 15px;
        }
        .help-sidebar .list-group-item.active {
            background-color: #e6b800;
            border-color: #e6b800;
        }
        .help-sidebar .list-group-item:hover {
            background-color: #fff;
        }
        .help-content {
            padding: 30px;
        }
        .help-content h1 {
            color: #333;
            margin-bottom: 20px;
            padding-bottom: 10px;
            border-bottom: 3px solid #e6b800;
        }
        .help-content h2 {
            color: #996633;
            margin-top: 30px;
            margin-bottom: 15px;
        }
        .help-content h3 {
            color: #555;
            margin-top: 20px;
            margin-bottom: 10px;
        }
        .alert-info {
            border-left: 4px solid #e6b800;
        }
        .code-block {
            background: #f5f5f5;
            border: 1px solid #e7e7e7;
            padding: 15px;
            border-radius: 4px;
            margin: 15px 0;
        }
        .step-number {
            display: inline-block;
            background: #e6b800;
            color: white;
            width: 30px;
            height: 30px;
            line-height: 30px;
            text-align: center;
            border-radius: 50%;
            font-weight: 600;
            margin-right: 10px;
        }
        .feature-box {
            background: #fff;
            border: 1px solid #e7e7e7;
            border-radius: 4px;
            padding: 20px;
            margin-bottom: 20px;
        }
        .feature-box h4 {
            color: #996633;
            margin-top: 0;
        }
    </style>
</head>
<body>
    <!-- Header and Navigation -->
    <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

    <div id="wrapper">
        <div class="row" style="margin: 0;">
            <div class="col-md-3 help-sidebar">
                <h4><i class="fa fa-book"></i> Help Topics</h4>
                <div class="list-group">
                    <a href="?topic=overview" class="list-group-item <?php echo ($topic == 'overview') ? 'active' : ''; ?>">
                        <i class="fa fa-home"></i> Overview
                    </a>
                    <a href="?topic=initial-setup" class="list-group-item <?php echo ($topic == 'initial-setup') ? 'active' : ''; ?>">
                        <i class="fa fa-rocket"></i> Initial Setup
                    </a>
                    <a href="?topic=basic-settings" class="list-group-item <?php echo ($topic == 'basic-settings') ? 'active' : ''; ?>">
                        <i class="fa fa-cog"></i> Basic Settings
                    </a>
                    <a href="?topic=instruments" class="list-group-item <?php echo ($topic == 'instruments') ? 'active' : ''; ?>">
                        <i class="fa fa-tachometer"></i> Instruments & Sensors
                    </a>
                    <a href="?topic=calibration" class="list-group-item <?php echo ($topic == 'calibration') ? 'active' : ''; ?>">
                        <i class="fa fa-balance-scale"></i> Calibration
                    </a>
                    <a href="?topic=weather" class="list-group-item <?php echo ($topic == 'weather') ? 'active' : ''; ?>">
                        <i class="fa fa-cloud"></i> Weather Sources
                    </a>
                    <a href="?topic=dashboard" class="list-group-item <?php echo ($topic == 'dashboard') ? 'active' : ''; ?>">
                        <i class="fa fa-dashboard"></i> Dashboard Guide
                    </a>
                    <a href="?topic=troubleshooting" class="list-group-item <?php echo ($topic == 'troubleshooting') ? 'active' : ''; ?>">
                        <i class="fa fa-wrench"></i> Troubleshooting
                    </a>
                </div>
            </div>

            <div class="col-md-9 help-content">
                <?php
                switch($topic) {
                    case 'overview':
                        include 'help/overview.php';
                        break;
                    case 'initial-setup':
                        include 'help/initial-setup.php';
                        break;
                    case 'basic-settings':
                        include 'help/basic-settings.php';
                        break;
                    case 'instruments':
                        include 'help/instruments.php';
                        break;
                    case 'calibration':
                        include 'help/calibration.php';
                        break;
                    case 'weather':
                        include 'help/weather.php';
                        break;
                    case 'dashboard':
                        include 'help/dashboard.php';
                        break;
                    case 'troubleshooting':
                        include 'help/troubleshooting.php';
                        break;
                    default:
                        include 'help/overview.php';
                }
                ?>
            </div>
        </div>
    </div>

    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>
    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>
    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>
    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>
</body>
</html>
