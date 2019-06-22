

<?php 

#if ($SHOW_METRIC == "on") { echo " C"; $i = "C"; } else {echo "F"; $i = "F";}""

//Get our location settings from the DB
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

$sth2 = $conn->prepare("SELECT * FROM hiveconfig");
$sth2->execute();
$result = $sth2->fetch(PDO::FETCH_ASSOC);

$key = $result['KEY'];
$city = $result['CITY'];
$state = $result['STATE'];
$wxstation = $result['WXSTATION'];
$weather_detail = $result['WEATHER_DETAIL'];
$SHOW_METRIC = $result['SHOW_METRIC'];

if ($weather_detail == "city") {
  $wxlocation = "$state/$city";
  

} else {
  $wxlocation = "pws:$wxstation";

}


// Get Current Conditions from a specific Weather Station
  $json_string_current = file_get_contents("http://api.wunderground.com/api/$key/conditions/q/$wxlocation.json");
  $parsed_json_current = json_decode($json_string_current);

  $location= $parsed_json_current->{'current_observation'}->{'display_location'}->{'full'};  
  
  if ($SHOW_METRIC == "on") {
    $temp = $parsed_json_current->{'current_observation'}->{'temp_c'};
    $feelslike = $parsed_json_current->{'current_observation'}->{'feelslike_c'};
  } else {
  $temp = $parsed_json_current->{'current_observation'}->{'temp_f'};
  $feelslike = $parsed_json_current->{'current_observation'}->{'feelslike_f'};
  }
  
  $current_icon_url = $parsed_json_current->{'current_observation'}->{'icon_url'};
  $current_weather = $parsed_json_current->{'current_observation'}->{'weather'};

// Get Hourly Forecast - 
  $json_string_hourly = file_get_contents("http://api.wunderground.com/api/$key/hourly/q/$wxlocation.json");
  $parsed_json_hourly = json_decode($json_string_hourly);
 

// Get 10day forecast - used below 
  $json_string = file_get_contents("http://api.wunderground.com/api/$key/forecast10day/q/$wxlocation.json");
  $parsed_json = json_decode($json_string);

if ($SHOW_METRIC == "on") {
$today_high = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[0]->{'high'}->celsius;
$today_low = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[0]->{'low'}->celsius;

} else {
$today_high = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[0]->{'high'}->fahrenheit;
$today_low = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[0]->{'low'}->fahrenheit;

}
$today_pop = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[0]->{'pop'};

?>



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
                <!-- /.col-lg-12 -->
                        </div>
            
            <!-- /.row -->

            <!-- /.row -->
            <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-body">
                            This supports native streaming via the "stream" protocol, streaming via javascript and a direct connection with a tool like VLC.
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
                            <img src="/pages/videostream.php" />
                            <P>
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
                                            <td colspan="2" align="center" valign="top"><h2><?PHP echo "$temp"; if ($SHOW_METRIC == "on") { echo " °C"; $i = "C"; } else {echo " °F"; $i = "F";}?></H2><br>
                                              </td>
                                            <td valign="middle" align="center"><img class="img-responsive" src="<?PHP echo "$current_icon_url"; ?>">
                                              <?PHP echo "$current_weather"; ?>
                                            </td>

                                            <td align="center" valign="middle" colspan="2">
                                            <font color="red"><?PHP echo "$today_high"; ?>°</font> | <font color="blue"><?PHP echo "$today_low"; ?>°</font><BR><br>
                                            <img align="bottom" src="/images/drop_blue.png"><?PHP echo "$today_pop"; ?>%
                                            </td>
                                        </tr>
                                        <tr>
                                      <?php
 

  if ($SHOW_METRIC == "on") {
    # Set variable to metric
    $unit = "metric";
  }
  else {
    #set to english
    $unit = "english";
  }

  $y = 0;
  while ($y < 4) {
  $hour = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'FCTTIME'}->hour;
  if ($hour > 12 ) {
      $hour = ($hour - 12); }
  elseif ($hour == 0) {
    $hour = 12;
  }

  $ampm = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'FCTTIME'}->ampm;
  
  $hour_temp = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'temp'}->$unit;
  //$low = $parsed_json_hourly->{'forecast'}->{'simpleforecast'}->{'forecastday'}[$y]->{'low'}->fahrenheit;
  $hour_icon_url = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'icon_url'};
  $hour_condition = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'condition'};
  $hour_pop = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'pop'};
  echo "<td align=\"center\"><center>";
  echo "<b><font size=\"2\"> ${hour} ${ampm}</B> <br>";
  echo "${hour_temp}°<BR> </font>";
  echo "<img class=\"img-responsive\" width=\"25\" height=\"25\" src=\"${hour_icon_url}\"> <BR>";
  echo "<font size=\"1\">${hour_condition}</font> <br>";
  echo "<font size=\"1\" color=\"blue\">${hour_pop}% </font><br></center>";
  
  echo "</td>";
  $y++;
}

?>
                                        </tr>
                                    </tbody>
                                </table>
                            </div>
              </div>

              </div>



            </div>
           <!-- <div class="row">
                <div class="col-lg-12">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            
                        </div>
                        <div class="panel-body">
                          <?PHP  include($_SERVER["DOCUMENT_ROOT"] . "/pages/video_js.html"); ?>
                        </div>
                    </div>
                </div>
            </div> -->
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

   
 
    
    <!-- Custom Theme JavaScript -->
<!-- Footer -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>
</body>

</html>
