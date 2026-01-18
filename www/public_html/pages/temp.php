

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

    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

    <div id="wrapper">


     
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
            <a href="/pages/temp.php?chart=line&period=today"><button type="button" class="btn btn btn-<?PHP if ($period == "today"){echo "primary";} else {echo "default";}?>">Today</button></a>
            <a href="/pages/temp.php?chart=line&period=day"><button type="button" class="btn btn btn-<?PHP if ($period == "day"){echo "primary";} else {echo "default";}?>">24 hr</button></a>
            <a href="/pages/temp.php?chart=line&period=week"><button type="button" class="btn btn btn-<?PHP if ($period == "week"){echo "primary";} else {echo "default";}?>">Week</button></a>
            <a href="/pages/temp.php?chart=line&period=month"><button type="button" class="btn btn btn-<?PHP if ($period == "month"){echo "primary";} else {echo "default";}?>">Month</button></a>
            <a href="/pages/temp.php?chart=line&period=year"><button type="button" class="btn btn btn-<?PHP if ($period == "year"){echo "primary";} else {echo "default";}?>">Year</button></a>
            <a href="/pages/temp.php?chart=line&period=all"><button type="button" class="btn btn btn-<?PHP if ($period == "all"){echo "primary";} else {echo "default";}?>">All</button></a>
            <br>
                </div>
                <!-- /.col-lg-12 -->
                        </div>
            
            <!-- /.row -->

            <!-- /.row -->
             <div class="row">
                <div class="col-lg-9">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Temperature / Humidity Analysis 
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="tempcontainer"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-3">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Stats - ( Hive / Ambient )
                        </div>
                                                <div class="panel-body">
                            <div class="table-responsive">
                                <table class="table">
                                    <?PHP include "datawidgets/temp_stats.php"; 
                                    ?>

                                    <tbody>
                                        <tr>
                                            <td>Avg Temp (<?PHP if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} ?>)</td>
                                            <td><?PHP echo "$avghivetempf  / $avgweather_tempf"; ?></td>
                                         
                                            
                                        </tr>
                                        <tr>
                                            <td>Avg Humidity (%)</td>
                                            <td><?PHP echo "$avghivehum  / $avgweather_humidity"; ?></td>
                                           
                                        </tr>
                                        <tr>
                                            <td>High (<?PHP if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} ?>)</td>
                                            <td><?PHP echo "$maxhivetempf  / $maxweathertempf "; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Low (<?PHP if ( $SHOW_METRIC == "on" ) { echo "°C";} else {echo "°F";} ?>)</td>
                                            <td><?PHP echo "$minhivetempf  / $minweathertempf "; ?></td>
                                        </tr>
                                    </tbody>
                                </table>
                            </div>
                            <!-- /.table-responsive -->
                        </div>
                    </div>

                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Download Raw Data
                        </div> 
                           <div class="panel-body">
                            <div class="table-responsive">
                                <table class="table">
                                    <?PHP include "datawidgets/temp_stats.php"; 
                                    ?>

                                    <tbody>
                                        <tr>
                                            <td><a href="/pages/datawidgets/datajob.php?type=temp_data&period=<?PHP echo $period;?>&output=csv">CSV</a></td>
                                        </tr>
                                        <tr>
                                            <td><a href="/pages/datawidgets/datajob.php?type=temp_data&period=<?PHP echo $period;?>&output=json">JSON</a></td>
                                            <td></td>
                                           
                                        </tr>
                                    </tbody>
                                </table>
                            </div>
                            <!-- /.table-responsive -->
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
    <script src="https://code.highcharts.com/stock/highstock.js"></script>
    <script src="https://code.highcharts.com/stock/modules/exporting.js"></script>    

    
    <!-- Full Screen Popups -->
    <script src="/js/popup.js"></script>       

    


    <?php 
        #Since this document already has variables, we can pass variables by just including it.
        # Period and chart variables will be used in the charts.
    
    include "datawidgets/temp_chart.php"; ?>
    
    
    <!-- Custom Theme JavaScript -->
<!-- Footer -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>
</body>

</html>
