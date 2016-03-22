
<?PHP


#Get Chart Config Option
$sth2 = $conn->prepare("SELECT www_chart_theme FROM hiveconfig");
$sth2->execute();
$theme = $sth2->fetchColumn();

#Return theme

echo "<script src='/js/highcharts/themes/";echo $theme; echo ".js'></script>";

#Set default colors, since every chart pulls from this file

$color_hivetemp="#FF1493"; 
$color_hivehum="#0000FF"; #Blue
$color_outtemp="#8A2BE2"; #Blue Vilolet
$color_outhum="#000080"; #Navy
$color_grossweight="#FFFF00"; #Yellow 
$color_netweight="#FFD700"; #Gold
$color_lux="#800000"; #Maroon
$color_solarradiation="#FF0000"; #red
$color_rain="#00FFFF"; #Aqua
$color_gdd="#8B4513"; #SaddleBrown


?>


