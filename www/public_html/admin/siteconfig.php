<?PHP

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

if ($_SERVER["REQUEST_METHOD"] == "GET") {
$sth = $conn->prepare("SELECT * FROM hiveconfig");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);


}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
// Set Error Fields
// Ensure checkbox fields exist in POST data with empty string if unchecked
$checkbox_fields = ['trend_hivetemp', 'trend_hivehum', 'trend_outtemp', 'trend_outhum', 'trend_grossweight', 'trend_netweight', 'trend_lux', 'trend_solarradiation', 'trend_rain', 'trend_gdd', 'trend_beecount_in', 'trend_beecount_out', 'trend_wind', 'trend_pressure', 'trend_pollen', 'chart_rounding', 'chart_smoothing', 'SHOW_METRIC'];
foreach ($checkbox_fields as $field) {
    if (!isset($_POST[$field])) {
        $_POST[$field] = '';
    }
}

$v = new Valitron\Validator($_POST);
$v->rule('slug', ['SITE_CONFIG', 'www_chart_theme', 'SITE_TYPE']);
$v->rule('lengthmax', ['color_hivetemp', 'color_hivehum', 'color_outtemp', 'color_outhum', 'color_grossweight', 'color_netweight', 'color_lux', 'color_solarradiation', 'color_rain', 'color_gdd', 'color_beecount_in', 'color_beecount_out', 'color_wind','color_pressure','color_pollen'], 7);
$v->rule('lengthmin', ['color_hivetemp', 'color_hivehum', 'color_outtemp', 'color_outhum', 'color_grossweight', 'color_netweight', 'color_lux', 'color_solarradiation', 'color_rain', 'color_gdd', 'color_beecount_in', 'color_beecount_out', 'color_wind','color_pressure','color_pollen'], 7);
$v->rule('lengthmax', ['trend_hivetemp', 'trend_hivehum', 'trend_outtemp', 'trend_outhum', 'trend_grossweight', 'trend_netweight', 'trend_lux', 'trend_solarradiation', 'trend_rain', 'trend_gdd', 'trend_beecount_in', 'trend_beecount_out', 'SHOW_METRIC', 'trend_wind', 'trend_pressure', 'trend_pollen'], 2);
$v->rule('in', ['trend_hivetemp', 'trend_hivehum', 'trend_outtemp', 'trend_outhum', 'trend_grossweight', 'trend_netweight', 'trend_lux', 'trend_solarradiation', 'trend_rain', 'trend_gdd', 'chart_rounding', 'chart_smoothing', 'trend_beecount_in', 'trend_beecount_out', 'SHOW_METRIC', 'trend_wind', 'trend_pressure', 'trend_pollen'], ['on', '']);

}
//Check input for badness
function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}

