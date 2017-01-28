
<?PHP

//Pass variables to determine the type of view

// Functions
// period - specify what timeperiods you want to see

//Set Variables to null to start
$period="";
$chart_rounding="";

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
        
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Dashboard - <?PHP echo "$recorddate"; ?></h1>
                </div>
                <!-- /.col-lg-12 -->
            </div>
            <!-- /.row -->
            <!-- Top Row -->
            <div class="row">
                <div class="col-lg-3 col-md-6">
                    <div class="panel panel-green">
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
                                    <div>Temp - Hive / Ambient</div>
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
                <div class="col-lg-3 col-md-6">
                    <div class="panel panel-hiveyellow">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3">
                                     <img src="/images/scalesm.png">
                                </div>
                                <div class="col-xs-9 text-right">
            
                                    <div class="h4"><?php echo "$hiveweight "; if ($SHOW_METRIC == "on") { echo " kg"; $i = "kg"; } else {echo "lb"; $i = "lb";} echo " / "."$changeweight"." $i"; ?> </div>
                                    <div>Weight / Trend </div>
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
                <div class="col-lg-3 col-md-6">
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
                                    <div>Humidity - Hive / Ambient</div>
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
                <div class="col-lg-3 col-md-6">
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
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Hive Charts 
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="allcontainer"></div>
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

    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>

    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>

    
    <script src="/js/highcharts/highcharts.js"></script>
    <script src="/js/highcharts/modules/exporting.js"></script>

  <!-- Full Screen Popups -->
    <script src="/js/popup.js"></script>    

  <?php 

  
  include "datawidgets/all_chart.php"; 

  include "datawidgets/weightguage-hc.php"; 

  ?> 
  

    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>

</html>
