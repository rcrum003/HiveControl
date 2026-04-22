

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
                    <h1 class="page-header">Weight Analysis</h1>
                </div>
            </div>

            <!-- Context-Aware Alerts -->
            <div class="row">
                <div class="col-lg-12">
                    <?php
                    include_once($_SERVER["DOCUMENT_ROOT"] . "/include/alert_engine.php");
                    $page_alerts = get_page_alerts($conn, 'weight');
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
            <a href="/pages/weight.php?chart=line&period=today"><button type="button" class="btn btn-<?PHP if ($period == "today"){echo "primary";} else {echo "default";}?>">Today</button></a>
            <a href="/pages/weight.php?chart=line&period=day"><button type="button" class="btn btn-<?PHP if ($period == "day"){echo "primary";} else {echo "default";}?>">24 hr</button></a>
            <a href="/pages/weight.php?chart=line&period=week"><button type="button" class="btn btn-<?PHP if ($period == "week"){echo "primary";} else {echo "default";}?>">Week</button></a>
            <a href="/pages/weight.php?chart=line&period=month"><button type="button" class="btn btn-<?PHP if ($period == "month"){echo "primary";} else {echo "default";}?>">Month</button></a>
            <a href="/pages/weight.php?chart=line&period=year"><button type="button" class="btn btn-<?PHP if ($period == "year"){echo "primary";} else {echo "default";}?>">Year</button></a>
            <a href="/pages/weight.php?chart=line&period=all"><button type="button" class="btn btn-<?PHP if ($period == "all"){echo "primary";} else {echo "default";}?>">All</button></a>
            <br>
                </div>
            </div>

            <!-- Main Content: Chart + Sidebar -->
            <div class="row" style="margin-top: 10px;">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Weight Analysis
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="weightcontainer"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <?PHP include "datawidgets/stats/weight_stats.php"; ?>

                    <?php
                    $weight_unit = ($SHOW_METRIC == "on") ? "kg" : "lb";
                    $curr_weight_sth = $conn->prepare("SELECT hiveweight, hiverawweight FROM allhivedata WHERE hiveweight > 0 ORDER BY datetime(date) DESC LIMIT 1");
                    $curr_weight_sth->execute();
                    $curr_w = $curr_weight_sth->fetch(PDO::FETCH_ASSOC);
                    $curr_net = null; $curr_gross = null;
                    if ($curr_w) {
                        $curr_net = floatval($curr_w['hiveweight']);
                        $curr_gross = is_numeric($curr_w['hiverawweight']) ? floatval($curr_w['hiverawweight']) : null;
                        if ($SHOW_METRIC == "on") {
                            $curr_net = round($curr_net * 0.453592, 2);
                            if ($curr_gross !== null) $curr_gross = round($curr_gross * 0.453592, 2);
                        }
                    }
                    ?>

                    <!-- Current Reading -->
                    <div class="panel panel-default">
                        <div class="panel-heading">Current Weight</div>
                        <div class="panel-body text-center">
                            <?php if ($curr_net !== null) { ?>
                            <div style="font-size:32px; font-weight:bold; color:#5CB85C;">
                                <?php echo round($curr_net, 1) . ' ' . $weight_unit; ?>
                            </div>
                            <div style="font-size:12px; color:#666;">Net Weight</div>
                            <?php if ($curr_gross !== null) { ?>
                            <div style="font-size:18px; color:#999; margin-top:5px;">
                                <?php echo round($curr_gross, 1) . ' ' . $weight_unit; ?> gross
                            </div>
                            <?php } ?>
                            <?php if (isset($diffweight) && is_numeric($diffweight)) {
                                $diff_color = ($diffweight >= 0) ? '#5CB85C' : '#D9534F';
                                $diff_icon = ($diffweight >= 0) ? 'fa-arrow-up' : 'fa-arrow-down';
                            ?>
                            <div style="margin-top:8px; font-size:14px; color:<?php echo $diff_color; ?>;">
                                <i class="fa <?php echo $diff_icon; ?>"></i> <?php echo $diffweight . ' ' . $weight_unit; ?> this period
                            </div>
                            <?php } ?>
                            <?php } else { ?>
                            <div style="color:#999;">No weight data</div>
                            <?php } ?>
                        </div>
                    </div>

                    <!-- Stats -->
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats &mdash; Net / Gross</div>
                        <div class="panel-body">
                            <div class="table-responsive">
                                <table class="table table-condensed">
                                    <tbody>
                                        <tr>
                                            <td>Avg (<?PHP echo $weight_unit; ?>)</td>
                                            <td><?PHP echo "$avghiveweight / $avghiverawweight"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Start / End</td>
                                            <td><?PHP echo "$startweight / $endweight"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Gain / Loss</td>
                                            <td><?PHP echo "$diffweight"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Max</td>
                                            <td><?PHP echo "$maxhiveweight / $maxhiverawweight"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Min</td>
                                            <td><?PHP echo "$minhiveweight / $minhiverawweight"; ?></td>
                                        </tr>
                                    </tbody>
                                </table>
                            </div>
                        </div>
                    </div>

                    <!-- Download -->
                    <div class="panel panel-default">
                        <div class="panel-heading">Download Raw Data</div>
                        <div class="panel-body">
                            <a href="/pages/datawidgets/datajob.php?type=weight_data&period=<?PHP echo htmlspecialchars($period, ENT_QUOTES);?>&output=csv" class="btn btn-default btn-sm"><i class="fa fa-download"></i> CSV</a>
                            <a href="/pages/datawidgets/datajob.php?type=weight_data&period=<?PHP echo htmlspecialchars($period, ENT_QUOTES);?>&output=json" class="btn btn-default btn-sm"><i class="fa fa-download"></i> JSON</a>
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

    <?php include "datawidgets/weight_chart.php"; ?>

    <!-- Custom Theme JavaScript -->
<!-- Footer -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>
</body>

</html>
