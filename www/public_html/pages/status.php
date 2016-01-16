
<?PHP

include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';


# Get main sensor data and store in a variable
$sth = $conn->prepare("select (strftime('%s','now', 'localtime') - strftime('%s',date)) as age, date, hivetempf, hiveweight, weather_tempf from allhivedata ORDER BY datetime(\"date\") DESC LIMIT 1;");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);

$AGE=$result['age'];
$HIVETEMPF=$result['hivetempf'];
$HIVEWEIGHT=$result['hiveweight'];
$WEATHERTEMPF=$result['weather_tempf'];

#Get GDD Data
#GDD 2015-12-05 23:04:47|0|3478
$gddsth = $conn->prepare("select (strftime('%s','now', 'localtime') - strftime('%s',calcdate)) as age2, daygdd, seasongdd from gdd ORDER BY datetime(\"calcdate\") DESC LIMIT 1;");
$gddsth->execute();
$gddresult = $gddsth->fetch(PDO::FETCH_ASSOC);


$GDDAGE=$gddresult['age2'];
$DAYGDD=$gddresult['daygdd'];
$SEASONGDD=$gddresult['seasongdd'];

#86400 seconds is 24 hrs
# Check to see if the weather was pulled within the last 10 minutes
# 10 Minutes = 600 seconds

                               #<button type="button" class="btn btn-success">Success</button>            
                               # <button type="button" class="btn btn-warning">Warning</button>
                               # <button type="button" class="btn btn-danger">Danger</button>

#Start table
#Row
echo '<button type="button" class="btn btn-';
if ($AGE > "600" || $HIVETEMPF > "150" || $HIVETEMPF < "-50") {
	echo "danger";
} else {
	echo "success";
}
echo '">Temp</button>';
#/Row

echo '<button type="button" class="btn btn-';
if ($AGE > "600" || $HIVEWEIGHT == "0" || $HIVEWEIGHT > "500" || $HIVEWEIGHT < "0") {
	echo "danger";
} else {
	echo "success";
}
echo '">Weight</button>';
#/Row


echo '<button type="button" class="btn btn-';
if ($AGE > "600" || $WEATHERTEMPF > "150" || $WEATHERTEMPF < "-70") {
	echo "danger";
} else {
	echo "success";
}
echo '">Weather</button>';
#/Row

#GDD, every 24 hrs (86400)
echo '<button type="button" class="btn btn-';
if ($GDDAGE > "86400" || $DAYGDD < "0" || $DAYGDD > "36" || $SEASONGDD > "5000" ) {
	echo "danger";
} else {
	echo "success";
}
echo '">GDD</button>';
#/Row


#    if ($result['age'] > "600") {
    #echo 'It has been more than 10 minutes since last report';
#	} 
#  	else {
#  		echo "We are good";
#  	}	



?>
