

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
    $period = "week";
}

if(isset($_GET["chart"]) && !empty($_GET["chart"])) {
    $chart = test_input($_GET["chart"]);
} else {
    $chart = "line";
}

?>

<!DOCTYPE html>
<html lang="en">

    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

    <div id="wrapper">

            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Growing Degree Days Analysis</h1>
                </div>
            </div>

            <!-- Button Bar -->
            <div class="row">
                <div class="col-lg-12">
            <a href="/pages/gdd.php?chart=line&period=week"><button type="button" class="btn btn-<?PHP if ($period == "week"){echo "primary";} else {echo "default";}?>">Week</button></a>
            <a href="/pages/gdd.php?chart=line&period=month"><button type="button" class="btn btn-<?PHP if ($period == "month"){echo "primary";} else {echo "default";}?>">Month</button></a>
            <a href="/pages/gdd.php?chart=line&period=year"><button type="button" class="btn btn-<?PHP if ($period == "year"){echo "primary";} else {echo "default";}?>">Year</button></a>
            <a href="/pages/gdd.php?chart=line&period=all"><button type="button" class="btn btn-<?PHP if ($period == "all"){echo "primary";} else {echo "default";}?>">All</button></a>
            <br>
                </div>
            </div>

            <!-- Main Content: Chart + Sidebar -->
            <div class="row" style="margin-top: 10px;">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Growing Degree Days
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="container"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <?PHP include "datawidgets/stats/gdd_stats.php"; ?>

                    <!-- Download -->
                    <div class="panel panel-default">
                        <div class="panel-heading">Download Raw Data</div>
                        <div class="panel-body">
                            <a href="/pages/datawidgets/datajob.php?type=gdd_data&period=<?PHP echo htmlspecialchars($period, ENT_QUOTES);?>&output=csv" class="btn btn-default btn-sm"><i class="fa fa-download"></i> CSV</a>
                            <a href="/pages/datawidgets/datajob.php?type=gdd_data&period=<?PHP echo htmlspecialchars($period, ENT_QUOTES);?>&output=json" class="btn btn-default btn-sm"><i class="fa fa-download"></i> JSON</a>
                        </div>
                    </div>

                </div>
            </div>

    </div>
    <!-- /#wrapper -->

    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>

    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>

    <script src="../dist/js/sb-admin-2.js"></script>

    <!-- High Charts -->
    <script src="/js/highcharts/highcharts.js"></script>
    <script src="/js/highcharts/modules/exporting.js"></script>

    <!-- Full Screen Popups -->
    <script src="/js/popup.js"></script>

    <?php include "datawidgets/gdd_chart.php"; ?>

    <!-- Custom Theme JavaScript -->
    <!-- Footer -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>

</body>

</html>
