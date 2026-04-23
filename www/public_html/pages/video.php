
<!DOCTYPE html>
<html lang="en">

    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

    <div id="wrapper">

            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Hive Video Stream</h1>
            <?PHP if(isset($error)){
                echo '<div class="alert alert-danger">'; echo $error; echo'</div>';} ?>
                </div>
            </div>

            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-body">
                            This supports native MJPEG streaming in the browser and direct connection with a tool like VLC.
                            To connect with VLC, use URL: http://<?PHP echo $_SERVER['HTTP_HOST'];?>:8080/?action=stream
                        </div>
                    </div>
                </div>
            </div>

            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            Live Stream
                        </div>
                        <div class="panel-body">
                            <img id="livestream" src="/pages/videostream.php" style="width:100%; max-width:640px;" />
                            <div id="stream-error" class="alert alert-warning" style="display:none;">
                                Stream not available. Make sure the livestream service is running:<br>
                                <code>sudo /etc/init.d/livestream start</code>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="col-lg-4">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <b><?PHP echo "$location"; ?></b>
                        </div>
                        <div class="table-responsive">
                            <table class="table">
                                <tbody>
                                    <tr>
                                        <td colspan="2" align="center" valign="top"><h2><?PHP echo "$temp"; if ($SHOW_METRIC == "on") { echo " °C"; $i = "C"; } else {echo " °F"; $i = "F";}?></h2><br>
                                        </td>
                                        <td valign="middle" align="center"><img class="img-responsive" src="<?PHP echo "$current_icon_url"; ?>">
                                            <?PHP echo "$current_weather"; ?>
                                        </td>
                                        <td align="center" valign="middle" colspan="2">
                                            <font color="red"><?PHP echo "$today_high"; ?>°</font> | <font color="blue"><?PHP echo "$today_low"; ?>°</font><br><br>
                                            <img align="bottom" src="/images/drop_blue.png"><?PHP echo "$today_pop"; ?>%
                                        </td>
                                    </tr>
                                </tbody>
                            </table>
                        </div>
                    </div>
                </div>
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

    <script>
    document.getElementById('livestream').onerror = function() {
        this.style.display = 'none';
        document.getElementById('stream-error').style.display = 'block';
    };
    </script>

    <!-- Custom Theme JavaScript -->
<!-- Footer -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>
</body>

</html>
