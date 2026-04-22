

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
                    <h1 class="page-header">Hive Temp/Humidity Analysis</h1>
                </div>
            </div>

            <!-- Context-Aware Alerts -->
            <div class="row">
                <div class="col-lg-12">
                    <?php
                    include_once($_SERVER["DOCUMENT_ROOT"] . "/include/alert_engine.php");
                    $page_alerts = get_page_alerts($conn, 'temp');
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
            <a href="/pages/temp.php?chart=line&period=today"><button type="button" class="btn btn-<?PHP if ($period == "today"){echo "primary";} else {echo "default";}?>">Today</button></a>
            <a href="/pages/temp.php?chart=line&period=day"><button type="button" class="btn btn-<?PHP if ($period == "day"){echo "primary";} else {echo "default";}?>">24 hr</button></a>
            <a href="/pages/temp.php?chart=line&period=week"><button type="button" class="btn btn-<?PHP if ($period == "week"){echo "primary";} else {echo "default";}?>">Week</button></a>
            <a href="/pages/temp.php?chart=line&period=month"><button type="button" class="btn btn-<?PHP if ($period == "month"){echo "primary";} else {echo "default";}?>">Month</button></a>
            <a href="/pages/temp.php?chart=line&period=year"><button type="button" class="btn btn-<?PHP if ($period == "year"){echo "primary";} else {echo "default";}?>">Year</button></a>
            <a href="/pages/temp.php?chart=line&period=all"><button type="button" class="btn btn-<?PHP if ($period == "all"){echo "primary";} else {echo "default";}?>">All</button></a>
            <br>
                </div>
            </div>

            <!-- Main Content: Chart + Sidebar -->
            <div class="row" style="margin-top: 10px;">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Temperature / Humidity Analysis
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="tempcontainer"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <?PHP include "datawidgets/stats/temp_stats.php"; ?>

                    <!-- Current Reading -->
                    <?php
                    $current_sth = $conn->prepare("SELECT hivetempf, hivetempc, hiveHum, weather_tempf, weather_tempc, weather_humidity FROM allhivedata ORDER BY datetime(date) DESC LIMIT 1");
                    $current_sth->execute();
                    $current = $current_sth->fetch(PDO::FETCH_ASSOC);
                    $temp_unit = ($SHOW_METRIC == "on") ? "°C" : "°F";
                    $curr_hive_temp = ($SHOW_METRIC == "on") ? $current['hivetempc'] : $current['hivetempf'];
                    $curr_amb_temp = ($SHOW_METRIC == "on") ? $current['weather_tempc'] : $current['weather_tempf'];
                    $curr_hive_hum = $current['hiveHum'];
                    $curr_amb_hum = $current['weather_humidity'];
                    ?>
                    <div class="panel panel-default">
                        <div class="panel-heading">Current Readings</div>
                        <div class="panel-body">
                            <div class="row text-center">
                                <div class="col-xs-6">
                                    <div style="font-size:28px; font-weight:bold; color:#D9534F;">
                                        <?php echo is_numeric($curr_hive_temp) ? round($curr_hive_temp, 1) . $temp_unit : 'N/A'; ?>
                                    </div>
                                    <div style="font-size:12px; color:#666;">Hive Temp</div>
                                </div>
                                <div class="col-xs-6">
                                    <div style="font-size:28px; font-weight:bold; color:#5BC0DE;">
                                        <?php echo is_numeric($curr_amb_temp) ? round($curr_amb_temp, 1) . $temp_unit : 'N/A'; ?>
                                    </div>
                                    <div style="font-size:12px; color:#666;">Ambient Temp</div>
                                </div>
                            </div>
                            <hr style="margin: 10px 0;">
                            <div class="row text-center">
                                <div class="col-xs-6">
                                    <div style="font-size:20px; font-weight:bold; color:#337AB7;">
                                        <?php echo is_numeric($curr_hive_hum) ? round($curr_hive_hum, 0) . '%' : 'N/A'; ?>
                                    </div>
                                    <div style="font-size:12px; color:#666;">Hive Humidity</div>
                                </div>
                                <div class="col-xs-6">
                                    <div style="font-size:20px; font-weight:bold; color:#5CB85C;">
                                        <?php echo is_numeric($curr_amb_hum) ? round($curr_amb_hum, 0) . '%' : 'N/A'; ?>
                                    </div>
                                    <div style="font-size:12px; color:#666;">Ambient Humidity</div>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- Stats -->
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats &mdash; Hive / Ambient</div>
                        <div class="panel-body">
                            <div class="table-responsive">
                                <table class="table table-condensed">
                                    <tbody>
                                        <tr>
                                            <td>Avg Temp (<?PHP echo $temp_unit; ?>)</td>
                                            <td><?PHP echo "$avghivetempf / $avgweather_tempf"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Avg Humidity (%)</td>
                                            <td><?PHP echo "$avghivehum / $avgweather_humidity"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>High (<?PHP echo $temp_unit; ?>)</td>
                                            <td><?PHP echo "$maxhivetempf / $maxweathertempf"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Low (<?PHP echo $temp_unit; ?>)</td>
                                            <td><?PHP echo "$minhivetempf / $minweathertempf"; ?></td>
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
                            <a href="/pages/datawidgets/datajob.php?type=temp_data&period=<?PHP echo htmlspecialchars($period, ENT_QUOTES);?>&output=csv" class="btn btn-default btn-sm"><i class="fa fa-download"></i> CSV</a>
                            <a href="/pages/datawidgets/datajob.php?type=temp_data&period=<?PHP echo htmlspecialchars($period, ENT_QUOTES);?>&output=json" class="btn btn-default btn-sm"><i class="fa fa-download"></i> JSON</a>
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
    <script src="/js/highcharts/highcharts-more.js"></script>
    <script src="/js/highcharts/modules/exporting.js"></script>

    <!-- Full Screen Popups -->
    <script src="/js/popup.js"></script>

    <?php include "datawidgets/temp_chart.php"; ?>

    <!-- Custom Theme JavaScript -->
<!-- Footer -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>
</body>

</html>
