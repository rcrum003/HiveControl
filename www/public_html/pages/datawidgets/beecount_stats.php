



<?php
# We are going to use variables passed from the main page,
# Specifically - Period and Chart


switch ($period) {
    case "today":
        $sqlperiod = "start of day";
        break;
    case "day":
        $sqlperiod = "-1 days";
        break;
    case "week":
        $sqlperiod = "-7 days";
        break;
    case "month":
        $sqlperiod = "-1 months";
        break;
    case "year":
        $sqlperiod =  "-1 years";
        break;
    case "all":
        $sqlperiod =  "-20 years";
        break;
    }

    # Echo back the Javascript code
 
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

// Get Hive Data First
$sth = $conn->prepare("SELECT sum(IN_COUNT) as total_in, sum(OUT_COUNT) as total_out, strftime('%s',date)*1000 AS datetime FROM allhivedata WHERE date > datetime('now','$sqlperiod', 'localtime')");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);


$total_in = $result['total_in'];
$total_out = $result['total_out'];

?>


 




