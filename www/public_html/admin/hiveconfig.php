<?PHP

// Standard includes
include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");
require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';
//require $_SERVER["DOCUMENT_ROOT"] . '/admin/http.php';
//require $_SERVER["DOCUMENT_ROOT"] . '/admin/api.php';

###################################################
# Declare Empty Variables
###################################################
$GDDSTATUS="";

###################################################
# GET Process
###################################################
if ($_SERVER["REQUEST_METHOD"] == "GET") {
$sth = $conn->prepare("SELECT * FROM hiveconfig");
$sth->execute();
$result = $sth->fetch(PDO::FETCH_ASSOC);

$dateme=strtotime($result['GDD_START_DATE']);
$futuredate = strtotime('-1 year');
$isit = (strtotime($dateme) < strtotime('-1 year'));
#echo $dateme."\r";
#echo $futuredate."\r";

if (strtotime($result['GDD_START_DATE']) > strtotime('-1 year')) {
    // echo "score";
} else {
      $GDDSTATUS = "invalid";

}

}
###################################################
# POST Process
###################################################
if ($_SERVER["REQUEST_METHOD"] == "POST") {
// Set Error Fields

    $regex1 = "/^[a-zA-Z]+(?:[\s-][a-zA-Z]+)*$/";

    ###################################################
    # Validation
    ###################################################
    $v = new Valitron\Validator($_POST);
    $v->rule('required', ['HIVENAME', 'HIVEAPI', 'CITY', 'STATE', 'COUNTRY'], 1)->message('{field} is required');
    $v->rule('regex', 'HIVENAME', '/^[a-zA-Z0-9_-]+$/')->message('Hive Name can only contain letters, numbers, dashes, and underscores');
    $v->rule('slug', ['POWER', 'INTERNET', 'STATUS', 'COMPUTER']);
    #$v->rule('alphaNum', ['HIVEID'],  1)->message('{field} can only be alpha numeric');
    #$v->rule('lengthmin', ['HIVEID'], 1)->message('{field} is required to be 13 characters');
    $v->rule('lengthmax', ['HIVENAME', 'CITY', 'STATE', 'COUNTRY', 'LATITUDE', 'LONGITUDE', 'ZIP'], 40);
    $v->rule('lengthmax', ['HIVEAPI'], 70);
    $v->rule('alphaNum', ['HIVEAPI']);
    $v->rule('regex', ['CITY', 'STATE', 'COUNTRY'], $regex1);
    $v->rule('numeric', ['GDD_BASE_TEMP', 'ZIP']);

}
###################################################
# Functions 
###################################################
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
#####################################################################################################
?>


<!DOCTYPE html>
<html lang="en">


    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

    <div id="wrapper">
        
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Settings - Basic</h1>
                </div>
                <!-- /.col-lg-12 -->
            </div>
            <!-- /.row -->

