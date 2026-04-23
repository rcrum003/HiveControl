
<?PHP

//Pass variables to determine the type of view

// Functions
// period - specify what timeperiods you want to see

//Set Variables to null to start
$period="";
$chart_rounding="";

// Check if initial setup is complete - redirect to wizard if not
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
try {
    $setup_check = $conn->prepare("SELECT HIVENAME, CITY, STATE FROM hiveconfig WHERE id=1");
    $setup_check->execute();
    $setup_result = $setup_check->fetch(PDO::FETCH_ASSOC);

    if (!$setup_result ||
        empty($setup_result['HIVENAME']) ||
        $setup_result['HIVENAME'] === 'NOTSET' ||
        empty($setup_result['CITY']) ||
        empty($setup_result['STATE'])) {
        header("Location: /admin/setup-wizard.php");
        exit();
    }
} catch (Exception $e) {
    // If there's a database error, let the page continue but it may fail later
    error_log("Setup check failed: " . $e->getMessage());
}

//Check input for badness
function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}

#Check to see if the neccessary variables exist

if(isset($_GET["period"])) {
    // type exists
    if (empty($_GET["period"])) {
        // Default to Day if no period is set or empty
        $period = "today";
        } else {
            $period = test_input($_GET["period"]);
        }
    } else {
    $period = "today";
}

#Refresh our index.php page every 5 minutes so we get the latest data and can follow along during the day.
$page = $_SERVER['PHP_SELF'];
$sec = "300";
header("Refresh: $sec; url=$page");

?>



<!DOCTYPE html>
<html lang="en">


    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->
    <link rel="stylesheet" href="/css/hive-diagram.css">



    <div id="wrapper">


