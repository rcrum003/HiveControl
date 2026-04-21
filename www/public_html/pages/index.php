
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



    <div id="wrapper">


<?php include "datawidgets/currentconditions.php"; ?>

<?php include "datawidgets/alert_banner.php"; ?>

            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Dashboard - <?PHP echo "$recorddate"; ?></h1>
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
                        <a href="/pages/gdd.php?chart=line&period=week">
                            <div class="panel-footer">
                                <span class="pull-left"><?php echo htmlspecialchars($pollentypes); ?></span>
                                <span class="pull-right"><i class="fa fa-arrow-circle-right"></i></span>
                                <div class="clearfix"></div>
                            </div>
                        </a>
                    </div>
                </div>
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

            <span style="margin-left: 20px;">
                <button type="button" class="btn btn-sm btn-info" id="btn-split-view" onclick="toggleChartView('split')">Split View</button>
                <button type="button" class="btn btn-sm btn-default" id="btn-combined-view" onclick="toggleChartView('combined')">Combined View</button>
            </span>
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
                            Environment & Foraging
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="envcontainer"></div>
                        </div>
                    </div>
                </div>

                <!-- Combined View: Original All-in-One Chart -->
                <div id="combined-view" style="display:none;">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Hive Charts
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="allcontainer"></div>
                        </div>
                    </div>
                </div>

                </div>
                <!-- /.col-lg-6 -->
            <?PHP 
            if ($SITE_TYPE == "normal") {
                echo '
            
                <div class="col-lg-4">
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
                    <!-- /.panel -->
                </div>
                <!-- /.col-lg-6 -->
            </div>
            <!-- /.row -->                
                        <!-- /.panel-body -->
                    </div>
                    <!-- /.panel -->
                </div>
                <!-- /.col-lg-6 -->

                <!-- /.col-lg-6 -->
            </div>
            <!-- /.row -->    
                        </div>
                        </div>
                        <!-- /.panel-footer -->
                    </div>
                    <!-- /.panel .chat-panel -->
                </div>
                <!-- /.col-lg-4 -->
            </div>
            <!-- /.Row1 -->
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

  <?php

  include "datawidgets/climate_chart.php";
  include "datawidgets/weight_overview_chart.php";
  include "datawidgets/environment_chart.php";
  include "datawidgets/all_chart.php";

  include "datawidgets/weightguage-hc.php";

  ?>

  <script>
  function toggleChartView(view) {
      if (view === 'split') {
          document.getElementById('split-view').style.display = 'block';
          document.getElementById('combined-view').style.display = 'none';
          document.getElementById('btn-split-view').className = 'btn btn-sm btn-info';
          document.getElementById('btn-combined-view').className = 'btn btn-sm btn-default';
          localStorage.setItem('hc_chart_view', 'split');
      } else {
          document.getElementById('split-view').style.display = 'none';
          document.getElementById('combined-view').style.display = 'block';
          document.getElementById('btn-split-view').className = 'btn btn-sm btn-default';
          document.getElementById('btn-combined-view').className = 'btn btn-sm btn-info';
          localStorage.setItem('hc_chart_view', 'combined');
      }
  }
  (function() {
      var saved = localStorage.getItem('hc_chart_view');
      if (saved === 'combined') {
          toggleChartView('combined');
      }
  })();
  </script>
  
    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>

</html>
