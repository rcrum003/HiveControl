<!DOCTYPE html>
<html lang="en">

<head>
   <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="Application to manage hive management tasks, and to report on various instruments.">
    <meta name="author" content="Ryan Crum">

    <title>Live Values</title>

    <!-- Bootstrap Core CSS -->
    <link href="../bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">

    <!-- MetisMenu CSS -->
    <link href="../bower_components/metisMenu/dist/metisMenu.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="../dist/css/sb-admin-2.css" rel="stylesheet">
    
    <!-- Custom Fonts -->
    <link href="../bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesnt work if you view the page via file:// -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->

</head>

<?PHP
#Script to get all the values

########################################################
# Get Config to see what we have setup
########################################################
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

 $sth = $conn->prepare("SELECT * FROM hiveconfig");
 $sth->execute();
 $r = $sth->fetch(PDO::FETCH_ASSOC);


#$r['ENABLE_HIVE_TEMP_CHK'] # yes,no
#$r['TEMPTYPE'] # temperhum, dht22, dht21




########################################################
# Weight
########################################################

########################################################
# Light
########################################################

########################################################
# Ambient Temp
########################################################


?>

<body>
            <div class="row">
                <div class="col-lg-12">
                        <div class="panel-body">
                            <div class="dataTable_wrapper">
                                <table class="table table-striped table-bordered table-hover" id="dataTables-example">
                                    <thead>
                                        <tr>
                                            <th>Instrument</th>
                                            <th>Raw Readings</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                    <?PHP 
                                    ########################################################
									# Hive Temp
									########################################################
									if ( $r['ENABLE_HIVE_TEMP_CHK'] == 'yes') {
										echo '<tr class="odd gradeX"> <td>Hive Temp/Humidity for <b>'.$r['TEMPTYPE'].'</b>';
										switch ($r['TEMPTYPE']) {
										    case "dht22":
										        $hivetemp = shell_exec("/home/HiveControl/scripts/temp/dht22.sh");
										        echo ' (Status, Temp C, Humidity %)</td>';
										        break;
										    case "dht21":
										        $hivetemp = shell_exec("/home/HiveControl/scripts/temp/dht22.sh");
										        echo ' (Status, Temp C, Humidity %)</td>';
										        break;
										    case "temperhum":
										        $hivetemp = shell_exec("/usr/bin/sudo /usr/local/bin/tempered");
										        echo '</td>';
										        break;
										}
	                                    echo '<td>'.'$hivetemp'.'</td></tr>';
									} 
                                    ########################################################
									# Hive Weight
									########################################################
									if ( $r['ENABLE_HIVE_WEIGHT_CHK'] == 'yes') {
										echo '<tr class="odd gradeX"> <td>Weight for <b>'.$r['SCALETYPE'].' - ZERO='.$r['</b>';
										switch ($r['TEMPTYPE']) {
										    case "hx711":
										        $weight = shell_exec("/home/HiveControl/scripts/temp/dht22.sh");
										        echo '</td>';
										        break;
										    case "phidget1046":
										        $weight = shell_exec("/home/HiveControl/scripts/weight/phidget1046.sh");
										        echo ' (Status, Temp C, Humidity %)</td>';
										        break;
										    case "cpw200plus":
										        $weight = shell_exec("/usr/bin/sudo /usr/local/bin/tempered");
										        echo '</td>';
										        break;
										}
	                                    echo '<td>'.'$hivetemp'.'</td></tr>';
									}






									#############################################
									?>
	                                    <tr class="odd gradeX">
	                                    <td>Weight</td>
	                                    <td>34</td>
	                                    </tr>
	                                    <tr class="odd gradeX">
	                                    <td>Light</td>
	                                    <td>34</td>
	                                    </tr>
	                                    <tr class="odd gradeX">
	                                    <td>Ambient Temp</td>
	                                    <td>34</td>
	                                    </tr>
	                                    <tr class="odd gradeX">
	                                    <td>Weight</td>
	                                    <td>34</td>
	                                    </tr>
                                    </tbody>
                              	</table>
					</div>
    			</div>
    		</div>
    	</div>



 	<!-- jQuery -->
    <script src="../bower_components/jquery/dist/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../bower_components/bootstrap/dist/js/bootstrap.min.js"></script>

    <!-- Metis Menu Plugin JavaScript -->
    <script src="../bower_components/metisMenu/dist/metisMenu.min.js"></script>
    
    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

 
</body>
</html>



