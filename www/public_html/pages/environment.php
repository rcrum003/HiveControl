
<!DOCTYPE html>
<html lang="en">

<head>

    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">

    <title>Hive Control</title>

    <!-- Bootstrap Core CSS -->
    <link href="../bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">

    <!-- MetisMenu CSS -->
    <link href="../bower_components/metisMenu/dist/metisMenu.min.css" rel="stylesheet">

    <!-- Timeline CSS -->
    <link href="../dist/css/timeline.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="../dist/css/sb-admin-2.css" rel="stylesheet">


    <!-- Custom Fonts -->
    <link href="../bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->


</head>

<body> 
 
    <div id="wrapper">

        <!-- Navigation -->
       <?PHP include "../include/navigation.php" ?>
        <!-- /Navigation -->

        <div id="page-wrapper">
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Weather Forecast <?php echo $id; ?></h1>
                </div>
                <!-- /.col-lg-12 -->
            </div>

 
            <!-- /.row -->
            <div class="row">
              <div class="col-lg-4">
                <div class="panel panel-default">
              

<?php 

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


if ($weather_detail == "city") {
  $wxlocation = "$state/$city";
  

} else {
  $wxlocation = "pws:$wxstation";

}


// Get Current Conditions from a specific Weather Station
  $json_string_current = file_get_contents("http://api.wunderground.com/api/$key/conditions/q/$wxlocation.json");
  $parsed_json_current = json_decode($json_string_current);

  $location= $parsed_json_current->{'current_observation'}->{'display_location'}->{'full'};  
  $tempf = $parsed_json_current->{'current_observation'}->{'temp_f'};
  $feelslikef = $parsed_json_current->{'current_observation'}->{'feelslike_f'};
  $current_icon_url = $parsed_json_current->{'current_observation'}->{'icon_url'};
  $current_weather = $parsed_json_current->{'current_observation'}->{'weather'};

// Get Hourly Forecast - 
  $json_string_hourly = file_get_contents("http://api.wunderground.com/api/$key/hourly/q/$wxlocation.json");
  $parsed_json_hourly = json_decode($json_string_hourly);
 

// Get 10day forecast - used below 
  $json_string = file_get_contents("http://api.wunderground.com/api/$key/forecast10day/q/$wxlocation.json");
  $parsed_json = json_decode($json_string);

$today_high = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[0]->{'high'}->fahrenheit;
$today_low = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[0]->{'low'}->fahrenheit;
$today_pop = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[0]->{'pop'};

?>

                        <div class="panel-heading">
                           <b><?PHP echo "$location"; ?></b>
                        </div>
                         <div class="table-responsive">
                                <table class="table">
                                    <tbody>
                                        <tr>
                                            <td colspan="2" align="center" valign="top"><h2><?PHP echo "$tempf"; ?> °F</H2><br>
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
 

  $y = 0;
  while ($y < 4) {
  $hour = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'FCTTIME'}->hour;
  if ($hour > 12 ) {
      $hour = ($hour - 12); }
  elseif ($hour == 0) {
    $hour = 12;
  }

  $ampm = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'FCTTIME'}->ampm;
  
  $hour_tempf = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'temp'}->english;
  //$low = $parsed_json_hourly->{'forecast'}->{'simpleforecast'}->{'forecastday'}[$y]->{'low'}->fahrenheit;
  $hour_icon_url = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'icon_url'};
  $hour_condition = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'condition'};
  $hour_pop = $parsed_json_hourly->{'hourly_forecast'}[$y]->{'pop'};
  echo "<td align=\"center\"><center>";
  echo "<b><font size=\"2\"> ${hour} ${ampm}</B> <br>";
  echo "${hour_tempf}°<BR> </font>";
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
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            7-Day Forecast
                        </div>
                        <div class="panel-body">
                            <div class="table">
                                <table class="table">
                                    <tbody>
                                        <tr>

<?php


  $x = 1;
  while ($x < 8) {
  $weekday = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[$x]->{'date'}->weekday_short;
  $month = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[$x]->{'date'}->month;
  $day = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[$x]->{'date'}->day;
  $high = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[$x]->{'high'}->fahrenheit;
  $low = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[$x]->{'low'}->fahrenheit;
  $icon_url = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[$x]->{'icon_url'};
  $conditions = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[$x]->{'conditions'};
  $pop = $parsed_json->{'forecast'}->{'simpleforecast'}->{'forecastday'}[$x]->{'pop'};
  echo "<td><center>";
  echo "${weekday} ${month}/${day} <br>";
  echo "<font color=\"red\">${high}</font> | <font color=\"blue\">${low} </font><BR>";
  echo "<img class=\"img-responsive\" src=\"${icon_url}\"> <BR>";
  echo "${conditions} <br>";
  echo "<font size=\"1\" color=\"blue\">${pop}% </font><br></center>";
  
  echo "</td>";
  $x++;
}

?>
                                        </tr>
                                       
                                    </tbody>
                                </table>
   
                            </div>
                            <!-- /.table-responsive -->
                        </div>
                    </div>

                </div>
            </div>

             <!-- /.row -->
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
    
    <!-- High Charts -->
    <script src="/js/highcharts/highcharts.js"></script>
    <script src="/js/highcharts/modules/exporting.js"></script>

    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>

</html>
