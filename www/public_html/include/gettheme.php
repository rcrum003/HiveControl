
<?PHP


#Get Chart Config Option
$sth2 = $conn->prepare("SELECT * FROM hiveconfig");
$sth2->execute();
$result2 = $sth2->fetch(PDO::FETCH_ASSOC);

#Return theme

$theme = $result2['www_chart_theme'];

echo "<script src='/js/highcharts/themes/";echo $theme; echo ".js'></script>";

#Set default colors, since every chart pulls from this file

#$color_hivetemp="#FF1493"; 
#$color_hivehum="#0000FF"; #Blue
#$color_outtemp="#8A2BE2"; #Blue Vilolet
#$color_outhum="#000080"; #Navy
#$color_grossweight="#FFFF00"; #Yellow 
#$color_netweight="#FFD700"; #Gold
#$color_lux="#800000"; #Maroon
#$color_solarradiation="#FF0000"; #red
#$color_rain="#00FFFF"; #Aqua
#$color_gdd="#8B4513"; #SaddleBrown


    $color_hivetemp = $result2['color_hivetemp'];
    $color_hivehum = $result2['color_hivehum'];
    $color_outtemp = $result2['color_outtemp'];
    $color_outhum = $result2['color_outhum'];
    $color_grossweight = $result2['color_grossweight'];
    $color_netweight = $result2['color_netweight'];
    $color_lux = $result2['color_lux'];
    $color_solarradiation = $result2['color_solarradiation'];
    $color_rain = $result2['color_rain'];
    $color_gdd = $result2['color_gdd'];

    
    if ($result2['trend_hivetemp'] == "on") {$trend_hivetemp = "true";} else { $trend_hivetemp = "false"; }
    if ($result2['trend_hivehum'] == "on") {$trend_hivehum = "true";} else { $trend_hivehum = "false"; }
    if ($result2['trend_outtemp'] == "on") {$trend_outtemp = "true";} else { $trend_outtemp = "false"; }
    if ($result2['trend_outhum'] == "on") {$trend_outhum = "true";} else { $trend_outhum = "false"; }
    if ($result2['trend_grossweight'] == "on") {$trend_grossweight = "true";} else { $trend_grossweight = "false"; }
    if ($result2['trend_netweight'] == "on") {$trend_netweight = "true";} else { $trend_netweight = "false"; }
    if ($result2['trend_lux'] == "on") {$trend_lux = "true";} else { $trend_lux = "false"; }
    if ($result2['trend_solarradiation'] == "on") {$trend_solarradiation = "true";} else { $trend_solarradiation = "false"; }
    if ($result2['trend_rain'] == "on") {$trend_rain = "true";} else { $trend_rain = "false"; }
	if ($result2['trend_gdd'] == "on") {$trend_gdd = "true";} else { $trend_gdd = "false"; }


?>


