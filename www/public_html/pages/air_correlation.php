

<?PHP

function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}

$period = test_input($_GET["period"]);

if(isset($_GET["period"])) {
    if (empty($_GET["period"])) {
        $error = "ERROR: Period can't be empty";
    } else {
        $period = test_input($_GET["period"]);
    }
} else {
    $error = "ERROR: Must specify a period";
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
                    <h1 class="page-header">Air Quality &mdash; Bee Impact Correlations</h1>
            <?PHP if(isset($error)){
                echo '<div class="alert alert-danger">'; echo $error; echo'</div>';} ?>
                </div>
            </div>
            <div class="row">
                <div class="col-lg-12">

        <!-- Button Bar -->
            <a href="/pages/air_correlation.php?period=today"><button type="button" class="btn btn btn-<?PHP if ($period == "today"){echo "primary";} else {echo "default";}?>">Today</button></a>
            <a href="/pages/air_correlation.php?period=day"><button type="button" class="btn btn btn-<?PHP if ($period == "day"){echo "primary";} else {echo "default";}?>">24 hr</button></a>
            <a href="/pages/air_correlation.php?period=week"><button type="button" class="btn btn btn-<?PHP if ($period == "week"){echo "primary";} else {echo "default";}?>">Week</button></a>
            <a href="/pages/air_correlation.php?period=month"><button type="button" class="btn btn btn-<?PHP if ($period == "month"){echo "primary";} else {echo "default";}?>">Month</button></a>
            <a href="/pages/air_correlation.php?period=year"><button type="button" class="btn btn btn-<?PHP if ($period == "year"){echo "primary";} else {echo "default";}?>">Year</button></a>
            <a href="/pages/air_correlation.php?period=all"><button type="button" class="btn btn btn-<?PHP if ($period == "all"){echo "primary";} else {echo "default";}?>">All</button></a>

            <span style="margin-left: 20px;">
                <a href="/pages/air.php?chart=line&period=<?PHP echo htmlspecialchars($period, ENT_QUOTES); ?>"><button type="button" class="btn btn-sm btn-info"><i class="fa fa-line-chart"></i> Back to Air Quality</button></a>
            </span>
            <br>
                </div>
                        </div>

            <!-- Charts -->
            <div class="row">
                <div class="col-lg-12">
                    <?PHP include "datawidgets/air_correlation_chart.php"; ?>
                </div>
            </div>

            <!-- EPA O3/NO2 Detail -->
            <div class="row">
                <div class="col-lg-12">
                    <?PHP include "datawidgets/epa_air_chart.php"; ?>
                </div>
            </div>

                        </div>

    <!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>

    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>

    <script src="../dist/js/sb-admin-2.js"></script>

    <!-- Custom Theme JavaScript -->
<!-- Footer -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>
</body>

</html>
