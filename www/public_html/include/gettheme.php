
<?PHP


#Get Chart Config Option
$sth2 = $conn->prepare("SELECT www_chart_theme FROM hiveconfig");
$sth2->execute();
$theme = $sth2->fetchColumn();

#Return theme

echo "<script src='/js/highcharts/themes/";echo $theme; echo ".js'></script>";

 

?>


