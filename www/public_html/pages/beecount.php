

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
                    <h1 class="page-header">Hive Activity Analysis</h1>
            <?PHP if(isset($error)){ 
                echo '<div class="alert alert-danger">'; echo $error; echo'</div>';} ?>
                </div>
                <!-- /.col-lg-12 -->
                        </div>
            <div class="row">
                <div class="col-lg-12">
        
        <!-- Button Bar -->
            <a href="/pages/beecount.php?chart=line&period=day"><button type="button" class="btn btn btn-<?PHP if ($period == "day"){echo "primary";} else {echo "default";}?>">Day</button></a>
            <a href="/pages/beecount.php?chart=line&period=week"><button type="button" class="btn btn btn-<?PHP if ($period == "week"){echo "primary";} else {echo "default";}?>">Week</button></a>
            <a href="/pages/beecount.php?chart=line&period=month"><button type="button" class="btn btn btn-<?PHP if ($period == "month"){echo "primary";} else {echo "default";}?>">Month</button></a>
            <a href="/pages/beecount.php?chart=line&period=year"><button type="button" class="btn btn btn-<?PHP if ($period == "year"){echo "primary";} else {echo "default";}?>">Year</button></a>
            <a href="/pages/beecount.php?chart=line&period=all"><button type="button" class="btn btn btn-<?PHP if ($period == "all"){echo "primary";} else {echo "default";}?>">All</button></a>
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
                            Hive Activity Analysis 
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="container"></div>
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
                                    <?PHP include "datawidgets/beecount_stats.php"; 
                                    ?>

                                    <tbody>
                                        <tr>
                                            <td>Total Bees In </td>
                                            <td><?PHP echo "$total_in"; ?></td>
                                         
                                            
                                        </tr>
                                        <tr>
                                            <td>Total Bees Out </td>
                                            <td><?PHP echo "$total_out"; ?></td>
                                           
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
                                    <?PHP include "datawidgets/beecount_stats.php"; 
                                    ?>

                                    <tbody>
                                        <tr>
                                            <td><a href="/pages/datawidgets/datajob.php?type=beecount_data&period=<?PHP echo $period;?>&output=csv">CSV</a></td>
                                        </tr>
                                        <tr>
                                            <td><a href="/pages/datawidgets/datajob.php?type=beecount_data&period=<?PHP echo $period;?>&output=json">JSON</a></td>
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
    <script src="/js/highcharts/modules/exporting.js"></script>
    
    <!-- Full Screen Popups -->
    <script src="/js/popup.js"></script>       

    


    <?php 
        #Since this document already has variables, we can pass variables by just including it.
        # Period and chart variables will be used in the charts.
    
    include "datawidgets/beecount_chart.php"; ?>
    
    
    <!-- Custom Theme JavaScript -->

</body>

</html>
