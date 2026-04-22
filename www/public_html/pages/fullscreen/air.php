

<?PHP

function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}

$period = "";
$chart = "";

if(isset($_GET["period"]) && !empty($_GET["period"])) {
    $period = test_input($_GET["period"]);
} else {
    $period = "today";
}

if(isset($_GET["chart"]) && !empty($_GET["chart"])) {
    $chart = test_input($_GET["chart"]);
} else {
    $chart = "line";
}

?>

<!DOCTYPE html>
<html lang="en">

<head>

    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>Air Quality - HiveControl</title>

    <!-- Bootstrap Core CSS -->
    <link href="/bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">

    <!-- MetisMenu CSS -->
    <link href="/bower_components/metisMenu/dist/metisMenu.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="/dist/css/sb-admin-2.css" rel="stylesheet">

    <!-- Custom Fonts -->
    <link href="/bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

</head>

<body>
            <div class="row">
                <div class="col-lg-12">
                <button><a href="JavaScript:window.close()">Close</a></button>
                </div>
            </div>
            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-heading">Particulate Matter (ug/m3)</div>
                        <div class="panel-body">
                            <div id="pm-chart"></div>
                        </div>
                    </div>
                    <div class="panel panel-default">
                        <div class="panel-heading">Air Quality Index (AQI)</div>
                        <div class="panel-body">
                            <div id="aqi-chart"></div>
                        </div>
                    </div>
                </div>
            </div>

    <!-- jQuery -->
    <script src="/bower_components/jquery/dist/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="/bower_components/bootstrap/dist/js/bootstrap.min.js"></script>

    <!-- Metis Menu Plugin JavaScript -->
    <script src="/bower_components/metisMenu/dist/metisMenu.min.js"></script>

    <script src="/dist/js/sb-admin-2.js"></script>

    <!-- High Charts -->
    <script src="/js/highcharts/highcharts.js"></script>
    <script src="/js/highcharts/modules/exporting.js"></script>

    <!-- Full Screen Popups -->
    <script src="/js/popup.js"></script>

    <?php $is_fullscreen = true; include($_SERVER["DOCUMENT_ROOT"] . "/pages/datawidgets/air_chart.php"); ?>

</body>

</html>