<?php include "datawidgets/currentconditions.php"; ?>

            <div class="row">
                <div class="col-lg-12">
                    <?php include "datawidgets/alert_banner.php"; ?>
                </div>
            </div>

            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Dashboard</h1>
                    <p style="margin-top:-15px;color:#555"><i class="fa fa-clock-o"></i> Last Sensor Reading: <strong><?php echo trim($recorddate, '()'); ?></strong></p>
                </div>
                <!-- /.col-lg-12 -->
            </div>
            <!-- /.row -->
            <!-- Top Row -->
            <div class="row">
                <div class="col-lg-fifth col-md-6">
                    <div class="panel panel-<?php echo $temp_card_class; ?>">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3">
                                    <img src="../images/temp.png" width="75" height="75">
                                </div>
                                <div class="col-xs-9 text-right">
                                    <div class="h4">
                                    <?php

                                    if ($hivetempf == "null" ) {
                                        $hivetempf="NA";
                                    }
                                    echo "$hivetempf ";

                                    if ($SHOW_METRIC == "on") {
                                        echo " C"; $i = "C";
                                        }
                                    else {
                                         echo "F"; $i = "F";
                                     }
                                if ($wxtempf == "null" ) {
                                        $wxtempf="NA";
                                    }
                                        echo " / "."$wxtempf"." $i";
                                        ?> </div>
                                    <div>Temp - Hive / Ambient<?php if (!empty($temp_delta)) { echo " <small>({$temp_delta}&deg;/hr)</small>"; } ?></div>
                                </div>
                            </div>
                        </div>
                        <a href="/pages/temp.php?chart=line&period=week">
                            <div class="panel-footer">
                                <span class="pull-left">View Details</span>
                                <span class="pull-right"><i class="fa fa-arrow-circle-right"></i></span>
                                <div class="clearfix"></div>
                            </div>
                        </a>
                    </div>
                </div>
                <div class="col-lg-fifth col-md-6">
                    <div class="panel panel-<?php echo $weight_card_class; ?>">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3">
                                     <img src="/images/scalesm.png" width="75" height="75">
                                </div>
                                <div class="col-xs-9 text-right">

                                    <div class="h4"><?php echo "$hiveweight "; if ($SHOW_METRIC == "on") { echo " kg"; $i = "kg"; } else {echo "lb"; $i = "lb";} echo " / "."$changeweight"." $i"; ?> </div>
                                    <div>Weight / Trend<?php if (!empty($weight_trend_text)) { echo " <small>({$weight_trend_text})</small>"; } ?></div>
                                </div>
                            </div>
                        </div>
                        <a href="/pages/weight.php?chart=line&period=week">
                            <div class="panel-footer">
                                <span class="pull-left">View Details</span>
                                <span class="pull-right"><i class="fa fa-arrow-circle-right"></i></span>
                                <div class="clearfix"></div>
                            </div>
                        </a>
                    </div>
                </div>
                <div class="col-lg-fifth col-md-6">
                    <div class="panel panel-primary">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3">
                                     <img src="../images/hum.png" width="75" height="75">
                                </div>
                                <div class="col-xs-9 text-right">
                                    <div class="h4"><?php
                                    if ($hivehumi == "null" ) {
                                        $hivehumi="NA";
                                    }
                                    if ($wxhumi == "null" ) {
                                        $wxhumi="NA";
                                    }

                                    echo "$hivehumi"." / "."$wxhumi"; ?>%</div>
                                    <div>Humidity - Hive / Ambient<?php if (!empty($hum_delta)) { echo " <small>({$hum_delta}/hr)</small>"; } ?></div>
                                </div>
                            </div>
                        </div>
                        <a href="/pages/temp.php?chart=line&period=week">
                            <div class="panel-footer">
                                <span class="pull-left">View Details</span>
                                <span class="pull-right"><i class="fa fa-arrow-circle-right"></i></span>
                                <div class="clearfix"></div>
                            </div>
                        </a>
                    </div>
                </div>
                <div class="col-lg-fifth col-md-6">
                    <div class="panel panel-hivebrown">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3">
                                    <img src="../images/treesm.png" width="75" height="75">
                                </div>
                                <div class="col-xs-9 text-right">
                                    <div class="h4"><?PHP echo "$daygdd / $seasongdd"; ?></div>
                                    <div>GDD - Day/Season</div>
                                </div>
                            </div>
                        </div>
                        <a href="/pages/gdd.php?chart=line&period=week">
                            <div class="panel-footer">
                                <span class="pull-left">View Details</span>
                                <span class="pull-right"><i class="fa fa-arrow-circle-right"></i></span>
                                <div class="clearfix"></div>
                            </div>
                        </a>
                    </div>
                </div>
                <?php if ($enable_pollen): ?>
                <div class="col-lg-fifth col-md-6">
                    <div class="panel panel-<?php echo $pollen_card_class; ?>">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3">
                                    <img src="../images/pollen.png" width="75" height="75">
                                </div>
                                <div class="col-xs-9 text-right">
                                    <div class="h4"><?php echo "$pollenlevel / 12"; ?></div>
                                    <div>Pollen - <?php echo htmlspecialchars($pollen_label); ?></div>
                                </div>
                            </div>
                        </div>
                        <a href="/pages/pollen.php?chart=line&period=week">
                            <div class="panel-footer">
                                <span class="pull-left"><?php echo htmlspecialchars($pollentypes); ?></span>
                                <span class="pull-right"><i class="fa fa-arrow-circle-right"></i></span>
                                <div class="clearfix"></div>
                            </div>
                        </a>
                    </div>
                </div>
                <?php endif; ?>
            </div>
            <!-- /.Top Row -->
            <!-- New Row 1 -->
            <div class="row">
                <div class="col-lg-12">
        
        <!-- Button Bar -->
            <a href="/pages/index.php?period=today"><button type="button" class="btn btn btn-<?PHP if ($period == "today"){echo "primary";} else {echo "default";}?>">Today</button></a>
            <a href="/pages/index.php?period=day"><button type="button" class="btn btn btn-<?PHP if ($period == "day"){echo "primary";} else {echo "default";}?>">24 hrs</button></a>
            <a href="/pages/index.php?period=week"><button type="button" class="btn btn btn-<?PHP if ($period == "week"){echo "primary";} else {echo "default";}?>">Week</button></a>
            <a href="/pages/index.php?period=month"><button type="button" class="btn btn btn-<?PHP if ($period == "month"){echo "primary";} else {echo "default";}?>">Month</button></a>
            <a href="/pages/index.php?period=year"><button type="button" class="btn btn btn-<?PHP if ($period == "year"){echo "primary";} else {echo "default";}?>">Year</button></a>
            <a href="/pages/index.php?period=all"><button type="button" class="btn btn btn-<?PHP if ($period == "all"){echo "primary";} else {echo "default";}?>">All</button></a>

            <br>
                </div>
                <!-- /.col-lg-12 -->
                        </div>
            <div class="row">

            <?PHP
            if ($SITE_TYPE == "normal") {
                echo '<div class="col-lg-8">';
            }
            if ($SITE_TYPE == "compact") {
                echo '<div class="col-lg-12">';
            }
            ?>

                <!-- Split View: 3 Focused Charts -->
                <div id="split-view">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Hive Climate
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="climatecontainer"></div>
                        </div>
                    </div>
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Weight & Stores
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="weightcontainer"></div>
                        </div>
                    </div>
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Environment
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="envcontainer"></div>
                        </div>
                    </div>
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Foraging Conditions
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="foragingcontainer"></div>
                        </div>
                    </div>
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Air Quality
                            <span class="pull-right"><a href="/pages/air.php?chart=line&period=week" class="btn btn-xs btn-default">Detail</a></span>
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="aqcontainer"></div>
                        </div>
                    </div>
                </div>

                </div>
                <!-- /.col-lg-8 -->
            <?PHP
            if ($SITE_TYPE == "normal") {
                echo '
            
                <div class="col-lg-4">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Hive Configuration
                        </div>
                        <div class="panel-body">';
            include "datawidgets/hive_diagram.php";
                        echo '</div>
                    </div>';

            $cam_enabled = false;
            try {
                $cam_q = $conn->prepare("SELECT ENABLE_HIVE_CAMERA FROM hiveconfig WHERE id=1");
                $cam_q->execute();
                $cam_row = $cam_q->fetch(PDO::FETCH_ASSOC);
                $cam_enabled = (($cam_row['ENABLE_HIVE_CAMERA'] ?? 'no') === 'yes');
            } catch (Exception $e) {}

            if ($cam_enabled):
                $snap_path = $_SERVER['DOCUMENT_ROOT'] . '/images/hive_snapshot.jpg';
                $snap_exists = file_exists($snap_path);
                $snap_time = $snap_exists ? date("M j, g:i A", filemtime($snap_path)) : '';
            ?>
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-video-camera fa-fw"></i> Hive Camera
                            <?php if ($snap_time): ?>
                            <span class="pull-right small text-muted"><?php echo $snap_time; ?></span>
                            <?php endif; ?>
                        </div>
                        <div class="panel-body" style="padding:5px;">
                            <?php if ($snap_exists): ?>
                            <a href="/pages/video.php">
                                <img src="/images/hive_snapshot.jpg?t=<?php echo filemtime($snap_path); ?>"
                                     alt="Hive Camera" style="width:100%; border-radius:4px;" />
                            </a>
                            <p class="text-center small text-muted" style="margin:5px 0 0;">
                                <a href="/pages/video.php">View Live Stream <i class="fa fa-arrow-right"></i></a>
                            </p>
                            <?php else: ?>
                            <p class="text-muted text-center" style="margin:20px 0;">
                                <i class="fa fa-camera fa-2x"></i><br>
                                Snapshot not yet available.<br>
                                <small>Image captures on next data collection cycle.</small>
                            </p>
                            <?php endif; ?>
                        </div>
                    </div>
            <?php
            endif;

                echo '
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Estimated Stores (Honey,Pollen,Nectar)
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="weightgauge-container"></div>
                        </div>
                    </div>
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            System Status
                        </div>
                        <div class="panel-body">';
            include "status.php";
                        echo '</div>

                    </div>

                </div>
                ';} ?>

            </div>
            <!-- /.row -->
        </div>
        <!-- /#page-wrapper -->
    </div>
    <!-- /#wrapper -->

    <!-- Footer -->
    <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>

    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>

    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>

    
    <script src="/js/highcharts/highcharts.js"></script>
    <script src="/js/highcharts/highcharts-more.js"></script>
    <script src="/js/highcharts/modules/exporting.js"></script>

  <!-- Full Screen Popups -->
    <script src="/js/popup.js"></script>

    <!-- Hive Diagram -->
    <script src="/js/hive-diagram.js"></script>
    <script>
    if (window._hiveDiagramDashCfg && typeof HiveDiagram !== 'undefined') {
        HiveDiagram.renderDashboard('#dashboard-hive-svg', window._hiveDiagramDashCfg);
    }
    </script>

  <?php

  include "datawidgets/climate_chart.php";
  include "datawidgets/weight_overview_chart.php";
  include "datawidgets/environment_chart.php";
  include "datawidgets/foraging_chart.php";
  include "datawidgets/airquality_dashboard_chart.php";

  include "datawidgets/weightguage-hc.php";

  ?>
  
    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>

</html>
