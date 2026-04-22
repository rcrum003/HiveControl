

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
                    <h1 class="page-header">Air Quality Analysis</h1>
                </div>
            </div>

            <!-- Context-Aware Alerts -->
            <div class="row">
                <div class="col-lg-12">
                    <?php
                    include_once($_SERVER["DOCUMENT_ROOT"] . "/include/alert_engine.php");
                    $page_alerts = get_page_alerts($conn, 'air');
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
            <a href="/pages/air.php?chart=line&period=today"><button type="button" class="btn btn-<?PHP if ($period == "today"){echo "primary";} else {echo "default";}?>">Today</button></a>
            <a href="/pages/air.php?chart=line&period=day"><button type="button" class="btn btn-<?PHP if ($period == "day"){echo "primary";} else {echo "default";}?>">24 hr</button></a>
            <a href="/pages/air.php?chart=line&period=week"><button type="button" class="btn btn-<?PHP if ($period == "week"){echo "primary";} else {echo "default";}?>">Week</button></a>
            <a href="/pages/air.php?chart=line&period=month"><button type="button" class="btn btn-<?PHP if ($period == "month"){echo "primary";} else {echo "default";}?>">Month</button></a>
            <a href="/pages/air.php?chart=line&period=year"><button type="button" class="btn btn-<?PHP if ($period == "year"){echo "primary";} else {echo "default";}?>">Year</button></a>
            <a href="/pages/air.php?chart=line&period=all"><button type="button" class="btn btn-<?PHP if ($period == "all"){echo "primary";} else {echo "default";}?>">All</button></a>

            <span style="margin-left: 20px;">
                <a href="/pages/air_correlation.php?period=<?PHP echo htmlspecialchars($period, ENT_QUOTES); ?>"><button type="button" class="btn btn-sm btn-info"><i class="fa fa-bar-chart"></i> Bee Impact Correlations</button></a>
            </span>
            <br>
                </div>
            </div>

            <!-- Main Content: Charts + Sidebar -->
            <div class="row" style="margin-top: 10px;">
                <div class="col-lg-8">
                    <!-- Chart 1: Particulate Matter Concentrations -->
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Particulate Matter (ug/m3)
                        </div>
                        <div class="panel-body">
                            <div id="pm-chart"></div>
                        </div>
                    </div>

                    <!-- Chart 2: Air Quality Index -->
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Air Quality Index (AQI)
                        </div>
                        <div class="panel-body">
                            <div id="aqi-chart"></div>
                        </div>
                    </div>
                </div>

                <div class="col-lg-4">
                    <?PHP include "datawidgets/stats/air_stats.php"; ?>

                    <!-- Current AQI Badge -->
                    <?php
                    $current_aqi = null;
                    $current_pm25 = null;
                    $aqi_category = '';
                    $aqi_color = '#999999';
                    $aqi_text_color = '#FFFFFF';

                    $aqi_sth = $conn->prepare("SELECT air_pm2_5_aqi, air_pm2_5_raw, air_pm2_5 FROM allhivedata WHERE (air_pm2_5_aqi IS NOT NULL OR air_pm2_5 IS NOT NULL) ORDER BY datetime(date) DESC LIMIT 1");
                    $aqi_sth->execute();
                    $aqi_row = $aqi_sth->fetch(PDO::FETCH_ASSOC);
                    if ($aqi_row) {
                        $current_aqi = is_numeric($aqi_row['air_pm2_5_aqi']) ? intval($aqi_row['air_pm2_5_aqi']) : null;
                        $current_pm25 = is_numeric($aqi_row['air_pm2_5_raw']) ? floatval($aqi_row['air_pm2_5_raw']) : (is_numeric($aqi_row['air_pm2_5']) ? floatval($aqi_row['air_pm2_5']) : null);
                    }

                    if ($current_aqi !== null) {
                        if ($current_aqi <= 50) {
                            $aqi_category = 'Good';
                            $aqi_color = '#00E400';
                            $aqi_text_color = '#000000';
                        } elseif ($current_aqi <= 100) {
                            $aqi_category = 'Moderate';
                            $aqi_color = '#FFFF00';
                            $aqi_text_color = '#000000';
                        } elseif ($current_aqi <= 150) {
                            $aqi_category = 'Unhealthy for Sensitive Groups';
                            $aqi_color = '#FF7E00';
                            $aqi_text_color = '#000000';
                        } elseif ($current_aqi <= 200) {
                            $aqi_category = 'Unhealthy';
                            $aqi_color = '#FF0000';
                            $aqi_text_color = '#FFFFFF';
                        } elseif ($current_aqi <= 300) {
                            $aqi_category = 'Very Unhealthy';
                            $aqi_color = '#8F3F97';
                            $aqi_text_color = '#FFFFFF';
                        } else {
                            $aqi_category = 'Hazardous';
                            $aqi_color = '#7E0023';
                            $aqi_text_color = '#FFFFFF';
                        }
                    }
                    ?>
                    <div class="panel panel-default">
                        <div class="panel-heading">Current Air Quality</div>
                        <div class="panel-body text-center">
                            <?php if ($current_aqi !== null) { ?>
                            <div style="display:inline-block; background:<?php echo $aqi_color; ?>; color:<?php echo $aqi_text_color; ?>; border-radius:12px; padding:15px 30px; margin-bottom:10px;">
                                <div style="font-size:36px; font-weight:bold; line-height:1.1;"><?php echo $current_aqi; ?></div>
                                <div style="font-size:13px;"><?php echo htmlspecialchars($aqi_category); ?></div>
                            </div>
                            <?php if ($current_pm25 !== null) { ?>
                            <div style="font-size:13px; color:#666; margin-top:5px;">
                                PM2.5: <?php echo round($current_pm25, 1); ?> ug/m3
                            </div>
                            <?php } ?>
                            <?php } else { ?>
                            <div style="color:#999;">No current data</div>
                            <?php } ?>
                        </div>
                    </div>

                    <!-- PM2.5 Stats -->
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats &mdash; PM 2.5</div>
                        <div class="panel-body">
                            <div class="table-responsive">
                                <table class="table table-condensed">
                                    <tbody>
                                        <tr>
                                            <td>Avg (ug/m3)</td>
                                            <td><?PHP echo "$avgair_pm2_5"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Start / End</td>
                                            <td><?PHP echo "$startair / $endair"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Max / Min</td>
                                            <td><?PHP echo "$maxair_pm2_5 / $minair_pm2_5"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Avg AQI</td>
                                            <td><?PHP echo "$avgair_pm2_5_aqi"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Max AQI</td>
                                            <td><?PHP echo "$maxair_pm2_5_aqi"; ?></td>
                                        </tr>
                                    </tbody>
                                </table>
                            </div>
                        </div>
                    </div>

                    <!-- PM10 Stats -->
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats &mdash; PM 10</div>
                        <div class="panel-body">
                            <div class="table-responsive">
                                <table class="table table-condensed">
                                    <tbody>
                                        <tr>
                                            <td>Avg (ug/m3)</td>
                                            <td><?PHP echo "$avgair_pm10_raw"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Max / Min</td>
                                            <td><?PHP echo "$maxair_pm10_raw / $minair_pm10_raw"; ?></td>
                                        </tr>
                                    </tbody>
                                </table>
                            </div>
                        </div>
                    </div>

                    <!-- EPA Stats (conditional) -->
                    <?PHP if (!empty($avg_o3_aqi) || !empty($avg_no2_aqi)) { ?>
                    <div class="panel panel-default">
                        <div class="panel-heading">Stats &mdash; EPA (O3 / NO2)</div>
                        <div class="panel-body">
                            <div class="table-responsive">
                                <table class="table table-condensed">
                                    <tbody>
                                        <tr>
                                            <td>Avg O3 AQI</td>
                                            <td><?PHP echo "$avg_o3_aqi"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Max O3 AQI</td>
                                            <td><?PHP echo "$max_o3_aqi"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Avg NO2 AQI</td>
                                            <td><?PHP echo "$avg_no2_aqi"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Max NO2 AQI</td>
                                            <td><?PHP echo "$max_no2_aqi"; ?></td>
                                        </tr>
                                    </tbody>
                                </table>
                            </div>
                        </div>
                    </div>
                    <?PHP } ?>

                    <!-- Download -->
                    <div class="panel panel-default">
                        <div class="panel-heading">Download Raw Data</div>
                        <div class="panel-body">
                            <a href="/pages/datawidgets/datajob.php?type=air_data&period=<?PHP echo htmlspecialchars($period, ENT_QUOTES);?>&output=csv" class="btn btn-default btn-sm"><i class="fa fa-download"></i> CSV</a>
                            <a href="/pages/datawidgets/datajob.php?type=air_data&period=<?PHP echo htmlspecialchars($period, ENT_QUOTES);?>&output=json" class="btn btn-default btn-sm"><i class="fa fa-download"></i> JSON</a>
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

    <?php include "datawidgets/air_chart.php"; ?>

    <!-- Custom Theme JavaScript -->
<!-- Footer -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>
</body>

</html>