function test_input_allow_slash($data) {
  $data = trim($data);
  $data = htmlspecialchars($data);
  return $data;
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
                    <h1 class="page-header">Settings - Site Config</h1>
                </div>
                <!-- /.col-lg-12 -->
            </div>
            <!-- /.row -->
        <?PHP
        if ($_SERVER["REQUEST_METHOD"] == "POST") {
        
if($v->validate()) {


    
// Make sure no bad stuff got through our filters
// Probably not needed, but doesn't hurt

    $SITE_ORIENT = test_input($_POST["SITE_ORIENT"]);
    $SITE_TYPE = test_input($_POST["SITE_TYPE"]);
    $www_chart_theme = test_input($_POST["www_chart_theme"]);
    $color_hivetemp = test_input($_POST["color_hivetemp"]);
    $color_hivehum = test_input($_POST["color_hivehum"]);
    $color_outtemp = test_input($_POST["color_outtemp"]);
    $color_outhum = test_input($_POST["color_outhum"]);
    $color_grossweight = test_input($_POST["color_grossweight"]);
    $color_netweight = test_input($_POST["color_netweight"]);
    $color_lux = test_input($_POST["color_lux"]);
    $color_solarradiation = test_input($_POST["color_solarradiation"]);
    $color_rain = test_input($_POST["color_rain"]);
    $color_gdd = test_input($_POST["color_gdd"]);
    $color_beecount_in = test_input($_POST["color_beecount_in"]);
    $color_beecount_out = test_input($_POST["color_beecount_out"]);
    
    $color_wind = test_input($_POST["color_wind"]);
    $color_pressure = test_input($_POST["color_pressure"]);
    $color_pollen = test_input($_POST["color_pollen"]);

    $trend_hivetemp = test_input($_POST["trend_hivetemp"]);
    $trend_hivehum = test_input($_POST["trend_hivehum"]);
    $trend_outtemp = test_input($_POST["trend_outtemp"]);
    $trend_outhum = test_input($_POST["trend_outhum"]);
    $trend_grossweight = test_input($_POST["trend_grossweight"]);
    $trend_netweight = test_input($_POST["trend_netweight"]);
    $trend_lux = test_input($_POST["trend_lux"]);
    $trend_solarradiation = test_input($_POST["trend_solarradiation"]);
    $trend_rain = test_input($_POST["trend_rain"]);
    $trend_gdd = test_input($_POST["trend_gdd"]);
    $trend_beecount_in = test_input($_POST["trend_beecount_in"]);
    $trend_beecount_out = test_input($_POST["trend_beecount_out"]);
    $trend_wind = test_input($_POST["trend_wind"]);
    $trend_pressure = test_input($_POST["trend_pressure"]);
    $trend_pollen = test_input($_POST["trend_pollen"]);

    $chart_rounding = test_input($_POST["chart_rounding"]);
    $chart_smoothing = test_input($_POST["chart_smoothing"]);

    $SHOW_METRIC = test_input($_POST["SHOW_METRIC"]);



  // Get current version    
  //  $ver = $conn->prepare("SELECT version FROM hiveconfig");
  //  $ver->execute();
  //  $ver = $ver->fetchColumn();
// Increment version
   // $version = ++$ver;

    // Update into the DB
    $doit = $conn->prepare("UPDATE hiveconfig SET SITE_ORIENT=?,SITE_TYPE=?,www_chart_theme=?,color_hivetemp=?,trend_hivetemp=?,color_hivehum=?,color_outtemp=?,color_outhum=?,color_grossweight=?,color_netweight=?,color_lux=?,color_solarradiation=?,color_rain=?,color_gdd=?,trend_hivehum=?,trend_outtemp=?,trend_outhum=?,trend_grossweight=?,trend_netweight=?,trend_lux=?,trend_solarradiation=?,trend_rain=?,trend_gdd=?,chart_rounding=?,chart_smoothing=?,color_beecount_in=?,color_beecount_out=?,trend_beecount_in=?,trend_beecount_out=?,SHOW_METRIC=?,color_wind=?,color_pressure=?,color_pollen=?,trend_wind=?,trend_pressure=?,trend_pollen=? WHERE id=1");
    $doit->execute(array($SITE_ORIENT,$SITE_TYPE,$www_chart_theme,$color_hivetemp,$trend_hivetemp,$color_hivehum,$color_outtemp,$color_outhum,$color_grossweight,$color_netweight,$color_lux,$color_solarradiation,$color_rain,$color_gdd,$trend_hivehum,$trend_outtemp,$trend_outhum,$trend_grossweight,$trend_netweight,$trend_lux,$trend_solarradiation,$trend_rain,$trend_gdd,$chart_rounding,$chart_smoothing,$color_beecount_in,$color_beecount_out,$trend_beecount_in,$trend_beecount_out,$SHOW_METRIC,$color_wind,$color_pressure,$color_pollen,$trend_wind,$trend_pressure,$trend_pollen));
    sleep(1);



    // Refresh the fields in the form
    $sth = $conn->prepare("SELECT * FROM hiveconfig");
    $sth->execute();
    $result = $sth->fetch(PDO::FETCH_ASSOC);
    
    // Tell user it saved
    #echo '<div class="alert alert-success alert-dismissable">
     #                           <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>';
    #echo 'Successfully Saved';
    #echo '</div>';
} else {
    // Errors
     echo '<div class="alert alert-danger alert-dismissable"> <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>';
      $err=array_values($v->errors());
      for ($i=0; $i < count($err); $i++){
        echo $err[$i][0]."<br>";
      }
          echo  '</div>';
    // Refresh the fields in the form
    $sth = $conn->prepare("SELECT * FROM hiveconfig");
    $sth->execute();
    $result = $sth->fetch(PDO::FETCH_ASSOC);  
}
               
    }
        ?>
            <!-- /.row -->
           <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <!-- /.panel-heading -->
                        <form method="POST" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]);?>">
                        <div class="panel-body">
                            <div class="dataTable_wrapper">
                                <table class="table table-striped table-bordered table-hover" id="dataTables-example">
                                    <thead> 
                                            <th>Setting</th>
                                            <th>Value</th>
                                            <th>Description</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                    <tr class="odd gradeX">
                                    <td style="width:200px">Check for Upgrades</td>
                                    <td style="width:300px"><?PHP echo $result['HCVersion']; ?></td>
                                    <td>The current version installed. For updates/upgrades go to the <a href="/admin/system.php">System Mgmt</a> page.</td>
                                    </tr>

                                       <tr class="odd gradeX">
                                        <td style="width:200px">Chart Theme</td>
                                        <td style="width:300px"><select onchange="this.form.submit()" name="www_chart_theme">
                                        <option value="" <?php if ($result['www_chart_theme'] == "") {echo "selected='selected'";} ?>>Default</option>
                                        <option value="dark-blue" <?php if ($result['www_chart_theme'] == "dark-blue") {echo "selected='selected'";} ?>>Dark Blue</option>
                                        <option value="dark-unica" <?php if ($result['www_chart_theme'] == "dark-unica") {echo "selected='selected'";} ?>>Dark Unica</option>
                                        <option value="gray" <?php if ($result['www_chart_theme'] == "gray") {echo "selected='selected'";} ?>>Gray</option>
                                        <option value="grid-light" <?php if ($result['www_chart_theme'] == "grid-light") {echo "selected='selected'";} ?>>Grid Light</option>
                                        <option value="grid" <?php if ($result['www_chart_theme'] == "grid") {echo "selected='selected'";} ?>>Grid</option>
                                        <option value="sand-signika" <?php if ($result['www_chart_theme'] == "sand-signika") {echo "selected='selected'";} ?>>Sand Signika</option> 
                                        <option value="skies" <?php if ($result['www_chart_theme'] == "skies") {echo "selected='selected'";} ?>>Skies</option> 
                                        </select>
                                        </td>
                                        <td>Configure your preferred theme for the highcharts used throughout the site. </td>
                                        </tr>
                                    <tr class="odd gradeX">
                                        <td>Chart - Round</td>
                                        <td><input type="checkbox" onchange="this.form.submit()" name="chart_rounding" value="on" <?php if ($result['chart_rounding'] == "on") {echo "checked='checked'";} ?> > </td>
                                        </td>
                                        <td>Controls if the main chart (on the dashboard) uses rounded numbers. Often our sensors can vary quite a bit within the .00, but it doesn't really show true change as it's just sensor noise. This will round it on the main chart. The detail charts will still show the exact values </td>
                                    </tr>
                                    <tr class="odd gradeX">
                                        <td>Chart - Smoothing</td>
                    
                                        <td><input type="checkbox" onchange="this.form.submit()" name="chart_smoothing" value="on" <?php if ($result['chart_smoothing'] == "on") {echo "checked='checked'";} ?> > </td>
                                        
                                        </td>
                                        <td>Sometimes our sensors error out past our routines, which causes zero values. This can cause our charts to looked skewed. This option removes any record that has a zero value - This option only applies to the main chart. </td>
                                    </tr>

                                     <tr class="odd gradeX">
                                        <td>Measurement Display</td>
                                        <td><select onchange="this.form.submit()" name="SHOW_METRIC">
                                        <option value="" <?php if ($result['SHOW_METRIC'] == "") {echo "selected='selected'";} ?>>Imperial</option>
                                        <option value="on" <?php if ($result['SHOW_METRIC'] == "on") {echo "selected='selected'";} ?>>Metric</option>
                                        </select>
                                        </td>
                                        <td>Configure your preferred measurement types. Useful for our international folks. </td>
                                       </tr>
                                        
                                       </tr>
                                    <tr class="odd gradeX">
                                        <td>Site Layout</td>
                                        <td><select onchange="this.form.submit()" name="SITE_ORIENT">
                                        <option value="normal" <?php if ($result['SITE_ORIENT'] == "") {echo "selected='selected'";} ?>>Default</option>
                                        <option value="wide" <?php if ($result['SITE_ORIENT'] == "wide") {echo "selected='selected'";} ?>>Wide</option>
                                        </select>
                                        </td>
                                        <td>Configure your preferred site layout. </td>
                                       </tr>
                                    <tr class="odd gradeX">
                                        <td>Site Type</td>
                                        <td><select onchange="this.form.submit()" name="SITE_TYPE">
                                        <option value="normal" <?php if ($result['SITE_TYPE'] == "") {echo "selected='selected'";} ?>>Default</option>
                                        <option value="compact" <?php if ($result['SITE_TYPE'] == "compact") {echo "selected='selected'";} ?>>Compact</option>
                                        </select>
                                        </td>
                                        <td>Configure your preferred site detail type. Compact removes the stats bars from the dashboard. </td>
                                       </tr>

                                       <tr class="odd gradeX">
                                        <td>Trend Lines</td>
                                        <td>
                                        <table class="table table-striped table-bordered table-hover" id="dataTables-example"><thead>
                                        <tr>
                                            <th></th>
                                            <th>On/Off</th>
                                            <th>Color</th>
                                        </tr>
                                    </thead>
                                    
                                    <tbody>
                                    <tr class="odd gradeX">
                                            <td>Hive Temp </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_hivetemp" value="on" <?php if ($result['trend_hivetemp'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_hivetemp' value="<?php echo $result['color_hivetemp']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Hive Humidity </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_hivehum" value="on" <?php if ($result['trend_hivehum'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_hivehum' value="<?php echo $result['color_hivehum']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Outside Temp </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_outtemp" value="on" <?php if ($result['trend_outtemp'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_outtemp' value="<?php echo $result['color_outtemp']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Outside Humidity </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_outhum" value="on" <?php if ($result['trend_outhum'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_outhum' value="<?php echo $result['color_outhum']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Gross Weight </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_grossweight" value="on" <?php if ($result['trend_grossweight'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_grossweight' value="<?php echo $result['color_grossweight']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Net Weight </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_netweight" value="on" <?php if ($result['trend_netweight'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_netweight' value="<?php echo $result['color_netweight']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Lux </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_lux" value="on" <?php if ($result['trend_lux'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_lux' value="<?php echo $result['color_lux']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Solar Radiation </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_solarradiation" value="on" <?php if ($result['trend_solarradiation'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_solarradiation' value="<?php echo $result['color_solarradiation']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Rain </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_rain" value="on" <?php if ($result['trend_rain'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_rain' value="<?php echo $result['color_rain']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Growing Degree Days </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_gdd" value="on" <?php if ($result['trend_gdd'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_gdd' value="<?php echo $result['color_gdd']; ?>" /> </td>
                                        </td>
                                        </tr>

                                        <tr class="odd gradeX">
                                            <td>Hive Activity In </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_beecount_in" value="on" <?php if ($result['trend_beecount_in'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_beecount_in' value="<?php echo $result['color_beecount_in']; ?>" /> </td>
                                        </td>
                                        </tr>

                                        <tr class="odd gradeX">
                                            <td>Hive Activity Out </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_beecount_out" value="on" <?php if ($result['trend_beecount_out'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_beecount_out' value="<?php echo $result['color_beecount_out']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Wind </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_wind" value="on" <?php if ($result['trend_wind'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_wind' value="<?php echo $result['color_wind']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Pressure </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_pressure" value="on" <?php if ($result['trend_pressure'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_pressure' value="<?php echo $result['color_pressure']; ?>" /> </td>
                                        </td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Pollen </td> 
                                            <td><input type="checkbox" onchange="this.form.submit()" name="trend_pollen" value="on" <?php if ($result['trend_pollen'] == "on") {echo "checked='checked'";} ?> > </td>
                                            <td><input type='color' onchange="this.form.submit()" name='color_pollen' value="<?php echo $result['color_pollen']; ?>" /> </td>
                                        </td>
                                        </tr>
                                    </tbody>
                                </table>
                                        <td>Configure your preferred trend colors and weather to show them on the main dashboard. </td>
                                       </tr>

                                     </tr>

                                    </tbody>
                                </table>


                            </div>
                            <!-- /.table-responsive -->
                        </div>
                        <!-- /.panel-body -->
                    </div>
                    <!-- /.panel -->
                </div>
                <!-- /.col-lg-12 -->
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

<script>
$(document).ready(function(){
    $('[data-toggle="popover"]').popover(); 
});
</script>

    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>

</html>
