

<?PHP

//Check input for badness
function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}


$period = test_input($_GET["period"]);
$chart = test_input($_GET["chart"]);

#Check to see if the neccessary variables exist

if(isset($_GET["period"])) {
    if (empty($_GET["period"])) {
        $error = "ERROR: Period can't be empty";
        } else {
            $period = test_input($_GET["period"]);
        }
    } else {
    $error = "ERROR: Must specify a period";
}

if(isset($_GET["chart"])) {
    if (empty($_GET["chart"])) {
        $error = "ERROR: Chart can't be empty";
        } else {
            $chart = test_input($_GET["chart"]);
        }
    } else {
    $error =  "ERROR: Must specify chart";
}

?>



<!DOCTYPE html>
<html lang="en">

<head>

    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">

    <title>Hive Control - Pollen</title>

    <!-- Bootstrap Core CSS -->
    <link href="/bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">

    <!-- MetisMenu CSS -->
    <link href="/bower_components/metisMenu/dist/metisMenu.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="/dist/css/sb-admin-2.css" rel="stylesheet">

    <!-- Custom Fonts -->
    <link href="/bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->

</head>

<body>

           <div class="row">
                <div class="col-lg-12">
            <?PHP if(isset($error)){
                echo '<div class="alert alert-danger">'; echo $error; echo'</div>';} ?>
                <button><a href="JavaScript:window.close()">Close</a></button>
                </div>

                </div>
            <!-- /.row -->
            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-body">
                            <div class="pull-center" id="container"></div>
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

    <?php
    include($_SERVER["DOCUMENT_ROOT"] . "/pages/datawidgets/pollen_chart.php"); ?>


</body>

</html>
