

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
                    <h1 class="page-header">Pollen Analysis</h1>
            <?PHP if(isset($error)){
                echo '<div class="alert alert-danger">'; echo $error; echo'</div>';} ?>
                </div>
                <!-- /.col-lg-12 -->
                        </div>
            <div class="row">
                <div class="col-lg-12">

        <!-- Button Bar -->
            <a href="/pages/pollen.php?chart=line&period=week"><button type="button" class="btn btn btn-<?PHP if ($period == "week"){echo "primary";} else {echo "default";}?>">Week</button></a>
            <a href="/pages/pollen.php?chart=line&period=month"><button type="button" class="btn btn btn-<?PHP if ($period == "month"){echo "primary";} else {echo "default";}?>">Month</button></a>
            <a href="/pages/pollen.php?chart=line&period=year"><button type="button" class="btn btn btn-<?PHP if ($period == "year"){echo "primary";} else {echo "default";}?>">Year</button></a>
            <a href="/pages/pollen.php?chart=line&period=all"><button type="button" class="btn btn btn-<?PHP if ($period == "all"){echo "primary";} else {echo "default";}?>">All</button></a>
            <br>
                </div>
                <!-- /.col-lg-12 -->
                        </div>

            <!-- /.row -->

            <!-- /.row -->
            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Pollen Index History
                        </div>
                        <div class="panel-body">
                            <div class="pull-center" id="container"></div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">

                    <?PHP include "datawidgets/stats/pollen_stats.php"; ?>

                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Current Pollen
                        </div>
                        <div class="panel-body">
                            <div class="table-responsive">
                                <table class="table table-condensed">
                                    <tbody>
                                        <tr>
                                            <td>Level</td>
                                            <td><strong><?PHP echo "$latest_level"; ?> / 12</strong></td>
                                        </tr>
                                        <tr>
                                            <td>Severity</td>
                                            <td><?PHP echo is_numeric($latest_level) ? pollen_severity($latest_level) : 'N/A'; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Types</td>
                                            <td><?PHP echo htmlspecialchars($latest_types); ?></td>
                                        </tr>
                                        <tr>
                                            <td>Last Reading</td>
                                            <td><?PHP echo htmlspecialchars($latest_date); ?></td>
                                        </tr>
                                    </tbody>
                                </table>
                            </div>
                        </div>
                    </div>

                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Period Statistics
                        </div>
                        <div class="panel-body">
                            <div class="table-responsive">
                                <table class="table table-condensed">
                                    <tbody>
                                        <tr>
                                            <td>Average</td>
                                            <td><?PHP echo "$avgpollen"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Max / Min</td>
                                            <td><?PHP echo "$maxpollen / $minpollen"; ?></td>
                                        </tr>
                                        <tr>
                                            <td>Readings</td>
                                            <td><?PHP echo "$readings"; ?></td>
                                        </tr>
                                    </tbody>
                                </table>
                            </div>
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
    include "datawidgets/pollen_chart.php"; ?>


    <!-- Custom Theme JavaScript -->
    <!-- Footer -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>

</body>

</html>
