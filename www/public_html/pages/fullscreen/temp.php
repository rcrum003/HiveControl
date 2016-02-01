

<?PHP

//Pass variables to determine the type of view
// Make just one page that can show day/month/year, etc
// Functions
// period 

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
    // type exists
    if (empty($_GET["period"])) {
        $error = "ERROR: Period can't be empty";
        } else {
            $period = test_input($_GET["period"]);
            #echo "The Period is $period";
        }
    } else {
    $error = "ERROR: Must specify a period";
}

if(isset($_GET["chart"])) {
    // type exists
    if (empty($_GET["chart"])) {
        $error = "ERROR: Chart can't be empty";
        } else {
            $chart = test_input($_GET["chart"]);
            #echo "The Period is $period";
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

    <title>Hive Control</title>

    <!-- Bootstrap Core CSS -->
    <link href="/bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">

    <!-- MetisMenu CSS -->
    <link href="/bower_components/metisMenu/dist/metisMenu.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="/dist/css/sb-admin-2.css" rel="stylesheet">

    <!-- Custom Fonts -->
    <link href="/bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->

</head>

<body>

    <div id="wrapper">

        <!-- Navigation -->
        
       <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/topmenu.php"); ?>
        <!-- /Navigation -->

        
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Hive Temp/Humidity Analysis</h1>
            <?PHP if(isset($error)){ 
                echo '<div class="alert alert-danger">'; echo $error; echo'</div>';} ?>
                </div>
                <!-- /.col-lg-12 -->
                        </div>
            <div class="row">
                <div class="col-lg-12">
        
        <!-- Button Bar -->
            <a href="/pages/fullscreen/temp.php?chart=line&period=day"><button type="button" class="btn btn btn-<?PHP if ($period == "day"){echo "primary";} else {echo "default";}?>">Day</button></a>
            <a href="/pages/fullscreen/temp.php?chart=line&period=week"><button type="button" class="btn btn btn-<?PHP if ($period == "week"){echo "primary";} else {echo "default";}?>">Week</button></a>
            <a href="/pages/fullscreen/temp.php?chart=line&period=month"><button type="button" class="btn btn btn-<?PHP if ($period == "month"){echo "primary";} else {echo "default";}?>">Month</button></a>
            <a href="/pages/fullscreen/temp.php?chart=line&period=year"><button type="button" class="btn btn btn-<?PHP if ($period == "year"){echo "primary";} else {echo "default";}?>">Year</button></a>
            <a href="/pages/fullscreen/temp.php?chart=line&period=all"><button type="button" class="btn btn btn-<?PHP if ($period == "all"){echo "primary";} else {echo "default";}?>">All</button></a>
            <br>
                </div>
                <!-- /.col-lg-12 -->
                        </div>
            
            <!-- /.row -->

            <!-- /.row -->
            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Temperature and Humidity Analyis 
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="container"></div>
                        </div>
                    </div>
                </div>
            </div>
                        </div>
                        <!-- /.panel-footer -->
                    </div>
                    <!-- /.panel .chat-panel -->
                </div>
                <!-- /.col-lg-4 -->
            </div>
            <!-- /.row -->
        </div>
        <!-- /#page-wrapper -->

    </div>
    <!-- /#wrapper -->

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
        #Since this document already has variables, we can pass variables by just including it.
        # Period and chart variables will be used in the charts.
    
    include($_SERVER["DOCUMENT_ROOT"] . "/pages/datawidgets/temp_chart.php"); ?>
    
    
    <!-- Custom Theme JavaScript -->

</body>

</html>
