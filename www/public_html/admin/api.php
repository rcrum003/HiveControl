<?PHP

#API Variables
$MODE="DEV"; #Dev or Prod


if ($MODE == "DEV") {
	#Dev URL
	$APIHOST="http://my.hivecontrol/";
} else {
	$APIHOST="https://www.hivecontrol.org/";
}

#API Version
$apiver="v1";

#Registration and other URLs
$regnew="/registration/new";
$regupdate="/registration/update";
$checkhive="/"

#Usable Variables
$API_REG_NEW = $APIHOST . $apiver . $regnew; 
$API_REG_UPDATE = $APIHOST . $apiver . $regupdate;
$API_CHECK_HIVE = $APIHOST . $apiver . $checkhive;

#echo "API is $API_REG_NEW";
?>
