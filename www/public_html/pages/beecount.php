

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

    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

    <div id="wrapper">

            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Hive Activity Analysis</h1>
                </div>
            </div>

            <!-- Context-Aware Alerts -->
            <div class="row">
                <div class="col-lg-12">
                    <?php
                    include_once($_SERVER["DOCUMENT_ROOT"] . "/include/alert_engine.php");
                    $page_alerts = get_page_alerts($conn, 'beecount');
                    foreach ($page_alerts as $alert) {
                        $bs_class = 'alert-info';
                        switch ($alert['severity']) {
                            case 'danger':  $bs_class = 'alert-danger'; break;
                            case 'warning': $bs_class = 'alert-warning'; break;
                            case 'success': $bs_class = 'alert-success'; break;
                        }
                        echo '<div class="alert ' . $bs_class . ' alert-dismissible" role="alert">';
                        echo '<button type="button" class="close" data-dismiss="alert" aria-label="Close"><span aria-hidden="true">&times;</span></button>';
                        echo '<strong><i class="fa ' . htmlspecialchars($alert['icon']) . '"></i> ' . htmlspecialchars($alert['title']) . '</strong> &mdash; ';
                        echo htmlspecialchars($alert['message']);
                        echo '</div>';
                    }
                    ?>
                </div>
            </div>

            <!-- Button Bar -->
            <div class="row">
                <div class="col-lg-12">
            <a href="/pages/beecount.php?chart=line&period=today"><button type="button" class="btn btn-<?PHP if ($period == "today"){echo "primary";} else {echo "default";}?>">Today</button></a>
            <a href="/pages/beecount.php?chart=line&period=day"><button type="button" class="btn btn-<?PHP if ($period == "day"){echo "primary";} else {echo "default";}?>">24 hr</button></a>
            <a href="/pages/beecount.php?chart=line&period=week"><button type="button" class="btn btn-<?PHP if ($period == "week"){echo "primary";} else {echo "default";}?>">Week</button></a>
            <a href="/pages/beecount.php?chart=line&period=month"><button type="button" class="btn btn-<?PHP if ($period == "month"){echo "primary";} else {echo "default";}?>">Month</button></a>
            <a href="/pages/beecount.php?chart=line&period=year"><button type="button" class="btn btn-<?PHP if ($period == "year"){echo "primary";} else {echo "default";}?>">Year</button></a>
            <a href="/pages/beecount.php?chart=line&period=all"><button type="button" class="btn btn-<?PHP if ($period == "all"){echo "primary";} else {echo "default";}?>">All</button></a>
            <br>
                </div>
            </div>

            <!-- Main Content: Chart + Sidebar -->
            <div class="row" style="margin-top: 10px;">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Hive Activity Analysis
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="container"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <?PHP include "datawidgets/stats/beecount_stats.php"; ?>

                    <!-- Current Totals -->
                    <div class="panel panel-default">
                        <div class="panel-heading">Activity Totals</div>
                        <div class="panel-body">
                            <div class="row text-center">
                                <div class="col-xs-6">
                                    <div style="font-size:28px; font-weight:bold; color:#5CB85C;">
                                        <?php echo isset($total_in) && is_numeric($total_in) ? number_format($total_in) : 'N/A'; ?>
                                    </div>
                                    <div style="font-size:12px; color:#666;">Total Bees In</div>
                                </div>
                                <div class="col-xs-6">
                                    <div style="font-size:28px; font-weight:bold; color:#D9534F;">
                                        <?php echo isset($total_out) && is_numeric($total_out) ? number_format($total_out) : 'N/A'; ?>
                                    </div>
                                    <div style="font-size:12px; color:#666;">Total Bees Out</div>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- Download -->
                    <div class="panel panel-default">
                        <div class="panel-heading">Download Raw Data</div>
                        <div class="panel-body">
                            <a href="/pages/datawidgets/datajob.php?type=beecount_data&period=<?PHP echo htmlspecialchars($period, ENT_QUOTES);?>&output=csv" class="btn btn-default btn-sm"><i class="fa fa-download"></i> CSV</a>
                            <a href="/pages/datawidgets/datajob.php?type=beecount_data&period=<?PHP echo htmlspecialchars($period, ENT_QUOTES);?>&output=json" class="btn btn-default btn-sm"><i class="fa fa-download"></i> JSON</a>
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

    <?php include "datawidgets/beecount_chart.php"; ?>

    <!-- Custom Theme JavaScript -->
    <!-- Footer -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>

</body>

</html>