<?PHP
        if ($_SERVER["REQUEST_METHOD"] == "POST") {
        
if($v->validate()) {

// Get current version    
    $ver = $conn->prepare("SELECT version FROM hiveconfig");
    $ver->execute();
    $ver = $ver->fetchColumn();
// Increment version
    $version = ++$ver;
    
// Make sure no bad stuff got through our filters
// Probably not needed, but doesn't hurt

    $hivename = test_input($_POST["HIVENAME"]);
    #$hiveid = test_input($_POST["HIVEID"]);
    $HIVEAPI = test_input($_POST["HIVEAPI"]);    
    $yardid = test_input($_POST["YARDID"]);
    $city = test_input($_POST["CITY"]);
    $state = test_input($_POST["STATE"]);
    $country = test_input($_POST["COUNTRY"]);
    $latitude = test_input($_POST["LATITUDE"]);
    $longitude = test_input($_POST["LONGITUDE"]);
    #$homedir = test_input_allow_slash($_POST["HOMEDIR"]);
    $timezone = test_input_allow_slash($_POST["TIMEZONE"]);
    $share_hivetool = test_input($_POST["SHARE_HIVETOOL"]);
    $HT_USERNAME = test_input($_POST["HT_USERNAME"]);
    $HT_PASSWORD_INPUT = test_input($_POST["HT_PASSWORD"]);
    $HT_URL = test_input($_POST["HT_URL"]);
    $GDD_BASE_TEMP = test_input($_POST["GDD_BASE_TEMP"]);
    $GDD_START_DATE = test_input_allow_slash($_POST["GDD_START_DATE"]);
    #$NASA_HONEYBEE_NET_ID = test_input($_POST["NASA_HONEYBEE_NET_ID"]);
    $POWER = test_input($_POST["POWER"]);
    $INTERNET = test_input($_POST["INTERNET"]);
    $STATUS = test_input($_POST["STATUS"]);
    $COMPUTER = test_input($_POST["COMPUTER"]);
    $START_DATE = test_input($_POST["START_DATE"]);

    $ZIP = test_input($_POST["ZIP"]);

    // SECURITY FIX: Only update password if a new one was provided
    // Get current password from database
    $sth_pwd = $conn->prepare("SELECT HT_PASSWORD FROM hiveconfig WHERE id=1");
    $sth_pwd->execute();
    $current_pwd = $sth_pwd->fetchColumn();

    // Use new password if provided, otherwise keep the old one
    $HT_PASSWORD = !empty($HT_PASSWORD_INPUT) ? $HT_PASSWORD_INPUT : $current_pwd;

    // Update into the DB
    $doit = $conn->prepare("UPDATE hiveconfig SET hivename=?,hiveapi=?,yardid=?,city=?,state=?,country=?,latitude=?,longitude=?,version=?,timezone=?,share_hivetool=?,HT_USERNAME=?,HT_PASSWORD=?,HT_URL=?,GDD_BASE_TEMP=?,GDD_START_DATE=?,POWER=?,INTERNET=?,STATUS=?,COMPUTER=?,START_DATE=?,ZIP=? WHERE id=1");
    $doit->execute(array($hivename,$HIVEAPI,$yardid,$city,$state,$country,$latitude,$longitude,$version,$timezone,$share_hivetool,$HT_USERNAME,$HT_PASSWORD,$HT_URL,$GDD_BASE_TEMP,$GDD_START_DATE,$POWER,$INTERNET,$STATUS,$COMPUTER,$START_DATE,$ZIP));
    sleep(3);

    // Refresh the fields in the form
    $sth2 = $conn->prepare("SELECT * FROM hiveconfig");
    $sth2->execute();
    $result = $sth2->fetch(PDO::FETCH_ASSOC);


    
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

    if ($GDDSTATUS == "invalid") {
        echo '<div class="alert alert-danger alert-dismissable">GDD Start Date is greater than 1 year. Please update!</div>';
        
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
                                        <tr>
                                            <th>Setting</th>
                                            <th>Value</th>
                                            <th>Description(s)</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                        <tr class="odd gradeX">
                                            <td>Hive API</td>
                                            <td><input type="text" name="HIVEAPI" value="<?PHP echo $result['HIVEAPI'];?>" onchange="this.form.submit()"></td>  
                                            <td>Required Key - register at <a href="https://www.hivecontrol.org/">HiveControl</a><br>
                                            Click on Beekeeper picture, then "Your Settings", then "API". Type a name, then hit create. Copy and paste that key here. Note: you can use one API key for all of your hives.</td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Hive Name</td>
                                            <td><input type="text" name="HIVENAME" value="<?PHP echo $result['HIVENAME'];?>" onchange="this.form.submit()"></td>  
                                            <td>Name identifier for this hive.</td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Hive ID</td>
                                            <td><?PHP echo $result['HIVEID'];?></td>
                                            <td>Unique identifier for this hive.</td>
                                        </tr>

                                        <tr class="odd gradeX">
                                            <td>City</td>
                                            <td><input type="text" name="CITY" value="<?PHP echo $result['CITY'];?>" onchange="this.form.submit()"></td>
                                            <td>Closest city to this hive, to pull weather data from. Also, displays next to hive on hivetool.org</td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>State</td>
                                            <td><input type="text" name="STATE" value="<?PHP echo $result['STATE'];?>" onchange="this.form.submit()"></td>
                                            <td>State where this hive is located.</td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Postal Code / ZIP</td>
                                            <td><input type="text" name="ZIP" value="<?PHP echo $result['ZIP'];?>" onchange="this.form.submit()"></td>
                                            <td>Postal Code/ZIP where this hive is located, used only for Ambient Pollen Counts - Omit if you don't want to collect</td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Country</td>
                                            <td><input type="text" name="COUNTRY" value="<?PHP echo $result['COUNTRY'];?>" onchange="this.form.submit()"></td>
                                            <td>Country where this hive is located</td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Latitude</td>
                                            <td><input type="text" name="LATITUDE" value="<?PHP echo $result['LATITUDE'];?>" onchange="this.form.submit()"></td>
                                            <td>Latitude of Hive</td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Longitude</td>
                                            <td><input type="text" name="LONGITUDE" value="<?PHP echo $result['LONGITUDE'];?>" onchange="this.form.submit()"></td>
                                            <td>Longitude of Hive</td>
                                        </tr>
                                        <tr class="odd gradeX">
                                            <td>Time Zone</td>
                                            <td><select name="TIMEZONE" onchange="this.form.submit()">
                                            <option value="Pacific/Midway" <?PHP if ($result['TIMEZONE'] == "Pacific/Midway") {echo "selected='selected'";} ?>>(UTC-11:00) Midway Island </option>
                                        <option value="Pacific/Samoa" <?PHP if ($result['TIMEZONE'] == "Pacific/Samoa") {echo "selected='selected'";} ?>>(UTC-11:00) Samoa </option>
                                        <option value="Pacific/Honolulu" <?PHP if ($result['TIMEZONE'] == "Pacific/Honolulu") {echo "selected='selected'";} ?>>(UTC-10:00) Hawaii </option>
                                        <option value="US/Alaska" <?PHP if ($result['TIMEZONE'] == "US/Alaska") {echo "selected='selected'";} ?>>(UTC-09:00) Alaska </option>
                                        <option value="America/Los_Angeles" <?PHP if ($result['TIMEZONE'] == "America/Los_Angeles") {echo "selected='selected'";} ?>>(UTC-08:00) Pacific Time (US &amp; Canada) </option>
                                        <option value="America/Tijuana" <?PHP if ($result['TIMEZONE'] == "America/Tijuana") {echo "selected='selected'";} ?>>(UTC-08:00) Tijuana </option>
                                        <option value="US/Arizona" <?PHP if ($result['TIMEZONE'] == "US/Arizona") {echo "selected='selected'";} ?>>(UTC-07:00) Arizona </option>
                                        <option value="America/Chihuahua" <?PHP if ($result['TIMEZONE'] == "America/Chihuahua") {echo "selected='selected'";} ?>>(UTC-07:00) Chihuahua </option>
                                        <option value="America/Chihuahua" <?PHP if ($result['TIMEZONE'] == "America/Chihuahua") {echo "selected='selected'";} ?>>(UTC-07:00) La Paz </option>
                                        <option value="America/Mazatlan" <?PHP if ($result['TIMEZONE'] == "America/Mazatlan") {echo "selected='selected'";} ?>>(UTC-07:00) Mazatlan </option>
                                        <option value="US/Mountain" <?PHP if ($result['TIMEZONE'] == "US/Mountain") {echo "selected='selected'";} ?>>(UTC-07:00) Mountain Time (US &amp; Canada) </option>
                                        <option value="America/Managua" <?PHP if ($result['TIMEZONE'] == "America/Managua") {echo "selected='selected'";} ?>>(UTC-06:00) Central America </option>
                                        <option value="US/Central" <?PHP if ($result['TIMEZONE'] == "US/Central") {echo "selected='selected'";} ?>>(UTC-06:00) Central Time (US &amp; Canada) </option>
                                        <option value="America/Mexico_City" <?PHP if ($result['TIMEZONE'] == "America/Mexico_City") {echo "selected='selected'";} ?>>(UTC-06:00) Guadalajara </option>
                                        <option value="America/Mexico_City" <?PHP if ($result['TIMEZONE'] == "America/Mexico_City") {echo "selected='selected'";} ?>>(UTC-06:00) Mexico City </option>
                                        <option value="America/Monterrey" <?PHP if ($result['TIMEZONE'] == "America/Monterrey") {echo "selected='selected'";} ?>>(UTC-06:00) Monterrey </option>
                                        <option value="Canada/Saskatchewan" <?PHP if ($result['TIMEZONE'] == "Canada/Saskatchewan") {echo "selected='selected'";} ?>>(UTC-06:00) Saskatchewan </option>
                                        <option value="America/Bogota" <?PHP if ($result['TIMEZONE'] == "America/Bogota") {echo "selected='selected'";} ?>>(UTC-05:00) Bogota </option>
                                        <option value="US/Eastern" <?PHP if ($result['TIMEZONE'] == "US/Eastern") {echo "selected='selected'";} ?>>(UTC-05:00) Eastern Time (US &amp; Canada) </option>
                                        <option value="US/East-Indiana" <?PHP if ($result['TIMEZONE'] == "US/East-Indiana") {echo "selected='selected'";} ?>>(UTC-05:00) Indiana (East) </option>
                                        <option value="America/Lima" <?PHP if ($result['TIMEZONE'] == "America/Lima") {echo "selected='selected'";} ?>>(UTC-05:00) Lima </option>
                                        <option value="America/Bogota" <?PHP if ($result['TIMEZONE'] == "America/Bogota") {echo "selected='selected'";} ?>>(UTC-05:00) Quito </option>
                                        <option value="Canada/Atlantic" <?PHP if ($result['TIMEZONE'] == "Canada/Atlantic") {echo "selected='selected'";} ?>>(UTC-04:00) Atlantic Time (Canada) </option>
                                        <option value="America/Caracas" <?PHP if ($result['TIMEZONE'] == "America/Caracas") {echo "selected='selected'";} ?>>(UTC-04:30) Caracas </option>
                                        <option value="America/La_Paz" <?PHP if ($result['TIMEZONE'] == "America/La_Paz") {echo "selected='selected'";} ?>>(UTC-04:00) La Paz </option>
                                        <option value="America/Santiago" <?PHP if ($result['TIMEZONE'] == "America/Santiago") {echo "selected='selected'";} ?>>(UTC-04:00) Santiago </option>
                                        <option value="Canada/Newfoundland" <?PHP if ($result['TIMEZONE'] == "Canada/Newfoundland") {echo "selected='selected'";} ?>>(UTC-03:30) Newfoundland </option>
                                        <option value="America/Sao_Paulo" <?PHP if ($result['TIMEZONE'] == "America/Sao_Paulo") {echo "selected='selected'";} ?>>(UTC-03:00) Brasilia </option>
                                        <option value="America/Argentina/Buenos_Aires" <?PHP if ($result['TIMEZONE'] == "America/Argentina/Buenos_Aires") {echo "selected='selected'";} ?>>(UTC-03:00) Buenos Aires </option>
                                        <option value="America/Argentina/Buenos_Aires" <?PHP if ($result['TIMEZONE'] == "America/Argentina/Buenos_Aires") {echo "selected='selected'";} ?>>(UTC-03:00) Georgetown </option>
                                        <option value="America/Godthab" <?PHP if ($result['TIMEZONE'] == "America/Godthab") {echo "selected='selected'";} ?>>(UTC-03:00) Greenland </option>
                                        <option value="America/Noronha" <?PHP if ($result['TIMEZONE'] == "America/Noronha") {echo "selected='selected'";} ?>>(UTC-02:00) Mid-Atlantic </option>
                                        <option value="Atlantic/Azores" <?PHP if ($result['TIMEZONE'] == "Atlantic/Azores") {echo "selected='selected'";} ?>>(UTC-01:00) Azores </option>
                                        <option value="Atlantic/Cape_Verde" <?PHP if ($result['TIMEZONE'] == "Atlantic/Cape_Verde") {echo "selected='selected'";} ?>>(UTC-01:00) Cape Verde Is. </option>
                                        <option value="Africa/Casablanca" <?PHP if ($result['TIMEZONE'] == "Africa/Casablanca") {echo "selected='selected'";} ?>>(UTC+00:00) Casablanca </option>
                                        <option value="Europe/London" <?PHP if ($result['TIMEZONE'] == "Europe/London") {echo "selected='selected'";} ?>>(UTC+00:00) Edinburgh </option>
                                        <option value="Etc/Greenwich" <?PHP if ($result['TIMEZONE'] == "Etc/Greenwich") {echo "selected='selected'";} ?>>(UTC+00:00) Greenwich Mean Time : Dublin </option>
                                        <option value="Europe/Lisbon" <?PHP if ($result['TIMEZONE'] == "Europe/Lisbon") {echo "selected='selected'";} ?>>(UTC+00:00) Lisbon </option>
                                        <option value="Europe/London" <?PHP if ($result['TIMEZONE'] == "Europe/London") {echo "selected='selected'";} ?>>(UTC+00:00) London </option>
                                        <option value="Africa/Monrovia" <?PHP if ($result['TIMEZONE'] == "Africa/Monrovia") {echo "selected='selected'";} ?>>(UTC+00:00) Monrovia </option>
                                        <option value="UTC" <?PHP if ($result['TIMEZONE'] == "UTC") {echo "selected='selected'";} ?>>(UTC+00:00) UTC </option>
                                        <option value="Europe/Amsterdam" <?PHP if ($result['TIMEZONE'] == "Europe/Amsterdam") {echo "selected='selected'";} ?>>(UTC+01:00) Amsterdam </option>
                                        <option value="Europe/Belgrade" <?PHP if ($result['TIMEZONE'] == "Europe/Belgrade") {echo "selected='selected'";} ?>>(UTC+01:00) Belgrade </option>
                                        <option value="Europe/Berlin" <?PHP if ($result['TIMEZONE'] == "Europe/Berlin") {echo "selected='selected'";} ?>>(UTC+01:00) Berlin </option>
                                        <option value="Europe/Berlin" <?PHP if ($result['TIMEZONE'] == "Europe/Berlin") {echo "selected='selected'";} ?>>(UTC+01:00) Bern </option>
                                        <option value="Europe/Bratislava" <?PHP if ($result['TIMEZONE'] == "Europe/Bratislava") {echo "selected='selected'";} ?>>(UTC+01:00) Bratislava </option>
                                        <option value="Europe/Brussels" <?PHP if ($result['TIMEZONE'] == "Europe/Brussels") {echo "selected='selected'";} ?>>(UTC+01:00) Brussels </option>
                                        <option value="Europe/Budapest" <?PHP if ($result['TIMEZONE'] == "Europe/Budapest") {echo "selected='selected'";} ?>>(UTC+01:00) Budapest </option>
                                        <option value="Europe/Copenhagen" <?PHP if ($result['TIMEZONE'] == "Europe/Copenhagen") {echo "selected='selected'";} ?>>(UTC+01:00) Copenhagen </option>
                                        <option value="Europe/Ljubljana" <?PHP if ($result['TIMEZONE'] == "Europe/Ljubljana") {echo "selected='selected'";} ?>>(UTC+01:00) Ljubljana </option>
                                        <option value="Europe/Madrid" <?PHP if ($result['TIMEZONE'] == "Europe/Madrid") {echo "selected='selected'";} ?>>(UTC+01:00) Madrid </option>
                                        <option value="Europe/Paris" <?PHP if ($result['TIMEZONE'] == "Europe/Paris") {echo "selected='selected'";} ?>>(UTC+01:00) Paris </option>
                                        <option value="Europe/Prague" <?PHP if ($result['TIMEZONE'] == "Europe/Prague") {echo "selected='selected'";} ?>>(UTC+01:00) Prague </option>
                                        <option value="Europe/Rome" <?PHP if ($result['TIMEZONE'] == "Europe/Rome") {echo "selected='selected'";} ?>>(UTC+01:00) Rome </option>
                                        <option value="Europe/Sarajevo" <?PHP if ($result['TIMEZONE'] == "Europe/Sarajevo") {echo "selected='selected'";} ?>>(UTC+01:00) Sarajevo </option>
                                        <option value="Europe/Skopje" <?PHP if ($result['TIMEZONE'] == "Europe/Skopje") {echo "selected='selected'";} ?>>(UTC+01:00) Skopje </option>
                                        <option value="Europe/Stockholm" <?PHP if ($result['TIMEZONE'] == "Europe/Stockholm") {echo "selected='selected'";} ?>>(UTC+01:00) Stockholm </option>
                                        <option value="Europe/Vienna" <?PHP if ($result['TIMEZONE'] == "Europe/Vienna") {echo "selected='selected'";} ?>>(UTC+01:00) Vienna </option>
                                        <option value="Europe/Warsaw" <?PHP if ($result['TIMEZONE'] == "Europe/Warsaw") {echo "selected='selected'";} ?>>(UTC+01:00) Warsaw </option>
                                        <option value="Africa/Lagos" <?PHP if ($result['TIMEZONE'] == "Africa/Lagos") {echo "selected='selected'";} ?>>(UTC+01:00) West Central Africa </option>
                                        <option value="Europe/Zagreb" <?PHP if ($result['TIMEZONE'] == "Europe/Zagreb") {echo "selected='selected'";} ?>>(UTC+01:00) Zagreb </option>
                                        <option value="Europe/Athens" <?PHP if ($result['TIMEZONE'] == "Europe/Athens") {echo "selected='selected'";} ?>>(UTC+02:00) Athens </option>
                                        <option value="Europe/Bucharest" <?PHP if ($result['TIMEZONE'] == "Europe/Bucharest") {echo "selected='selected'";} ?>>(UTC+02:00) Bucharest </option>
                                        <option value="Africa/Cairo" <?PHP if ($result['TIMEZONE'] == "Africa/Cairo") {echo "selected='selected'";} ?>>(UTC+02:00) Cairo </option>
                                        <option value="Africa/Harare" <?PHP if ($result['TIMEZONE'] == "Africa/Harare") {echo "selected='selected'";} ?>>(UTC+02:00) Harare </option>
                                        <option value="Europe/Helsinki" <?PHP if ($result['TIMEZONE'] == "Europe/Helsinki") {echo "selected='selected'";} ?>>(UTC+02:00) Helsinki </option>
                                        <option value="Europe/Istanbul" <?PHP if ($result['TIMEZONE'] == "Europe/Istanbul") {echo "selected='selected'";} ?>>(UTC+02:00) Istanbul </option>
                                        <option value="Asia/Jerusalem" <?PHP if ($result['TIMEZONE'] == "Asia/Jerusalem") {echo "selected='selected'";} ?>>(UTC+02:00) Jerusalem </option>
                                        <option value="Europe/Helsinki" <?PHP if ($result['TIMEZONE'] == "Europe/Helsinki") {echo "selected='selected'";} ?>>(UTC+02:00) Kyiv </option>
                                        <option value="Africa/Johannesburg" <?PHP if ($result['TIMEZONE'] == "Africa/Johannesburg") {echo "selected='selected'";} ?>>(UTC+02:00) Pretoria </option>
                                        <option value="Europe/Riga" <?PHP if ($result['TIMEZONE'] == "Europe/Riga") {echo "selected='selected'";} ?>>(UTC+02:00) Riga </option>
                                        <option value="Europe/Sofia" <?PHP if ($result['TIMEZONE'] == "Europe/Sofia") {echo "selected='selected'";} ?>>(UTC+02:00) Sofia </option>
                                        <option value="Europe/Tallinn" <?PHP if ($result['TIMEZONE'] == "Europe/Tallinn") {echo "selected='selected'";} ?>>(UTC+02:00) Tallinn </option>
                                        <option value="Europe/Vilnius" <?PHP if ($result['TIMEZONE'] == "Europe/Vilnius") {echo "selected='selected'";} ?>>(UTC+02:00) Vilnius </option>
                                        <option value="Asia/Baghdad" <?PHP if ($result['TIMEZONE'] == "Asia/Baghdad") {echo "selected='selected'";} ?>>(UTC+03:00) Baghdad </option>
                                        <option value="Asia/Kuwait" <?PHP if ($result['TIMEZONE'] == "Asia/Kuwait") {echo "selected='selected'";} ?>>(UTC+03:00) Kuwait </option>
                                        <option value="Europe/Minsk" <?PHP if ($result['TIMEZONE'] == "Europe/Minsk") {echo "selected='selected'";} ?>>(UTC+03:00) Minsk </option>
                                        <option value="Europe/Moscow" <?PHP if ($result['TIMEZONE'] == "Europe/Moscow") {echo "selected='selected'";} ?>>(UTC+03:00) Moscow </option>
                                        <option value="Africa/Nairobi" <?PHP if ($result['TIMEZONE'] == "Africa/Nairobi") {echo "selected='selected'";} ?>>(UTC+03:00) Nairobi </option>
                                        <option value="Asia/Riyadh" <?PHP if ($result['TIMEZONE'] == "Asia/Riyadh") {echo "selected='selected'";} ?>>(UTC+03:00) Riyadh </option>
                                        <option value="Europe/Moscow" <?PHP if ($result['TIMEZONE'] == "Europe/Moscow") {echo "selected='selected'";} ?>>(UTC+03:00) St. Petersburg </option>
                                        <option value="Europe/Volgograd" <?PHP if ($result['TIMEZONE'] == "Europe/Volgograd") {echo "selected='selected'";} ?>>(UTC+03:00) Volgograd </option>
                                        <option value="Asia/Tehran" <?PHP if ($result['TIMEZONE'] == "Asia/Tehran") {echo "selected='selected'";} ?>>(UTC+03:30) Tehran </option>
                                        <option value="Asia/Muscat" <?PHP if ($result['TIMEZONE'] == "Asia/Muscat") {echo "selected='selected'";} ?>>(UTC+04:00) Abu Dhabi </option>
                                        <option value="Asia/Baku" <?PHP if ($result['TIMEZONE'] == "Asia/Baku") {echo "selected='selected'";} ?>>(UTC+04:00) Baku </option>
                                        <option value="Asia/Muscat" <?PHP if ($result['TIMEZONE'] == "Asia/Muscat") {echo "selected='selected'";} ?>>(UTC+04:00) Muscat </option>
                                        <option value="Asia/Tbilisi" <?PHP if ($result['TIMEZONE'] == "Asia/Tbilisi") {echo "selected='selected'";} ?>>(UTC+04:00) Tbilisi </option>
                                        <option value="Asia/Yerevan" <?PHP if ($result['TIMEZONE'] == "Asia/Yerevan") {echo "selected='selected'";} ?>>(UTC+04:00) Yerevan </option>
                                        <option value="Asia/Kabul" <?PHP if ($result['TIMEZONE'] == "Asia/Kabul") {echo "selected='selected'";} ?>>(UTC+04:30) Kabul </option>
                                        <option value="Asia/Yekaterinburg" <?PHP if ($result['TIMEZONE'] == "Asia/Yekaterinburg") {echo "selected='selected'";} ?>>(UTC+05:00) Ekaterinburg </option>
                                        <option value="Asia/Karachi" <?PHP if ($result['TIMEZONE'] == "Asia/Karachi") {echo "selected='selected'";} ?>>(UTC+05:00) Islamabad </option>
                                        <option value="Asia/Karachi" <?PHP if ($result['TIMEZONE'] == "Asia/Karachi") {echo "selected='selected'";} ?>>(UTC+05:00) Karachi </option>
                                        <option value="Asia/Tashkent" <?PHP if ($result['TIMEZONE'] == "Asia/Tashkent") {echo "selected='selected'";} ?>>(UTC+05:00) Tashkent </option>
                                        <option value="Asia/Calcutta" <?PHP if ($result['TIMEZONE'] == "Asia/Calcutta") {echo "selected='selected'";} ?>>(UTC+05:30) Chennai </option>
                                        <option value="Asia/Kolkata" <?PHP if ($result['TIMEZONE'] == "Asia/Kolkata") {echo "selected='selected'";} ?>>(UTC+05:30) Kolkata </option>
                                        <option value="Asia/Calcutta" <?PHP if ($result['TIMEZONE'] == "Asia/Calcutta") {echo "selected='selected'";} ?>>(UTC+05:30) Mumbai </option>
                                        <option value="Asia/Calcutta" <?PHP if ($result['TIMEZONE'] == "Asia/Calcutta") {echo "selected='selected'";} ?>>(UTC+05:30) New Delhi </option>
                                        <option value="Asia/Calcutta" <?PHP if ($result['TIMEZONE'] == "Asia/Calcutta") {echo "selected='selected'";} ?>>(UTC+05:30) Sri Jayawardenepura </option>
                                        <option value="Asia/Katmandu" <?PHP if ($result['TIMEZONE'] == "Asia/Katmandu") {echo "selected='selected'";} ?>>(UTC+05:45) Kathmandu </option>
                                        <option value="Asia/Almaty" <?PHP if ($result['TIMEZONE'] == "Asia/Almaty") {echo "selected='selected'";} ?>>(UTC+06:00) Almaty </option>
                                        <option value="Asia/Dhaka" <?PHP if ($result['TIMEZONE'] == "Asia/Dhaka") {echo "selected='selected'";} ?>>(UTC+06:00) Astana </option>
                                        <option value="Asia/Dhaka" <?PHP if ($result['TIMEZONE'] == "Asia/Dhaka") {echo "selected='selected'";} ?>>(UTC+06:00) Dhaka </option>
                                        <option value="Asia/Novosibirsk" <?PHP if ($result['TIMEZONE'] == "Asia/Novosibirsk") {echo "selected='selected'";} ?>>(UTC+06:00) Novosibirsk </option>
                                        <option value="Asia/Rangoon" <?PHP if ($result['TIMEZONE'] == "Asia/Rangoon") {echo "selected='selected'";} ?>>(UTC+06:30) Rangoon </option>
                                        <option value="Asia/Bangkok" <?PHP if ($result['TIMEZONE'] == "Asia/Bangkok") {echo "selected='selected'";} ?>>(UTC+07:00) Bangkok </option>
                                        <option value="Asia/Bangkok" <?PHP if ($result['TIMEZONE'] == "Asia/Bangkok") {echo "selected='selected'";} ?>>(UTC+07:00) Hanoi </option>
                                        <option value="Asia/Jakarta" <?PHP if ($result['TIMEZONE'] == "Asia/Jakarta") {echo "selected='selected'";} ?>>(UTC+07:00) Jakarta </option>
                                        <option value="Asia/Krasnoyarsk" <?PHP if ($result['TIMEZONE'] == "Asia/Krasnoyarsk") {echo "selected='selected'";} ?>>(UTC+07:00) Krasnoyarsk </option>
                                        <option value="Asia/Hong_Kong" <?PHP if ($result['TIMEZONE'] == "Asia/Hong_Kong") {echo "selected='selected'";} ?>>(UTC+08:00) Beijing </option>
                                        <option value="Asia/Chongqing" <?PHP if ($result['TIMEZONE'] == "Asia/Chongqing") {echo "selected='selected'";} ?>>(UTC+08:00) Chongqing </option>
                                        <option value="Asia/Hong_Kong" <?PHP if ($result['TIMEZONE'] == "Asia/Hong_Kong") {echo "selected='selected'";} ?>>(UTC+08:00) Hong Kong </option>
                                        <option value="Asia/Irkutsk" <?PHP if ($result['TIMEZONE'] == "Asia/Irkutsk") {echo "selected='selected'";} ?>>(UTC+08:00) Irkutsk </option>
                                        <option value="Asia/Kuala_Lumpur" <?PHP if ($result['TIMEZONE'] == "Asia/Kuala_Lumpur") {echo "selected='selected'";} ?>>(UTC+08:00) Kuala Lumpur </option>
                                        <option value="Asia/Manila" <?PHP if ($result['TIMEZONE'] == "Asia/Manila") {echo "selected='selected'";} ?>>(UTC+08:00) Manila </option>
                                        <option value="Australia/Perth" <?PHP if ($result['TIMEZONE'] == "Australia/Perth") {echo "selected='selected'";} ?>>(UTC+08:00) Perth </option>
                                        <option value="Asia/Singapore" <?PHP if ($result['TIMEZONE'] == "Asia/Singapore") {echo "selected='selected'";} ?>>(UTC+08:00) Singapore </option>
                                        <option value="Asia/Taipei" <?PHP if ($result['TIMEZONE'] == "Asia/Taipei") {echo "selected='selected'";} ?>>(UTC+08:00) Taipei </option>
                                        <option value="Asia/Ulan_Bator" <?PHP if ($result['TIMEZONE'] == "Asia/Ulan_Bator") {echo "selected='selected'";} ?>>(UTC+08:00) Ulaan Bataar </option>
                                        <option value="Asia/Urumqi" <?PHP if ($result['TIMEZONE'] == "Asia/Urumqi") {echo "selected='selected'";} ?>>(UTC+08:00) Urumqi </option>
                                        <option value="Asia/Tokyo" <?PHP if ($result['TIMEZONE'] == "Asia/Tokyo") {echo "selected='selected'";} ?>>(UTC+09:00) Osaka </option>
                                        <option value="Asia/Tokyo" <?PHP if ($result['TIMEZONE'] == "Asia/Tokyo") {echo "selected='selected'";} ?>>(UTC+09:00) Sapporo </option>
                                        <option value="Asia/Seoul" <?PHP if ($result['TIMEZONE'] == "Asia/Seoul") {echo "selected='selected'";} ?>>(UTC+09:00) Seoul </option>
                                        <option value="Asia/Tokyo" <?PHP if ($result['TIMEZONE'] == "Asia/Tokyo") {echo "selected='selected'";} ?>>(UTC+09:00) Tokyo </option>
                                        <option value="Asia/Yakutsk" <?PHP if ($result['TIMEZONE'] == "Asia/Yakutsk") {echo "selected='selected'";} ?>>(UTC+09:00) Yakutsk </option>
                                        <option value="Australia/Adelaide" <?PHP if ($result['TIMEZONE'] == "Australia/Adelaide") {echo "selected='selected'";} ?>>(UTC+09:30) Adelaide </option>
                                        <option value="Australia/Darwin" <?PHP if ($result['TIMEZONE'] == "Australia/Darwin") {echo "selected='selected'";} ?>>(UTC+09:30) Darwin </option>
                                        <option value="Australia/Brisbane" <?PHP if ($result['TIMEZONE'] == "Australia/Brisbane") {echo "selected='selected'";} ?>>(UTC+10:00) Brisbane </option>
                                        <option value="Australia/Canberra" <?PHP if ($result['TIMEZONE'] == "Australia/Canberra") {echo "selected='selected'";} ?>>(UTC+10:00) Canberra </option>
                                        <option value="Pacific/Guam" <?PHP if ($result['TIMEZONE'] == "Pacific/Guam") {echo "selected='selected'";} ?>>(UTC+10:00) Guam </option>
                                        <option value="Australia/Hobart" <?PHP if ($result['TIMEZONE'] == "Australia/Hobart") {echo "selected='selected'";} ?>>(UTC+10:00) Hobart </option>
                                        <option value="Asia/Magadan" <?PHP if ($result['TIMEZONE'] == "Asia/Magadan") {echo "selected='selected'";} ?>>(UTC+10:00) Magadan </option>
                                        <option value="Australia/Melbourne" <?PHP if ($result['TIMEZONE'] == "Australia/Melbourne") {echo "selected='selected'";} ?>>(UTC+10:00) Melbourne </option>
                                        <option value="Pacific/Port_Moresby" <?PHP if ($result['TIMEZONE'] == "Pacific/Port_Moresby") {echo "selected='selected'";} ?>>(UTC+10:00) Port Moresby </option>
                                        <option value="Australia/Sydney" <?PHP if ($result['TIMEZONE'] == "Australia/Sydney") {echo "selected='selected'";} ?>>(UTC+10:00) Sydney </option>
                                        <option value="Asia/Vladivostok" <?PHP if ($result['TIMEZONE'] == "Asia/Vladivostok") {echo "selected='selected'";} ?>>(UTC+10:00) Vladivostok </option>
                                        <option value="Pacific/Auckland" <?PHP if ($result['TIMEZONE'] == "Pacific/Auckland") {echo "selected='selected'";} ?>>(UTC+12:00) Auckland </option>
                                        <option value="Pacific/Fiji" <?PHP if ($result['TIMEZONE'] == "Pacific/Fiji") {echo "selected='selected'";} ?>>(UTC+12:00) Fiji </option>
                                        <option value="Pacific/Kwajalein" <?PHP if ($result['TIMEZONE'] == "Pacific/Kwajalein") {echo "selected='selected'";} ?>>(UTC+12:00) International Date Line West </option>
                                        <option value="Asia/Kamchatka" <?PHP if ($result['TIMEZONE'] == "Asia/Kamchatka") {echo "selected='selected'";} ?>>(UTC+12:00) Kamchatka </option>
                                        <option value="Pacific/Fiji" <?PHP if ($result['TIMEZONE'] == "Pacific/Fiji") {echo "selected='selected'";} ?>>(UTC+12:00) Marshall Is. </option>
                                        <option value="Asia/Magadan" <?PHP if ($result['TIMEZONE'] == "Asia/Magadan") {echo "selected='selected'";} ?>>(UTC+12:00) New Caledonia </option>
                                        <option value="Asia/Magadan" <?PHP if ($result['TIMEZONE'] == "Asia/Magadan") {echo "selected='selected'";} ?>>(UTC+12:00) Solomon Is. </option>
                                        <option value="Pacific/Auckland" <?PHP if ($result['TIMEZONE'] == "Pacific/Auckland") {echo "selected='selected'";} ?>>(UTC+12:00) Wellington </option>
                                        <option value="Pacific/Tongatapu" <?PHP if ($result['TIMEZONE'] == "Pacific/Tongatapu") {echo "selected='selected'";} ?>>(UTC+13:00) Nuku\alofa </option>
                                        </select>
                                        </td>
                                            <td>Timezone where Hive is located</td>
                                        </tr>
                                       <tr class="odd gradeX">
                                        <td>GDD Base Temp</td>
                                        <td><input type="text" name="GDD_BASE_TEMP" value="<?PHP echo $result['GDD_BASE_TEMP'];?>" onchange="this.form.submit()"></td> 
                                        <td>Base Temperature to start calculating GDD at. - Recommend 50</td>
                                       </tr> 
                                    </tr>
                                       <tr class="odd gradeX">
                                        <td>GDD Start Date</td>
                                        <td><input type="text" name="GDD_START_DATE" value="<?PHP echo $result['GDD_START_DATE'];?>" onchange="this.form.submit()"></td> 
                                        <td>Beginning date to start calculating GDD each year. - year, month, day (format 20150301) - Recommend the last freezing day. Most people recommend March 1, and most GDD plant values are based on a 3/1 start date. </td>
                                       </tr>

                                   <! ***************************************************** -->

                                   <tr class="odd gradeX">
                                        <td>Power Source</td>
                                        <td><select name="POWER">
                                        <option value="AC" <?php if ($result['POWER'] == "AC") {echo "selected='selected'";} ?> onchange="this.form.submit()">AC</option>
                                        <option value="solar" <?php if ($result['POWER'] == "solar") {echo "selected='selected'";} ?>>Solar</option> 
                                        </select>
                                        </td>
                                        <td>Specify if your hive is on AC or solar power</td>
                                       </tr>

                                   <! ***************************************************** -->

                                
                                   <tr class="odd gradeX">
                                        <td>Internet</td>
                                        <td><select name="INTERNET">
                                        <option value="wi-fi" <?php if ($result['INTERNET'] == "wi-fi") {echo "selected='selected'";} ?> onchange="this.form.submit()">Wi-Fi</option>
                                        <option value="ethernet" <?php if ($result['INTERNET'] == "ethernet") {echo "selected='selected'";} ?> onchange="this.form.submit()">Ethernet</option> 
                                        </select>
                                        </td>
                                        <td>Specify if your hive is using Wi-Fi or Ethernet</td>
                                       </tr>

                                   <! ***************************************************** -->


                                   <tr class="odd gradeX">
                                        <td>Status</td>
                                        <td><select name="STATUS" onchange="this.form.submit()">
                                        <option value="testing" <?php if ($result['STATUS'] == "testing") {echo "selected='selected'";} ?>>Testing</option>
                                        <option value="online" <?php if ($result['STATUS'] == "online") {echo "selected='selected'";} ?>>Online</option> 
                                        <option value="offline" <?php if ($result['STATUS'] == "offline") {echo "selected='selected'";} ?>>Offline</option>
                                        </select>
                                        </td>
                                        <td>Specify the status of your hive (used for Hivetool.org data reporting).</td>
                                       </tr>

                                   <! ***************************************************** -->

                                   <tr class="odd gradeX">
                                        <td>Computer</td>
                                        <td><select name="COMPUTER" onchange="this.form.submit()">
                                        <option value="pi" <?php if ($result['COMPUTER'] == "pi") {echo "selected='selected'";} ?>>PI</option>
                                        <option value="arduino" <?php if ($result['COMPUTER'] == "arduino") {echo "selected='selected'";} ?>>Arduino</option>
                                        <option value="x86" <?php if ($result['COMPUTER'] == "x86") {echo "selected='selected'";} ?>>Other x86</option>  
                                        </select>
                                        </td>
                                        <td>Specify what computing platform you are using (used for Hivetool reporting).</td>
                                   </tr>

                                   <! ***************************************************** -->

                                    <tr class="odd gradeX">
                                        <td>Start Date</td>
                                        <td><input type="text" id="datepicker" name="START_DATE" value="<?PHP echo $result['START_DATE'];?>" onchange="this.form.submit()"></td> 
                                        <td>Date you put this hive into Online status for Hivetool.org. </td>
                                   </tr>

                                    </tbody>
                                </table>
                            </div>
                            <!-- /.table-responsive -->
                       

            <div class="dataTable_wrapper">
                <table class="table table-striped table-bordered table-hover" id="dataTables-example">
                    <thead>
                        <tr>
                            <th>Setting</th>
                            <th>Choice</th>
                            <th>Options</th>
                            <th>Description(s)</th>
                        </tr>
                    </thead>
                    <tbody>
                       
                        
                         <tr class="odd gradeX">
                            <td>Share Data with Hivetool.org</td>
                            <td><select name="SHARE_HIVETOOL" onchange="this.form.submit()">
                            <option value="yes" <?php if ($result['SHARE_HIVETOOL'] == "yes") {echo "selected='selected'";} ?>>Yes</option>
                            <option value="no" <?php if ($result['SHARE_HIVETOOL'] == "no") {echo "selected='selected'";} ?>>No</option>
                            </td>
                            <td>
                                 <?php if ($result['SHARE_HIVETOOL'] == "yes") {


                                    echo 'Username <input type="text" name="HT_USERNAME" onchange="this.form.submit()" value="'; echo htmlspecialchars($result['HT_USERNAME']); echo '"><BR>';
                                    // SECURITY FIX: Don't pre-fill password field to prevent exposure
                                    echo 'Password <input type="password" name="HT_PASSWORD" placeholder="Enter to change" onchange="this.form.submit()"><BR>';
                                    echo 'URL <BR><input type="text" name="HT_URL" onchange="this.form.submit()" value="'; echo htmlspecialchars($result['HT_URL']); echo '"><BR>';
                                }
                                else {
                                    echo '<input type="hidden" name="HT_USERNAME" value="'; echo htmlspecialchars($result['HT_USERNAME']); echo '"><BR>';
                                    // SECURITY FIX: Don't pre-fill password field to prevent exposure
                                    echo '<input type="hidden" name="HT_PASSWORD" value="">';
                                    echo '<input type="hidden" name="HT_URL" value="'; echo htmlspecialchars($result['HT_URL']); echo '">';

                                } ?>

                            </td>
                            <td>Specify if you want to share data with Hivetool.org.<BR> (Please do, it helps our researchers out)</td>
                        </tr>


                    </tbody>
                </table>
        </div></div>    
        </div>
                        <!-- /.panel-body -->
                    </div>
                    <!-- /.panel -->
                </div>
                <!-- /.col-lg-12 -->
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


    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>

</html>
