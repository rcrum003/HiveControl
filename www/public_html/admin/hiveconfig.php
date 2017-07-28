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
    $v->rule('slug', ['HIVENAME', 'POWER', 'INTERNET', 'STATUS', 'COMPUTER']);
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
    $HT_PASSWORD = test_input($_POST["HT_PASSWORD"]);
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
                                            <td>Hive API</td>
                                            <td><input type="text" name="HIVEAPI" value="<?PHP echo $result['HIVEAPI'];?>" onchange="this.form.submit()"></td>  
                                            <td>Required Key - register yours are <a href="https://www.hivecontrol.org/">HiveControl</td>
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
<option value="Africa/Abidjan" <?php if ($result['TIMEZONE'] == "Africa/Abidjan") {echo "selected='selected'";} ?>>Africa/Abidjan</option>
<option value="Africa/Accra" <?php if ($result['TIMEZONE'] == "Africa/Accra") {echo "selected='selected'";} ?>>Africa/Accra</option>
<option value="Africa/Addis_Ababa" <?php if ($result['TIMEZONE'] == "Africa/Addis_Ababa") {echo "selected='selected'";} ?>>Africa/Addis_Ababa</option>
<option value="Africa/Algiers" <?php if ($result['TIMEZONE'] == "Africa/Algiers") {echo "selected='selected'";} ?>>Africa/Algiers</option>
<option value="Africa/Asmara" <?php if ($result['TIMEZONE'] == "Africa/Asmara") {echo "selected='selected'";} ?>>Africa/Asmara</option>
<option value="Africa/Bamako" <?php if ($result['TIMEZONE'] == "Africa/Bamako") {echo "selected='selected'";} ?>>Africa/Bamako</option>
<option value="Africa/Bangui" <?php if ($result['TIMEZONE'] == "Africa/Bangui") {echo "selected='selected'";} ?>>Africa/Bangui</option>
<option value="Africa/Banjul" <?php if ($result['TIMEZONE'] == "Africa/Banjul") {echo "selected='selected'";} ?>>Africa/Banjul</option>
<option value="Africa/Bissau" <?php if ($result['TIMEZONE'] == "Africa/Bissau") {echo "selected='selected'";} ?>>Africa/Bissau</option>
<option value="Africa/Blantyre" <?php if ($result['TIMEZONE'] == "Africa/Blantyre") {echo "selected='selected'";} ?>>Africa/Blantyre</option>
<option value="Africa/Brazzaville" <?php if ($result['TIMEZONE'] == "Africa/Brazzaville") {echo "selected='selected'";} ?>>Africa/Brazzaville</option>
<option value="Africa/Bujumbura" <?php if ($result['TIMEZONE'] == "Africa/Bujumbura") {echo "selected='selected'";} ?>>Africa/Bujumbura</option>
<option value="Africa/Cairo" <?php if ($result['TIMEZONE'] == "Africa/Cairo") {echo "selected='selected'";} ?>>Africa/Cairo</option>
<option value="Africa/Casablanca" <?php if ($result['TIMEZONE'] == "Africa/Casablanca") {echo "selected='selected'";} ?>>Africa/Casablanca</option>
<option value="Africa/Ceuta" <?php if ($result['TIMEZONE'] == "Africa/Ceuta") {echo "selected='selected'";} ?>>Africa/Ceuta</option>
<option value="Africa/Conakry" <?php if ($result['TIMEZONE'] == "Africa/Conakry") {echo "selected='selected'";} ?>>Africa/Conakry</option>
<option value="Africa/Dakar" <?php if ($result['TIMEZONE'] == "Africa/Dakar") {echo "selected='selected'";} ?>>Africa/Dakar</option>
<option value="Africa/Dar_es_Salaam" <?php if ($result['TIMEZONE'] == "Africa/Dar_es_Salaam") {echo "selected='selected'";} ?>>Africa/Dar_es_Salaam</option>
<option value="Africa/Djibouti" <?php if ($result['TIMEZONE'] == "Africa/Djibouti") {echo "selected='selected'";} ?>>Africa/Djibouti</option>
<option value="Africa/Douala" <?php if ($result['TIMEZONE'] == "Africa/Douala") {echo "selected='selected'";} ?>>Africa/Douala</option>
<option value="Africa/El_Aaiun" <?php if ($result['TIMEZONE'] == "Africa/El_Aaiun") {echo "selected='selected'";} ?>>Africa/El_Aaiun</option>
<option value="Africa/Freetown" <?php if ($result['TIMEZONE'] == "Africa/Freetown") {echo "selected='selected'";} ?>>Africa/Freetown</option>
<option value="Africa/Gaborone" <?php if ($result['TIMEZONE'] == "Africa/Gaborone") {echo "selected='selected'";} ?>>Africa/Gaborone</option>
<option value="Africa/Harare" <?php if ($result['TIMEZONE'] == "Africa/Harare") {echo "selected='selected'";} ?>>Africa/Harare</option>
<option value="Africa/Johannesburg" <?php if ($result['TIMEZONE'] == "Africa/Johannesburg") {echo "selected='selected'";} ?>>Africa/Johannesburg</option>
<option value="Africa/Juba" <?php if ($result['TIMEZONE'] == "Africa/Juba") {echo "selected='selected'";} ?>>Africa/Juba</option>
<option value="Africa/Kampala" <?php if ($result['TIMEZONE'] == "Africa/Kampala") {echo "selected='selected'";} ?>>Africa/Kampala</option>
<option value="Africa/Khartoum" <?php if ($result['TIMEZONE'] == "Africa/Khartoum") {echo "selected='selected'";} ?>>Africa/Khartoum</option>
<option value="Africa/Kigali" <?php if ($result['TIMEZONE'] == "Africa/Kigali") {echo "selected='selected'";} ?>>Africa/Kigali</option>
<option value="Africa/Kinshasa" <?php if ($result['TIMEZONE'] == "Africa/Kinshasa") {echo "selected='selected'";} ?>>Africa/Kinshasa</option>
<option value="Africa/Lagos" <?php if ($result['TIMEZONE'] == "Africa/Lagos") {echo "selected='selected'";} ?>>Africa/Lagos</option>
<option value="Africa/Libreville" <?php if ($result['TIMEZONE'] == "Africa/Libreville") {echo "selected='selected'";} ?>>Africa/Libreville</option>
<option value="Africa/Lome" <?php if ($result['TIMEZONE'] == "Africa/Lome") {echo "selected='selected'";} ?>>Africa/Lome</option>
<option value="Africa/Luanda" <?php if ($result['TIMEZONE'] == "Africa/Luanda") {echo "selected='selected'";} ?>>Africa/Luanda</option>
<option value="Africa/Lubumbashi" <?php if ($result['TIMEZONE'] == "Africa/Lubumbashi") {echo "selected='selected'";} ?>>Africa/Lubumbashi</option>
<option value="Africa/Lusaka" <?php if ($result['TIMEZONE'] == "Africa/Lusaka") {echo "selected='selected'";} ?>>Africa/Lusaka</option>
<option value="Africa/Malabo" <?php if ($result['TIMEZONE'] == "Africa/Malabo") {echo "selected='selected'";} ?>>Africa/Malabo</option>
<option value="Africa/Maputo" <?php if ($result['TIMEZONE'] == "Africa/Maputo") {echo "selected='selected'";} ?>>Africa/Maputo</option>
<option value="Africa/Maseru" <?php if ($result['TIMEZONE'] == "Africa/Maseru") {echo "selected='selected'";} ?>>Africa/Maseru</option>
<option value="Africa/Mbabane" <?php if ($result['TIMEZONE'] == "Africa/Mbabane") {echo "selected='selected'";} ?>>Africa/Mbabane</option>
<option value="Africa/Mogadishu" <?php if ($result['TIMEZONE'] == "Africa/Mogadishu") {echo "selected='selected'";} ?>>Africa/Mogadishu</option>
<option value="Africa/Monrovia" <?php if ($result['TIMEZONE'] == "Africa/Monrovia") {echo "selected='selected'";} ?>>Africa/Monrovia</option>
<option value="Africa/Nairobi" <?php if ($result['TIMEZONE'] == "Africa/Nairobi") {echo "selected='selected'";} ?>>Africa/Nairobi</option>
<option value="Africa/Ndjamena" <?php if ($result['TIMEZONE'] == "Africa/Ndjamena") {echo "selected='selected'";} ?>>Africa/Ndjamena</option>
<option value="Africa/Niamey" <?php if ($result['TIMEZONE'] == "Africa/Niamey") {echo "selected='selected'";} ?>>Africa/Niamey</option>
<option value="Africa/Nouakchott" <?php if ($result['TIMEZONE'] == "Africa/Nouakchott") {echo "selected='selected'";} ?>>Africa/Nouakchott</option>
<option value="Africa/Ouagadougou" <?php if ($result['TIMEZONE'] == "Africa/Ouagadougou") {echo "selected='selected'";} ?>>Africa/Ouagadougou</option>
<option value="Africa/Porto-Novo" <?php if ($result['TIMEZONE'] == "Africa/Porto-Novo") {echo "selected='selected'";} ?>>Africa/Porto-Novo</option>
<option value="Africa/Sao_Tome" <?php if ($result['TIMEZONE'] == "Africa/Sao_Tome") {echo "selected='selected'";} ?>>Africa/Sao_Tome</option>
<option value="Africa/Tripoli" <?php if ($result['TIMEZONE'] == "Africa/Tripoli") {echo "selected='selected'";} ?>>Africa/Tripoli</option>
<option value="Africa/Tunis" <?php if ($result['TIMEZONE'] == "Africa/Tunis") {echo "selected='selected'";} ?>>Africa/Tunis</option>
<option value="Africa/Windhoek" <?php if ($result['TIMEZONE'] == "Africa/Windhoek") {echo "selected='selected'";} ?>>Africa/Windhoek</option>
<option value="America/Adak" <?php if ($result['TIMEZONE'] == "America/Adak") {echo "selected='selected'";} ?>>America/Adak</option>
<option value="America/Anchorage" <?php if ($result['TIMEZONE'] == "America/Anchorage") {echo "selected='selected'";} ?>>America/Anchorage</option>
<option value="America/Anguilla" <?php if ($result['TIMEZONE'] == "America/Anguilla") {echo "selected='selected'";} ?>>America/Anguilla</option>
<option value="America/Antigua" <?php if ($result['TIMEZONE'] == "America/Antigua") {echo "selected='selected'";} ?>>America/Antigua</option>
<option value="America/Araguaina" <?php if ($result['TIMEZONE'] == "America/Araguaina") {echo "selected='selected'";} ?>>America/Araguaina</option>
<option value="America/Argentina/Buenos_Aires" <?php if ($result['TIMEZONE'] == "America/Argentina/Buenos_Aires") {echo "selected='selected'";} ?>>America/Argentina/Buenos_Aires</option>
<option value="America/Argentina/Catamarca" <?php if ($result['TIMEZONE'] == "America/Argentina/Catamarca") {echo "selected='selected'";} ?>>America/Argentina/Catamarca</option>
<option value="America/Argentina/Cordoba" <?php if ($result['TIMEZONE'] == "America/Argentina/Cordoba") {echo "selected='selected'";} ?>>America/Argentina/Cordoba</option>
<option value="America/Argentina/Jujuy" <?php if ($result['TIMEZONE'] == "America/Argentina/Jujuy") {echo "selected='selected'";} ?>>America/Argentina/Jujuy</option>
<option value="America/Argentina/La_Rioja" <?php if ($result['TIMEZONE'] == "America/Argentina/La_Rioja") {echo "selected='selected'";} ?>>America/Argentina/La_Rioja</option>
<option value="America/Argentina/Mendoza" <?php if ($result['TIMEZONE'] == "America/Argentina/Mendoza") {echo "selected='selected'";} ?>>America/Argentina/Mendoza</option>
<option value="America/Argentina/Rio_Gallegos" <?php if ($result['TIMEZONE'] == "America/Argentina/Rio_Gallegos") {echo "selected='selected'";} ?>>America/Argentina/Rio_Gallegos</option>
<option value="America/Argentina/Salta" <?php if ($result['TIMEZONE'] == "America/Argentina/Salta") {echo "selected='selected'";} ?>>America/Argentina/Salta</option>
<option value="America/Argentina/San_Juan" <?php if ($result['TIMEZONE'] == "America/Argentina/San_Juan") {echo "selected='selected'";} ?>>America/Argentina/San_Juan</option>
<option value="America/Argentina/San_Luis" <?php if ($result['TIMEZONE'] == "America/Argentina/San_Luis") {echo "selected='selected'";} ?>>America/Argentina/San_Luis</option>
<option value="America/Argentina/Tucuman" <?php if ($result['TIMEZONE'] == "America/Argentina/Tucuman") {echo "selected='selected'";} ?>>America/Argentina/Tucuman</option>
<option value="America/Argentina/Ushuaia" <?php if ($result['TIMEZONE'] == "America/Argentina/Ushuaia") {echo "selected='selected'";} ?>>America/Argentina/Ushuaia</option>
<option value="America/Aruba" <?php if ($result['TIMEZONE'] == "America/Aruba") {echo "selected='selected'";} ?>>America/Aruba</option>
<option value="America/Asuncion" <?php if ($result['TIMEZONE'] == "America/Asuncion") {echo "selected='selected'";} ?>>America/Asuncion</option>
<option value="America/Atikokan" <?php if ($result['TIMEZONE'] == "America/Atikokan") {echo "selected='selected'";} ?>>America/Atikokan</option>
<option value="America/Bahia" <?php if ($result['TIMEZONE'] == "America/Bahia") {echo "selected='selected'";} ?>>America/Bahia</option>
<option value="America/Bahia_Banderas" <?php if ($result['TIMEZONE'] == "America/Bahia_Banderas") {echo "selected='selected'";} ?>>America/Bahia_Banderas</option>
<option value="America/Barbados" <?php if ($result['TIMEZONE'] == "America/Barbados") {echo "selected='selected'";} ?>>America/Barbados</option>
<option value="America/Belem" <?php if ($result['TIMEZONE'] == "America/Belem") {echo "selected='selected'";} ?>>America/Belem</option>
<option value="America/Belize" <?php if ($result['TIMEZONE'] == "America/Belize") {echo "selected='selected'";} ?>>America/Belize</option>
<option value="America/Blanc-Sablon" <?php if ($result['TIMEZONE'] == "America/Blanc-Sablon") {echo "selected='selected'";} ?>>America/Blanc-Sablon</option>
<option value="America/Boa_Vista" <?php if ($result['TIMEZONE'] == "America/Boa_Vista") {echo "selected='selected'";} ?>>America/Boa_Vista</option>
<option value="America/Bogota" <?php if ($result['TIMEZONE'] == "America/Bogota") {echo "selected='selected'";} ?>>America/Bogota</option>
<option value="America/Boise" <?php if ($result['TIMEZONE'] == "America/Boise") {echo "selected='selected'";} ?>>America/Boise</option>
<option value="America/Cambridge_Bay" <?php if ($result['TIMEZONE'] == "America/Cambridge_Bay") {echo "selected='selected'";} ?>>America/Cambridge_Bay</option>
<option value="America/Campo_Grande" <?php if ($result['TIMEZONE'] == "America/Campo_Grande") {echo "selected='selected'";} ?>>America/Campo_Grande</option>
<option value="America/Cancun" <?php if ($result['TIMEZONE'] == "America/Cancun") {echo "selected='selected'";} ?>>America/Cancun</option>
<option value="America/Caracas" <?php if ($result['TIMEZONE'] == "America/Caracas") {echo "selected='selected'";} ?>>America/Caracas</option>
<option value="America/Cayenne" <?php if ($result['TIMEZONE'] == "America/Cayenne") {echo "selected='selected'";} ?>>America/Cayenne</option>
<option value="America/Cayman" <?php if ($result['TIMEZONE'] == "America/Cayman") {echo "selected='selected'";} ?>>America/Cayman</option>
<option value="America/Chicago" <?php if ($result['TIMEZONE'] == "America/Chicago") {echo "selected='selected'";} ?>>America/Chicago</option>
<option value="America/Chihuahua" <?php if ($result['TIMEZONE'] == "America/Chihuahua") {echo "selected='selected'";} ?>>America/Chihuahua</option>
<option value="America/Costa_Rica" <?php if ($result['TIMEZONE'] == "America/Costa_Rica") {echo "selected='selected'";} ?>>America/Costa_Rica</option>
<option value="America/Creston" <?php if ($result['TIMEZONE'] == "America/Creston") {echo "selected='selected'";} ?>>America/Creston</option>
<option value="America/Cuiaba" <?php if ($result['TIMEZONE'] == "America/Cuiaba") {echo "selected='selected'";} ?>>America/Cuiaba</option>
<option value="America/Curacao" <?php if ($result['TIMEZONE'] == "America/Curacao") {echo "selected='selected'";} ?>>America/Curacao</option>
<option value="America/Danmarkshavn" <?php if ($result['TIMEZONE'] == "America/Danmarkshavn") {echo "selected='selected'";} ?>>America/Danmarkshavn</option>
<option value="America/Dawson" <?php if ($result['TIMEZONE'] == "America/Dawson") {echo "selected='selected'";} ?>>America/Dawson</option>
<option value="America/Dawson_Creek" <?php if ($result['TIMEZONE'] == "America/Dawson_Creek") {echo "selected='selected'";} ?>>America/Dawson_Creek</option>
<option value="America/Denver" <?php if ($result['TIMEZONE'] == "America/Denver") {echo "selected='selected'";} ?>>America/Denver</option>
<option value="America/Detroit" <?php if ($result['TIMEZONE'] == "America/Detroit") {echo "selected='selected'";} ?>>America/Detroit</option>
<option value="America/Dominica" <?php if ($result['TIMEZONE'] == "America/Dominica") {echo "selected='selected'";} ?>>America/Dominica</option>
<option value="America/Edmonton" <?php if ($result['TIMEZONE'] == "America/Edmonton") {echo "selected='selected'";} ?>>America/Edmonton</option>
<option value="America/Eirunepe" <?php if ($result['TIMEZONE'] == "America/Eirunepe") {echo "selected='selected'";} ?>>America/Eirunepe</option>
<option value="America/El_Salvador" <?php if ($result['TIMEZONE'] == "America/El_Salvador") {echo "selected='selected'";} ?>>America/El_Salvador</option>
<option value="America/Fortaleza" <?php if ($result['TIMEZONE'] == "America/Fortaleza") {echo "selected='selected'";} ?>>America/Fortaleza</option>
<option value="America/Fort_Nelson" <?php if ($result['TIMEZONE'] == "America/Fort_Nelson") {echo "selected='selected'";} ?>>America/Fort_Nelson</option>
<option value="America/Glace_Bay" <?php if ($result['TIMEZONE'] == "America/Glace_Bay") {echo "selected='selected'";} ?>>America/Glace_Bay</option>
<option value="America/Godthab" <?php if ($result['TIMEZONE'] == "America/Godthab") {echo "selected='selected'";} ?>>America/Godthab</option>
<option value="America/Goose_Bay" <?php if ($result['TIMEZONE'] == "America/Goose_Bay") {echo "selected='selected'";} ?>>America/Goose_Bay</option>
<option value="America/Grand_Turk" <?php if ($result['TIMEZONE'] == "America/Grand_Turk") {echo "selected='selected'";} ?>>America/Grand_Turk</option>
<option value="America/Grenada" <?php if ($result['TIMEZONE'] == "America/Grenada") {echo "selected='selected'";} ?>>America/Grenada</option>
<option value="America/Guadeloupe" <?php if ($result['TIMEZONE'] == "America/Guadeloupe") {echo "selected='selected'";} ?>>America/Guadeloupe</option>
<option value="America/Guatemala" <?php if ($result['TIMEZONE'] == "America/Guatemala") {echo "selected='selected'";} ?>>America/Guatemala</option>
<option value="America/Guayaquil" <?php if ($result['TIMEZONE'] == "America/Guayaquil") {echo "selected='selected'";} ?>>America/Guayaquil</option>
<option value="America/Guyana" <?php if ($result['TIMEZONE'] == "America/Guyana") {echo "selected='selected'";} ?>>America/Guyana</option>
<option value="America/Halifax" <?php if ($result['TIMEZONE'] == "America/Halifax") {echo "selected='selected'";} ?>>America/Halifax</option>
<option value="America/Havana" <?php if ($result['TIMEZONE'] == "America/Havana") {echo "selected='selected'";} ?>>America/Havana</option>
<option value="America/Hermosillo" <?php if ($result['TIMEZONE'] == "America/Hermosillo") {echo "selected='selected'";} ?>>America/Hermosillo</option>
<option value="America/Indiana/Indianapolis" <?php if ($result['TIMEZONE'] == "America/Indiana/Indianapolis") {echo "selected='selected'";} ?>>America/Indiana/Indianapolis</option>
<option value="America/Indiana/Knox" <?php if ($result['TIMEZONE'] == "America/Indiana/Knox") {echo "selected='selected'";} ?>>America/Indiana/Knox</option>
<option value="America/Indiana/Marengo" <?php if ($result['TIMEZONE'] == "America/Indiana/Marengo") {echo "selected='selected'";} ?>>America/Indiana/Marengo</option>
<option value="America/Indiana/Petersburg" <?php if ($result['TIMEZONE'] == "America/Indiana/Petersburg") {echo "selected='selected'";} ?>>America/Indiana/Petersburg</option>
<option value="America/Indiana/Tell_City" <?php if ($result['TIMEZONE'] == "America/Indiana/Tell_City") {echo "selected='selected'";} ?>>America/Indiana/Tell_City</option>
<option value="America/Indiana/Vevay" <?php if ($result['TIMEZONE'] == "America/Indiana/Vevay") {echo "selected='selected'";} ?>>America/Indiana/Vevay</option>
<option value="America/Indiana/Vincennes" <?php if ($result['TIMEZONE'] == "America/Indiana/Vincennes") {echo "selected='selected'";} ?>>America/Indiana/Vincennes</option>
<option value="America/Indiana/Winamac" <?php if ($result['TIMEZONE'] == "America/Indiana/Winamac") {echo "selected='selected'";} ?>>America/Indiana/Winamac</option>
<option value="America/Inuvik" <?php if ($result['TIMEZONE'] == "America/Inuvik") {echo "selected='selected'";} ?>>America/Inuvik</option>
<option value="America/Iqaluit" <?php if ($result['TIMEZONE'] == "America/Iqaluit") {echo "selected='selected'";} ?>>America/Iqaluit</option>
<option value="America/Jamaica" <?php if ($result['TIMEZONE'] == "America/Jamaica") {echo "selected='selected'";} ?>>America/Jamaica</option>
<option value="America/Juneau" <?php if ($result['TIMEZONE'] == "America/Juneau") {echo "selected='selected'";} ?>>America/Juneau</option>
<option value="America/Kentucky/Louisville" <?php if ($result['TIMEZONE'] == "America/Kentucky/Louisville") {echo "selected='selected'";} ?>>America/Kentucky/Louisville</option>
<option value="America/Kentucky/Monticello" <?php if ($result['TIMEZONE'] == "America/Kentucky/Monticello") {echo "selected='selected'";} ?>>America/Kentucky/Monticello</option>
<option value="America/Kralendijk" <?php if ($result['TIMEZONE'] == "America/Kralendijk") {echo "selected='selected'";} ?>>America/Kralendijk</option>
<option value="America/La_Paz" <?php if ($result['TIMEZONE'] == "America/La_Paz") {echo "selected='selected'";} ?>>America/La_Paz</option>
<option value="America/Lima" <?php if ($result['TIMEZONE'] == "America/Lima") {echo "selected='selected'";} ?>>America/Lima</option>
<option value="America/Los_Angeles" <?php if ($result['TIMEZONE'] == "America/Los_Angeles") {echo "selected='selected'";} ?>>America/Los_Angeles</option>
<option value="America/Lower_Princes" <?php if ($result['TIMEZONE'] == "America/Lower_Princes") {echo "selected='selected'";} ?>>America/Lower_Princes</option>
<option value="America/Maceio" <?php if ($result['TIMEZONE'] == "America/Maceio") {echo "selected='selected'";} ?>>America/Maceio</option>
<option value="America/Managua" <?php if ($result['TIMEZONE'] == "America/Managua") {echo "selected='selected'";} ?>>America/Managua</option>
<option value="America/Manaus" <?php if ($result['TIMEZONE'] == "America/Manaus") {echo "selected='selected'";} ?>>America/Manaus</option>
<option value="America/Marigot" <?php if ($result['TIMEZONE'] == "America/Marigot") {echo "selected='selected'";} ?>>America/Marigot</option>
<option value="America/Martinique" <?php if ($result['TIMEZONE'] == "America/Martinique") {echo "selected='selected'";} ?>>America/Martinique</option>
<option value="America/Matamoros" <?php if ($result['TIMEZONE'] == "America/Matamoros") {echo "selected='selected'";} ?>>America/Matamoros</option>
<option value="America/Mazatlan" <?php if ($result['TIMEZONE'] == "America/Mazatlan") {echo "selected='selected'";} ?>>America/Mazatlan</option>
<option value="America/Menominee" <?php if ($result['TIMEZONE'] == "America/Menominee") {echo "selected='selected'";} ?>>America/Menominee</option>
<option value="America/Merida" <?php if ($result['TIMEZONE'] == "America/Merida") {echo "selected='selected'";} ?>>America/Merida</option>
<option value="America/Metlakatla" <?php if ($result['TIMEZONE'] == "America/Metlakatla") {echo "selected='selected'";} ?>>America/Metlakatla</option>
<option value="America/Mexico_City" <?php if ($result['TIMEZONE'] == "America/Mexico_City") {echo "selected='selected'";} ?>>America/Mexico_City</option>
<option value="America/Miquelon" <?php if ($result['TIMEZONE'] == "America/Miquelon") {echo "selected='selected'";} ?>>America/Miquelon</option>
<option value="America/Moncton" <?php if ($result['TIMEZONE'] == "America/Moncton") {echo "selected='selected'";} ?>>America/Moncton</option>
<option value="America/Monterrey" <?php if ($result['TIMEZONE'] == "America/Monterrey") {echo "selected='selected'";} ?>>America/Monterrey</option>
<option value="America/Montevideo" <?php if ($result['TIMEZONE'] == "America/Montevideo") {echo "selected='selected'";} ?>>America/Montevideo</option>
<option value="America/Montserrat" <?php if ($result['TIMEZONE'] == "America/Montserrat") {echo "selected='selected'";} ?>>America/Montserrat</option>
<option value="America/Nassau" <?php if ($result['TIMEZONE'] == "America/Nassau") {echo "selected='selected'";} ?>>America/Nassau</option>
<option value="America/New_York" <?php if ($result['TIMEZONE'] == "America/New_York") {echo "selected='selected'";} ?>>America/New_York</option>
<option value="America/Nipigon" <?php if ($result['TIMEZONE'] == "America/Nipigon") {echo "selected='selected'";} ?>>America/Nipigon</option>
<option value="America/Nome" <?php if ($result['TIMEZONE'] == "America/Nome") {echo "selected='selected'";} ?>>America/Nome</option>
<option value="America/Noronha" <?php if ($result['TIMEZONE'] == "America/Noronha") {echo "selected='selected'";} ?>>America/Noronha</option>
<option value="America/North_Dakota/Beulah" <?php if ($result['TIMEZONE'] == "America/North_Dakota/Beulah") {echo "selected='selected'";} ?>>America/North_Dakota/Beulah</option>
<option value="America/North_Dakota/Center" <?php if ($result['TIMEZONE'] == "America/North_Dakota/Center") {echo "selected='selected'";} ?>>America/North_Dakota/Center</option>
<option value="America/North_Dakota/New_Salem" <?php if ($result['TIMEZONE'] == "America/North_Dakota/New_Salem") {echo "selected='selected'";} ?>>America/North_Dakota/New_Salem</option>
<option value="America/Ojinaga" <?php if ($result['TIMEZONE'] == "America/Ojinaga") {echo "selected='selected'";} ?>>America/Ojinaga</option>
<option value="America/Panama" <?php if ($result['TIMEZONE'] == "America/Panama") {echo "selected='selected'";} ?>>America/Panama</option>
<option value="America/Pangnirtung" <?php if ($result['TIMEZONE'] == "America/Pangnirtung") {echo "selected='selected'";} ?>>America/Pangnirtung</option>
<option value="America/Paramaribo" <?php if ($result['TIMEZONE'] == "America/Paramaribo") {echo "selected='selected'";} ?>>America/Paramaribo</option>
<option value="America/Phoenix" <?php if ($result['TIMEZONE'] == "America/Phoenix") {echo "selected='selected'";} ?>>America/Phoenix</option>
<option value="America/Port-au-Prince" <?php if ($result['TIMEZONE'] == "America/Port-au-Prince") {echo "selected='selected'";} ?>>America/Port-au-Prince</option>
<option value="America/Port_of_Spain" <?php if ($result['TIMEZONE'] == "America/Port_of_Spain") {echo "selected='selected'";} ?>>America/Port_of_Spain</option>
<option value="America/Porto_Velho" <?php if ($result['TIMEZONE'] == "America/Porto_Velho") {echo "selected='selected'";} ?>>America/Porto_Velho</option>
<option value="America/Puerto_Rico" <?php if ($result['TIMEZONE'] == "America/Puerto_Rico") {echo "selected='selected'";} ?>>America/Puerto_Rico</option>
<option value="America/Rainy_River" <?php if ($result['TIMEZONE'] == "America/Rainy_River") {echo "selected='selected'";} ?>>America/Rainy_River</option>
<option value="America/Rankin_Inlet" <?php if ($result['TIMEZONE'] == "America/Rankin_Inlet") {echo "selected='selected'";} ?>>America/Rankin_Inlet</option>
<option value="America/Recife" <?php if ($result['TIMEZONE'] == "America/Recife") {echo "selected='selected'";} ?>>America/Recife</option>
<option value="America/Regina" <?php if ($result['TIMEZONE'] == "America/Regina") {echo "selected='selected'";} ?>>America/Regina</option>
<option value="America/Resolute" <?php if ($result['TIMEZONE'] == "America/Resolute") {echo "selected='selected'";} ?>>America/Resolute</option>
<option value="America/Rio_Branco" <?php if ($result['TIMEZONE'] == "America/Rio_Branco") {echo "selected='selected'";} ?>>America/Rio_Branco</option>
<option value="America/Santa_Isabel" <?php if ($result['TIMEZONE'] == "America/Santa_Isabel") {echo "selected='selected'";} ?>>America/Santa_Isabel</option>
<option value="America/Santarem" <?php if ($result['TIMEZONE'] == "America/Santarem") {echo "selected='selected'";} ?>>America/Santarem</option>
<option value="America/Santiago" <?php if ($result['TIMEZONE'] == "America/Santiago") {echo "selected='selected'";} ?>>America/Santiago</option>
<option value="America/Santo_Domingo" <?php if ($result['TIMEZONE'] == "America/Santo_Domingo") {echo "selected='selected'";} ?>>America/Santo_Domingo</option>
<option value="America/Sao_Paulo" <?php if ($result['TIMEZONE'] == "America/Sao_Paulo") {echo "selected='selected'";} ?>>America/Sao_Paulo</option>
<option value="America/Scoresbysund" <?php if ($result['TIMEZONE'] == "America/Scoresbysund") {echo "selected='selected'";} ?>>America/Scoresbysund</option>
<option value="America/Sitka" <?php if ($result['TIMEZONE'] == "America/Sitka") {echo "selected='selected'";} ?>>America/Sitka</option>
<option value="America/St_Barthelemy" <?php if ($result['TIMEZONE'] == "America/St_Barthelemy") {echo "selected='selected'";} ?>>America/St_Barthelemy</option>
<option value="America/St_Johns" <?php if ($result['TIMEZONE'] == "America/St_Johns") {echo "selected='selected'";} ?>>America/St_Johns</option>
<option value="America/St_Kitts" <?php if ($result['TIMEZONE'] == "America/St_Kitts") {echo "selected='selected'";} ?>>America/St_Kitts</option>
<option value="America/St_Lucia" <?php if ($result['TIMEZONE'] == "America/St_Lucia") {echo "selected='selected'";} ?>>America/St_Lucia</option>
<option value="America/St_Thomas" <?php if ($result['TIMEZONE'] == "America/St_Thomas") {echo "selected='selected'";} ?>>America/St_Thomas</option>
<option value="America/St_Vincent" <?php if ($result['TIMEZONE'] == "America/St_Vincent") {echo "selected='selected'";} ?>>America/St_Vincent</option>
<option value="America/Swift_Current" <?php if ($result['TIMEZONE'] == "America/Swift_Current") {echo "selected='selected'";} ?>>America/Swift_Current</option>
<option value="America/Tegucigalpa" <?php if ($result['TIMEZONE'] == "America/Tegucigalpa") {echo "selected='selected'";} ?>>America/Tegucigalpa</option>
<option value="America/Thule" <?php if ($result['TIMEZONE'] == "America/Thule") {echo "selected='selected'";} ?>>America/Thule</option>
<option value="America/Thunder_Bay" <?php if ($result['TIMEZONE'] == "America/Thunder_Bay") {echo "selected='selected'";} ?>>America/Thunder_Bay</option>
<option value="America/Tijuana" <?php if ($result['TIMEZONE'] == "America/Tijuana") {echo "selected='selected'";} ?>>America/Tijuana</option>
<option value="America/Toronto" <?php if ($result['TIMEZONE'] == "America/Toronto") {echo "selected='selected'";} ?>>America/Toronto</option>
<option value="America/Tortola" <?php if ($result['TIMEZONE'] == "America/Tortola") {echo "selected='selected'";} ?>>America/Tortola</option>
<option value="America/Vancouver" <?php if ($result['TIMEZONE'] == "America/Vancouver") {echo "selected='selected'";} ?>>America/Vancouver</option>
<option value="America/Whitehorse" <?php if ($result['TIMEZONE'] == "America/Whitehorse") {echo "selected='selected'";} ?>>America/Whitehorse</option>
<option value="America/Winnipeg" <?php if ($result['TIMEZONE'] == "America/Winnipeg") {echo "selected='selected'";} ?>>America/Winnipeg</option>
<option value="America/Yakutat" <?php if ($result['TIMEZONE'] == "America/Yakutat") {echo "selected='selected'";} ?>>America/Yakutat</option>
<option value="America/Yellowknife" <?php if ($result['TIMEZONE'] == "America/Yellowknife") {echo "selected='selected'";} ?>>America/Yellowknife</option>
<option value="Antarctica/Casey" <?php if ($result['TIMEZONE'] == "Antarctica/Casey") {echo "selected='selected'";} ?>>Antarctica/Casey</option>
<option value="Antarctica/Davis" <?php if ($result['TIMEZONE'] == "Antarctica/Davis") {echo "selected='selected'";} ?>>Antarctica/Davis</option>
<option value="Antarctica/DumontDUrville" <?php if ($result['TIMEZONE'] == "Antarctica/DumontDUrville") {echo "selected='selected'";} ?>>Antarctica/DumontDUrville</option>
<option value="Antarctica/Macquarie" <?php if ($result['TIMEZONE'] == "Antarctica/Macquarie") {echo "selected='selected'";} ?>>Antarctica/Macquarie</option>
<option value="Antarctica/Mawson" <?php if ($result['TIMEZONE'] == "Antarctica/Mawson") {echo "selected='selected'";} ?>>Antarctica/Mawson</option>
<option value="Antarctica/McMurdo" <?php if ($result['TIMEZONE'] == "Antarctica/McMurdo") {echo "selected='selected'";} ?>>Antarctica/McMurdo</option>
<option value="Antarctica/Palmer" <?php if ($result['TIMEZONE'] == "Antarctica/Palmer") {echo "selected='selected'";} ?>>Antarctica/Palmer</option>
<option value="Antarctica/Rothera" <?php if ($result['TIMEZONE'] == "Antarctica/Rothera") {echo "selected='selected'";} ?>>Antarctica/Rothera</option>
<option value="Antarctica/Syowa" <?php if ($result['TIMEZONE'] == "Antarctica/Syowa") {echo "selected='selected'";} ?>>Antarctica/Syowa</option>
<option value="Antarctica/Troll" <?php if ($result['TIMEZONE'] == "Antarctica/Troll") {echo "selected='selected'";} ?>>Antarctica/Troll</option>
<option value="Antarctica/Vostok" <?php if ($result['TIMEZONE'] == "Antarctica/Vostok") {echo "selected='selected'";} ?>>Antarctica/Vostok</option>
<option value="Arctic/Longyearbyen" <?php if ($result['TIMEZONE'] == "Arctic/Longyearbyen") {echo "selected='selected'";} ?>>Arctic/Longyearbyen</option>
<option value="Asia/Aden" <?php if ($result['TIMEZONE'] == "Asia/Aden") {echo "selected='selected'";} ?>>Asia/Aden</option>
<option value="Asia/Almaty" <?php if ($result['TIMEZONE'] == "Asia/Almaty") {echo "selected='selected'";} ?>>Asia/Almaty</option>
<option value="Asia/Amman" <?php if ($result['TIMEZONE'] == "Asia/Amman") {echo "selected='selected'";} ?>>Asia/Amman</option>
<option value="Asia/Anadyr" <?php if ($result['TIMEZONE'] == "Asia/Anadyr") {echo "selected='selected'";} ?>>Asia/Anadyr</option>
<option value="Asia/Aqtau" <?php if ($result['TIMEZONE'] == "Asia/Aqtau") {echo "selected='selected'";} ?>>Asia/Aqtau</option>
<option value="Asia/Aqtobe" <?php if ($result['TIMEZONE'] == "Asia/Aqtobe") {echo "selected='selected'";} ?>>Asia/Aqtobe</option>
<option value="Asia/Ashgabat" <?php if ($result['TIMEZONE'] == "Asia/Ashgabat") {echo "selected='selected'";} ?>>Asia/Ashgabat</option>
<option value="Asia/Baghdad" <?php if ($result['TIMEZONE'] == "Asia/Baghdad") {echo "selected='selected'";} ?>>Asia/Baghdad</option>
<option value="Asia/Bahrain" <?php if ($result['TIMEZONE'] == "Asia/Bahrain") {echo "selected='selected'";} ?>>Asia/Bahrain</option>
<option value="Asia/Baku" <?php if ($result['TIMEZONE'] == "Asia/Baku") {echo "selected='selected'";} ?>>Asia/Baku</option>
<option value="Asia/Bangkok" <?php if ($result['TIMEZONE'] == "Asia/Bangkok") {echo "selected='selected'";} ?>>Asia/Bangkok</option>
<option value="Asia/Beirut" <?php if ($result['TIMEZONE'] == "Asia/Beirut") {echo "selected='selected'";} ?>>Asia/Beirut</option>
<option value="Asia/Bishkek" <?php if ($result['TIMEZONE'] == "Asia/Bishkek") {echo "selected='selected'";} ?>>Asia/Bishkek</option>
<option value="Asia/Brunei" <?php if ($result['TIMEZONE'] == "Asia/Brunei") {echo "selected='selected'";} ?>>Asia/Brunei</option>
<option value="Asia/Chita" <?php if ($result['TIMEZONE'] == "Asia/Chita") {echo "selected='selected'";} ?>>Asia/Chita</option>
<option value="Asia/Choibalsan" <?php if ($result['TIMEZONE'] == "Asia/Choibalsan") {echo "selected='selected'";} ?>>Asia/Choibalsan</option>
<option value="Asia/Colombo" <?php if ($result['TIMEZONE'] == "Asia/Colombo") {echo "selected='selected'";} ?>>Asia/Colombo</option>
<option value="Asia/Damascus" <?php if ($result['TIMEZONE'] == "Asia/Damascus") {echo "selected='selected'";} ?>>Asia/Damascus</option>
<option value="Asia/Dhaka" <?php if ($result['TIMEZONE'] == "Asia/Dhaka") {echo "selected='selected'";} ?>>Asia/Dhaka</option>
<option value="Asia/Dili" <?php if ($result['TIMEZONE'] == "Asia/Dili") {echo "selected='selected'";} ?>>Asia/Dili</option>
<option value="Asia/Dubai" <?php if ($result['TIMEZONE'] == "Asia/Dubai") {echo "selected='selected'";} ?>>Asia/Dubai</option>
<option value="Asia/Dushanbe" <?php if ($result['TIMEZONE'] == "Asia/Dushanbe") {echo "selected='selected'";} ?>>Asia/Dushanbe</option>
<option value="Asia/Gaza" <?php if ($result['TIMEZONE'] == "Asia/Gaza") {echo "selected='selected'";} ?>>Asia/Gaza</option>
<option value="Asia/Hebron" <?php if ($result['TIMEZONE'] == "Asia/Hebron") {echo "selected='selected'";} ?>>Asia/Hebron</option>
<option value="Asia/Ho_Chi_Minh" <?php if ($result['TIMEZONE'] == "Asia/Ho_Chi_Minh") {echo "selected='selected'";} ?>>Asia/Ho_Chi_Minh</option>
<option value="Asia/Hong_Kong" <?php if ($result['TIMEZONE'] == "Asia/Hong_Kong") {echo "selected='selected'";} ?>>Asia/Hong_Kong</option>
<option value="Asia/Hovd" <?php if ($result['TIMEZONE'] == "Asia/Hovd") {echo "selected='selected'";} ?>>Asia/Hovd</option>
<option value="Asia/Irkutsk" <?php if ($result['TIMEZONE'] == "Asia/Irkutsk") {echo "selected='selected'";} ?>>Asia/Irkutsk</option>
<option value="Asia/Jakarta" <?php if ($result['TIMEZONE'] == "Asia/Jakarta") {echo "selected='selected'";} ?>>Asia/Jakarta</option>
<option value="Asia/Jayapura" <?php if ($result['TIMEZONE'] == "Asia/Jayapura") {echo "selected='selected'";} ?>>Asia/Jayapura</option>
<option value="Asia/Jerusalem" <?php if ($result['TIMEZONE'] == "Asia/Jerusalem") {echo "selected='selected'";} ?>>Asia/Jerusalem</option>
<option value="Asia/Kabul" <?php if ($result['TIMEZONE'] == "Asia/Kabul") {echo "selected='selected'";} ?>>Asia/Kabul</option>
<option value="Asia/Kamchatka" <?php if ($result['TIMEZONE'] == "Asia/Kamchatka") {echo "selected='selected'";} ?>>Asia/Kamchatka</option>
<option value="Asia/Karachi" <?php if ($result['TIMEZONE'] == "Asia/Karachi") {echo "selected='selected'";} ?>>Asia/Karachi</option>
<option value="Asia/Kathmandu" <?php if ($result['TIMEZONE'] == "Asia/Kathmandu") {echo "selected='selected'";} ?>>Asia/Kathmandu</option>
<option value="Asia/Khandyga" <?php if ($result['TIMEZONE'] == "Asia/Khandyga") {echo "selected='selected'";} ?>>Asia/Khandyga</option>
<option value="Asia/Kolkata" <?php if ($result['TIMEZONE'] == "Asia/Kolkata") {echo "selected='selected'";} ?>>Asia/Kolkata</option>
<option value="Asia/Krasnoyarsk" <?php if ($result['TIMEZONE'] == "Asia/Krasnoyarsk") {echo "selected='selected'";} ?>>Asia/Krasnoyarsk</option>
<option value="Asia/Kuala_Lumpur" <?php if ($result['TIMEZONE'] == "Asia/Kuala_Lumpur") {echo "selected='selected'";} ?>>Asia/Kuala_Lumpur</option>
<option value="Asia/Kuching" <?php if ($result['TIMEZONE'] == "Asia/Kuching") {echo "selected='selected'";} ?>>Asia/Kuching</option>
<option value="Asia/Kuwait" <?php if ($result['TIMEZONE'] == "Asia/Kuwait") {echo "selected='selected'";} ?>>Asia/Kuwait</option>
<option value="Asia/Macau" <?php if ($result['TIMEZONE'] == "Asia/Macau") {echo "selected='selected'";} ?>>Asia/Macau</option>
<option value="Asia/Magadan" <?php if ($result['TIMEZONE'] == "Asia/Magadan") {echo "selected='selected'";} ?>>Asia/Magadan</option>
<option value="Asia/Makassar" <?php if ($result['TIMEZONE'] == "Asia/Makassar") {echo "selected='selected'";} ?>>Asia/Makassar</option>
<option value="Asia/Manila" <?php if ($result['TIMEZONE'] == "Asia/Manila") {echo "selected='selected'";} ?>>Asia/Manila</option>
<option value="Asia/Muscat" <?php if ($result['TIMEZONE'] == "Asia/Muscat") {echo "selected='selected'";} ?>>Asia/Muscat</option>
<option value="Asia/Nicosia" <?php if ($result['TIMEZONE'] == "Asia/Nicosia") {echo "selected='selected'";} ?>>Asia/Nicosia</option>
<option value="Asia/Novokuznetsk" <?php if ($result['TIMEZONE'] == "Asia/Novokuznetsk") {echo "selected='selected'";} ?>>Asia/Novokuznetsk</option>
<option value="Asia/Novosibirsk" <?php if ($result['TIMEZONE'] == "Asia/Novosibirsk") {echo "selected='selected'";} ?>>Asia/Novosibirsk</option>
<option value="Asia/Omsk" <?php if ($result['TIMEZONE'] == "Asia/Omsk") {echo "selected='selected'";} ?>>Asia/Omsk</option>
<option value="Asia/Oral" <?php if ($result['TIMEZONE'] == "Asia/Oral") {echo "selected='selected'";} ?>>Asia/Oral</option>
<option value="Asia/Phnom_Penh" <?php if ($result['TIMEZONE'] == "Asia/Phnom_Penh") {echo "selected='selected'";} ?>>Asia/Phnom_Penh</option>
<option value="Asia/Pontianak" <?php if ($result['TIMEZONE'] == "Asia/Pontianak") {echo "selected='selected'";} ?>>Asia/Pontianak</option>
<option value="Asia/Pyongyang" <?php if ($result['TIMEZONE'] == "Asia/Pyongyang") {echo "selected='selected'";} ?>>Asia/Pyongyang</option>
<option value="Asia/Qatar" <?php if ($result['TIMEZONE'] == "Asia/Qatar") {echo "selected='selected'";} ?>>Asia/Qatar</option>
<option value="Asia/Qyzylorda" <?php if ($result['TIMEZONE'] == "Asia/Qyzylorda") {echo "selected='selected'";} ?>>Asia/Qyzylorda</option>
<option value="Asia/Rangoon" <?php if ($result['TIMEZONE'] == "Asia/Rangoon") {echo "selected='selected'";} ?>>Asia/Rangoon</option>
<option value="Asia/Riyadh" <?php if ($result['TIMEZONE'] == "Asia/Riyadh") {echo "selected='selected'";} ?>>Asia/Riyadh</option>
<option value="Asia/Sakhalin" <?php if ($result['TIMEZONE'] == "Asia/Sakhalin") {echo "selected='selected'";} ?>>Asia/Sakhalin</option>
<option value="Asia/Samarkand" <?php if ($result['TIMEZONE'] == "Asia/Samarkand") {echo "selected='selected'";} ?>>Asia/Samarkand</option>
<option value="Asia/Seoul" <?php if ($result['TIMEZONE'] == "Asia/Seoul") {echo "selected='selected'";} ?>>Asia/Seoul</option>
<option value="Asia/Shanghai" <?php if ($result['TIMEZONE'] == "Asia/Shanghai") {echo "selected='selected'";} ?>>Asia/Shanghai</option>
<option value="Asia/Singapore" <?php if ($result['TIMEZONE'] == "Asia/Singapore") {echo "selected='selected'";} ?>>Asia/Singapore</option>
<option value="Asia/Srednekolymsk" <?php if ($result['TIMEZONE'] == "Asia/Srednekolymsk") {echo "selected='selected'";} ?>>Asia/Srednekolymsk</option>
<option value="Asia/Taipei" <?php if ($result['TIMEZONE'] == "Asia/Taipei") {echo "selected='selected'";} ?>>Asia/Taipei</option>
<option value="Asia/Tashkent" <?php if ($result['TIMEZONE'] == "Asia/Tashkent") {echo "selected='selected'";} ?>>Asia/Tashkent</option>
<option value="Asia/Tbilisi" <?php if ($result['TIMEZONE'] == "Asia/Tbilisi") {echo "selected='selected'";} ?>>Asia/Tbilisi</option>
<option value="Asia/Tehran" <?php if ($result['TIMEZONE'] == "Asia/Tehran") {echo "selected='selected'";} ?>>Asia/Tehran</option>
<option value="Asia/Thimphu" <?php if ($result['TIMEZONE'] == "Asia/Thimphu") {echo "selected='selected'";} ?>>Asia/Thimphu</option>
<option value="Asia/Tokyo" <?php if ($result['TIMEZONE'] == "Asia/Tokyo") {echo "selected='selected'";} ?>>Asia/Tokyo</option>
<option value="Asia/Ulaanbaatar" <?php if ($result['TIMEZONE'] == "Asia/Ulaanbaatar") {echo "selected='selected'";} ?>>Asia/Ulaanbaatar</option>
<option value="Asia/Urumqi" <?php if ($result['TIMEZONE'] == "Asia/Urumqi") {echo "selected='selected'";} ?>>Asia/Urumqi</option>
<option value="Asia/Ust-Nera" <?php if ($result['TIMEZONE'] == "Asia/Ust-Nera") {echo "selected='selected'";} ?>>Asia/Ust-Nera</option>
<option value="Asia/Vientiane" <?php if ($result['TIMEZONE'] == "Asia/Vientiane") {echo "selected='selected'";} ?>>Asia/Vientiane</option>
<option value="Asia/Vladivostok" <?php if ($result['TIMEZONE'] == "Asia/Vladivostok") {echo "selected='selected'";} ?>>Asia/Vladivostok</option>
<option value="Asia/Yakutsk" <?php if ($result['TIMEZONE'] == "Asia/Yakutsk") {echo "selected='selected'";} ?>>Asia/Yakutsk</option>
<option value="Asia/Yekaterinburg" <?php if ($result['TIMEZONE'] == "Asia/Yekaterinburg") {echo "selected='selected'";} ?>>Asia/Yekaterinburg</option>
<option value="Asia/Yerevan" <?php if ($result['TIMEZONE'] == "Asia/Yerevan") {echo "selected='selected'";} ?>>Asia/Yerevan</option>
<option value="Atlantic/Azores" <?php if ($result['TIMEZONE'] == "Atlantic/Azores") {echo "selected='selected'";} ?>>Atlantic/Azores</option>
<option value="Atlantic/Bermuda" <?php if ($result['TIMEZONE'] == "Atlantic/Bermuda") {echo "selected='selected'";} ?>>Atlantic/Bermuda</option>
<option value="Atlantic/Canary" <?php if ($result['TIMEZONE'] == "Atlantic/Canary") {echo "selected='selected'";} ?>>Atlantic/Canary</option>
<option value="Atlantic/Cape_Verde" <?php if ($result['TIMEZONE'] == "Atlantic/Cape_Verde") {echo "selected='selected'";} ?>>Atlantic/Cape_Verde</option>
<option value="Atlantic/Faroe" <?php if ($result['TIMEZONE'] == "Atlantic/Faroe") {echo "selected='selected'";} ?>>Atlantic/Faroe</option>
<option value="Atlantic/Madeira" <?php if ($result['TIMEZONE'] == "Atlantic/Madeira") {echo "selected='selected'";} ?>>Atlantic/Madeira</option>
<option value="Atlantic/Reykjavik" <?php if ($result['TIMEZONE'] == "Atlantic/Reykjavik") {echo "selected='selected'";} ?>>Atlantic/Reykjavik</option>
<option value="Atlantic/South_Georgia" <?php if ($result['TIMEZONE'] == "Atlantic/South_Georgia") {echo "selected='selected'";} ?>>Atlantic/South_Georgia</option>
<option value="Atlantic/Stanley" <?php if ($result['TIMEZONE'] == "Atlantic/Stanley") {echo "selected='selected'";} ?>>Atlantic/Stanley</option>
<option value="Atlantic/St_Helena" <?php if ($result['TIMEZONE'] == "Atlantic/St_Helena") {echo "selected='selected'";} ?>>Atlantic/St_Helena</option>
<option value="Australia/Adelaide" <?php if ($result['TIMEZONE'] == "Australia/Adelaide") {echo "selected='selected'";} ?>>Australia/Adelaide</option>
<option value="Australia/Brisbane" <?php if ($result['TIMEZONE'] == "Australia/Brisbane") {echo "selected='selected'";} ?>>Australia/Brisbane</option>
<option value="Australia/Broken_Hill" <?php if ($result['TIMEZONE'] == "Australia/Broken_Hill") {echo "selected='selected'";} ?>>Australia/Broken_Hill</option>
<option value="Australia/Currie" <?php if ($result['TIMEZONE'] == "Australia/Currie") {echo "selected='selected'";} ?>>Australia/Currie</option>
<option value="Australia/Darwin" <?php if ($result['TIMEZONE'] == "Australia/Darwin") {echo "selected='selected'";} ?>>Australia/Darwin</option>
<option value="Australia/Eucla" <?php if ($result['TIMEZONE'] == "Australia/Eucla") {echo "selected='selected'";} ?>>Australia/Eucla</option>
<option value="Australia/Hobart" <?php if ($result['TIMEZONE'] == "Australia/Hobart") {echo "selected='selected'";} ?>>Australia/Hobart</option>
<option value="Australia/Lindeman" <?php if ($result['TIMEZONE'] == "Australia/Lindeman") {echo "selected='selected'";} ?>>Australia/Lindeman</option>
<option value="Australia/Lord_Howe" <?php if ($result['TIMEZONE'] == "Australia/Lord_Howe") {echo "selected='selected'";} ?>>Australia/Lord_Howe</option>
<option value="Australia/Melbourne" <?php if ($result['TIMEZONE'] == "Australia/Melbourne") {echo "selected='selected'";} ?>>Australia/Melbourne</option>
<option value="Australia/Perth" <?php if ($result['TIMEZONE'] == "Australia/Perth") {echo "selected='selected'";} ?>>Australia/Perth</option>
<option value="Australia/Sydney" <?php if ($result['TIMEZONE'] == "Australia/Sydney") {echo "selected='selected'";} ?>>Australia/Sydney</option>
<option value="Europe/Amsterdam" <?php if ($result['TIMEZONE'] == "Europe/Amsterdam") {echo "selected='selected'";} ?>>Europe/Amsterdam</option>
<option value="Europe/Andorra" <?php if ($result['TIMEZONE'] == "Europe/Andorra") {echo "selected='selected'";} ?>>Europe/Andorra</option>
<option value="Europe/Athens" <?php if ($result['TIMEZONE'] == "Europe/Athens") {echo "selected='selected'";} ?>>Europe/Athens</option>
<option value="Europe/Belgrade" <?php if ($result['TIMEZONE'] == "Europe/Belgrade") {echo "selected='selected'";} ?>>Europe/Belgrade</option>
<option value="Europe/Berlin" <?php if ($result['TIMEZONE'] == "Europe/Berlin") {echo "selected='selected'";} ?>>Europe/Berlin</option>
<option value="Europe/Bratislava" <?php if ($result['TIMEZONE'] == "Europe/Bratislava") {echo "selected='selected'";} ?>>Europe/Bratislava</option>
<option value="Europe/Brussels" <?php if ($result['TIMEZONE'] == "Europe/Brussels") {echo "selected='selected'";} ?>>Europe/Brussels</option>
<option value="Europe/Bucharest" <?php if ($result['TIMEZONE'] == "Europe/Bucharest") {echo "selected='selected'";} ?>>Europe/Bucharest</option>
<option value="Europe/Budapest" <?php if ($result['TIMEZONE'] == "Europe/Budapest") {echo "selected='selected'";} ?>>Europe/Budapest</option>
<option value="Europe/Busingen" <?php if ($result['TIMEZONE'] == "Europe/Busingen") {echo "selected='selected'";} ?>>Europe/Busingen</option>
<option value="Europe/Chisinau" <?php if ($result['TIMEZONE'] == "Europe/Chisinau") {echo "selected='selected'";} ?>>Europe/Chisinau</option>
<option value="Europe/Copenhagen" <?php if ($result['TIMEZONE'] == "Europe/Copenhagen") {echo "selected='selected'";} ?>>Europe/Copenhagen</option>
<option value="Europe/Dublin" <?php if ($result['TIMEZONE'] == "Europe/Dublin") {echo "selected='selected'";} ?>>Europe/Dublin</option>
<option value="Europe/Gibraltar" <?php if ($result['TIMEZONE'] == "Europe/Gibraltar") {echo "selected='selected'";} ?>>Europe/Gibraltar</option>
<option value="Europe/Guernsey" <?php if ($result['TIMEZONE'] == "Europe/Guernsey") {echo "selected='selected'";} ?>>Europe/Guernsey</option>
<option value="Europe/Helsinki" <?php if ($result['TIMEZONE'] == "Europe/Helsinki") {echo "selected='selected'";} ?>>Europe/Helsinki</option>
<option value="Europe/Isle_of_Man" <?php if ($result['TIMEZONE'] == "Europe/Isle_of_Man") {echo "selected='selected'";} ?>>Europe/Isle_of_Man</option>
<option value="Europe/Istanbul" <?php if ($result['TIMEZONE'] == "Europe/Istanbul") {echo "selected='selected'";} ?>>Europe/Istanbul</option>
<option value="Europe/Jersey" <?php if ($result['TIMEZONE'] == "Europe/Jersey") {echo "selected='selected'";} ?>>Europe/Jersey</option>
<option value="Europe/Kaliningrad" <?php if ($result['TIMEZONE'] == "Europe/Kaliningrad") {echo "selected='selected'";} ?>>Europe/Kaliningrad</option>
<option value="Europe/Kiev" <?php if ($result['TIMEZONE'] == "Europe/Kiev") {echo "selected='selected'";} ?>>Europe/Kiev</option>
<option value="Europe/Lisbon" <?php if ($result['TIMEZONE'] == "Europe/Lisbon") {echo "selected='selected'";} ?>>Europe/Lisbon</option>
<option value="Europe/Ljubljana" <?php if ($result['TIMEZONE'] == "Europe/Ljubljana") {echo "selected='selected'";} ?>>Europe/Ljubljana</option>
<option value="Europe/London" <?php if ($result['TIMEZONE'] == "Europe/London") {echo "selected='selected'";} ?>>Europe/London</option>
<option value="Europe/Luxembourg" <?php if ($result['TIMEZONE'] == "Europe/Luxembourg") {echo "selected='selected'";} ?>>Europe/Luxembourg</option>
<option value="Europe/Madrid" <?php if ($result['TIMEZONE'] == "Europe/Madrid") {echo "selected='selected'";} ?>>Europe/Madrid</option>
<option value="Europe/Malta" <?php if ($result['TIMEZONE'] == "Europe/Malta") {echo "selected='selected'";} ?>>Europe/Malta</option>
<option value="Europe/Mariehamn" <?php if ($result['TIMEZONE'] == "Europe/Mariehamn") {echo "selected='selected'";} ?>>Europe/Mariehamn</option>
<option value="Europe/Minsk" <?php if ($result['TIMEZONE'] == "Europe/Minsk") {echo "selected='selected'";} ?>>Europe/Minsk</option>
<option value="Europe/Monaco" <?php if ($result['TIMEZONE'] == "Europe/Monaco") {echo "selected='selected'";} ?>>Europe/Monaco</option>
<option value="Europe/Moscow" <?php if ($result['TIMEZONE'] == "Europe/Moscow") {echo "selected='selected'";} ?>>Europe/Moscow</option>
<option value="Europe/Oslo" <?php if ($result['TIMEZONE'] == "Europe/Oslo") {echo "selected='selected'";} ?>>Europe/Oslo</option>
<option value="Europe/Paris" <?php if ($result['TIMEZONE'] == "Europe/Paris") {echo "selected='selected'";} ?>>Europe/Paris</option>
<option value="Europe/Podgorica" <?php if ($result['TIMEZONE'] == "Europe/Podgorica") {echo "selected='selected'";} ?>>Europe/Podgorica</option>
<option value="Europe/Prague" <?php if ($result['TIMEZONE'] == "Europe/Prague") {echo "selected='selected'";} ?>>Europe/Prague</option>
<option value="Europe/Riga" <?php if ($result['TIMEZONE'] == "Europe/Riga") {echo "selected='selected'";} ?>>Europe/Riga</option>
<option value="Europe/Rome" <?php if ($result['TIMEZONE'] == "Europe/Rome") {echo "selected='selected'";} ?>>Europe/Rome</option>
<option value="Europe/Samara" <?php if ($result['TIMEZONE'] == "Europe/Samara") {echo "selected='selected'";} ?>>Europe/Samara</option>
<option value="Europe/San_Marino" <?php if ($result['TIMEZONE'] == "Europe/San_Marino") {echo "selected='selected'";} ?>>Europe/San_Marino</option>
<option value="Europe/Sarajevo" <?php if ($result['TIMEZONE'] == "Europe/Sarajevo") {echo "selected='selected'";} ?>>Europe/Sarajevo</option>
<option value="Europe/Simferopol" <?php if ($result['TIMEZONE'] == "Europe/Simferopol") {echo "selected='selected'";} ?>>Europe/Simferopol</option>
<option value="Europe/Skopje" <?php if ($result['TIMEZONE'] == "Europe/Skopje") {echo "selected='selected'";} ?>>Europe/Skopje</option>
<option value="Europe/Sofia" <?php if ($result['TIMEZONE'] == "Europe/Sofia") {echo "selected='selected'";} ?>>Europe/Sofia</option>
<option value="Europe/Stockholm" <?php if ($result['TIMEZONE'] == "Europe/Stockholm") {echo "selected='selected'";} ?>>Europe/Stockholm</option>
<option value="Europe/Tallinn" <?php if ($result['TIMEZONE'] == "Europe/Tallinn") {echo "selected='selected'";} ?>>Europe/Tallinn</option>
<option value="Europe/Tirane" <?php if ($result['TIMEZONE'] == "Europe/Tirane") {echo "selected='selected'";} ?>>Europe/Tirane</option>
<option value="Europe/Uzhgorod" <?php if ($result['TIMEZONE'] == "Europe/Uzhgorod") {echo "selected='selected'";} ?>>Europe/Uzhgorod</option>
<option value="Europe/Vaduz" <?php if ($result['TIMEZONE'] == "Europe/Vaduz") {echo "selected='selected'";} ?>>Europe/Vaduz</option>
<option value="Europe/Vatican" <?php if ($result['TIMEZONE'] == "Europe/Vatican") {echo "selected='selected'";} ?>>Europe/Vatican</option>
<option value="Europe/Vienna" <?php if ($result['TIMEZONE'] == "Europe/Vienna") {echo "selected='selected'";} ?>>Europe/Vienna</option>
<option value="Europe/Vilnius" <?php if ($result['TIMEZONE'] == "Europe/Vilnius") {echo "selected='selected'";} ?>>Europe/Vilnius</option>
<option value="Europe/Volgograd" <?php if ($result['TIMEZONE'] == "Europe/Volgograd") {echo "selected='selected'";} ?>>Europe/Volgograd</option>
<option value="Europe/Warsaw" <?php if ($result['TIMEZONE'] == "Europe/Warsaw") {echo "selected='selected'";} ?>>Europe/Warsaw</option>
<option value="Europe/Zagreb" <?php if ($result['TIMEZONE'] == "Europe/Zagreb") {echo "selected='selected'";} ?>>Europe/Zagreb</option>
<option value="Europe/Zaporozhye" <?php if ($result['TIMEZONE'] == "Europe/Zaporozhye") {echo "selected='selected'";} ?>>Europe/Zaporozhye</option>
<option value="Europe/Zurich" <?php if ($result['TIMEZONE'] == "Europe/Zurich") {echo "selected='selected'";} ?>>Europe/Zurich</option>
<option value="Indian/Antananarivo" <?php if ($result['TIMEZONE'] == "Indian/Antananarivo") {echo "selected='selected'";} ?>>Indian/Antananarivo</option>
<option value="Indian/Chagos" <?php if ($result['TIMEZONE'] == "Indian/Chagos") {echo "selected='selected'";} ?>>Indian/Chagos</option>
<option value="Indian/Christmas" <?php if ($result['TIMEZONE'] == "Indian/Christmas") {echo "selected='selected'";} ?>>Indian/Christmas</option>
<option value="Indian/Cocos" <?php if ($result['TIMEZONE'] == "Indian/Cocos") {echo "selected='selected'";} ?>>Indian/Cocos</option>
<option value="Indian/Comoro" <?php if ($result['TIMEZONE'] == "Indian/Comoro") {echo "selected='selected'";} ?>>Indian/Comoro</option>
<option value="Indian/Kerguelen" <?php if ($result['TIMEZONE'] == "Indian/Kerguelen") {echo "selected='selected'";} ?>>Indian/Kerguelen</option>
<option value="Indian/Mahe" <?php if ($result['TIMEZONE'] == "Indian/Mahe") {echo "selected='selected'";} ?>>Indian/Mahe</option>
<option value="Indian/Maldives" <?php if ($result['TIMEZONE'] == "Indian/Maldives") {echo "selected='selected'";} ?>>Indian/Maldives</option>
<option value="Indian/Mauritius" <?php if ($result['TIMEZONE'] == "Indian/Mauritius") {echo "selected='selected'";} ?>>Indian/Mauritius</option>
<option value="Indian/Mayotte" <?php if ($result['TIMEZONE'] == "Indian/Mayotte") {echo "selected='selected'";} ?>>Indian/Mayotte</option>
<option value="Indian/Reunion" <?php if ($result['TIMEZONE'] == "Indian/Reunion") {echo "selected='selected'";} ?>>Indian/Reunion</option>
<option value="Pacific/Apia" <?php if ($result['TIMEZONE'] == "Pacific/Apia") {echo "selected='selected'";} ?>>Pacific/Apia</option>
<option value="Pacific/Auckland" <?php if ($result['TIMEZONE'] == "Pacific/Auckland") {echo "selected='selected'";} ?>>Pacific/Auckland</option>
<option value="Pacific/Bougainville" <?php if ($result['TIMEZONE'] == "Pacific/Bougainville") {echo "selected='selected'";} ?>>Pacific/Bougainville</option>
<option value="Pacific/Chatham" <?php if ($result['TIMEZONE'] == "Pacific/Chatham") {echo "selected='selected'";} ?>>Pacific/Chatham</option>
<option value="Pacific/Chuuk" <?php if ($result['TIMEZONE'] == "Pacific/Chuuk") {echo "selected='selected'";} ?>>Pacific/Chuuk</option>
<option value="Pacific/Easter" <?php if ($result['TIMEZONE'] == "Pacific/Easter") {echo "selected='selected'";} ?>>Pacific/Easter</option>
<option value="Pacific/Efate" <?php if ($result['TIMEZONE'] == "Pacific/Efate") {echo "selected='selected'";} ?>>Pacific/Efate</option>
<option value="Pacific/Enderbury" <?php if ($result['TIMEZONE'] == "Pacific/Enderbury") {echo "selected='selected'";} ?>>Pacific/Enderbury</option>
<option value="Pacific/Fakaofo" <?php if ($result['TIMEZONE'] == "Pacific/Fakaofo") {echo "selected='selected'";} ?>>Pacific/Fakaofo</option>
<option value="Pacific/Fiji" <?php if ($result['TIMEZONE'] == "Pacific/Fiji") {echo "selected='selected'";} ?>>Pacific/Fiji</option>
<option value="Pacific/Funafuti" <?php if ($result['TIMEZONE'] == "Pacific/Funafuti") {echo "selected='selected'";} ?>>Pacific/Funafuti</option>
<option value="Pacific/Galapagos" <?php if ($result['TIMEZONE'] == "Pacific/Galapagos") {echo "selected='selected'";} ?>>Pacific/Galapagos</option>
<option value="Pacific/Gambier" <?php if ($result['TIMEZONE'] == "Pacific/Gambier") {echo "selected='selected'";} ?>>Pacific/Gambier</option>
<option value="Pacific/Guadalcanal" <?php if ($result['TIMEZONE'] == "Pacific/Guadalcanal") {echo "selected='selected'";} ?>>Pacific/Guadalcanal</option>
<option value="Pacific/Guam" <?php if ($result['TIMEZONE'] == "Pacific/Guam") {echo "selected='selected'";} ?>>Pacific/Guam</option>
<option value="Pacific/Honolulu" <?php if ($result['TIMEZONE'] == "Pacific/Honolulu") {echo "selected='selected'";} ?>>Pacific/Honolulu</option>
<option value="Pacific/Johnston" <?php if ($result['TIMEZONE'] == "Pacific/Johnston") {echo "selected='selected'";} ?>>Pacific/Johnston</option>
<option value="Pacific/Kiritimati" <?php if ($result['TIMEZONE'] == "Pacific/Kiritimati") {echo "selected='selected'";} ?>>Pacific/Kiritimati</option>
<option value="Pacific/Kosrae" <?php if ($result['TIMEZONE'] == "Pacific/Kosrae") {echo "selected='selected'";} ?>>Pacific/Kosrae</option>
<option value="Pacific/Kwajalein" <?php if ($result['TIMEZONE'] == "Pacific/Kwajalein") {echo "selected='selected'";} ?>>Pacific/Kwajalein</option>
<option value="Pacific/Majuro" <?php if ($result['TIMEZONE'] == "Pacific/Majuro") {echo "selected='selected'";} ?>>Pacific/Majuro</option>
<option value="Pacific/Marquesas" <?php if ($result['TIMEZONE'] == "Pacific/Marquesas") {echo "selected='selected'";} ?>>Pacific/Marquesas</option>
<option value="Pacific/Midway" <?php if ($result['TIMEZONE'] == "Pacific/Midway") {echo "selected='selected'";} ?>>Pacific/Midway</option>
<option value="Pacific/Nauru" <?php if ($result['TIMEZONE'] == "Pacific/Nauru") {echo "selected='selected'";} ?>>Pacific/Nauru</option>
<option value="Pacific/Niue" <?php if ($result['TIMEZONE'] == "Pacific/Niue") {echo "selected='selected'";} ?>>Pacific/Niue</option>
<option value="Pacific/Norfolk" <?php if ($result['TIMEZONE'] == "Pacific/Norfolk") {echo "selected='selected'";} ?>>Pacific/Norfolk</option>
<option value="Pacific/Noumea" <?php if ($result['TIMEZONE'] == "Pacific/Noumea") {echo "selected='selected'";} ?>>Pacific/Noumea</option>
<option value="Pacific/Pago_Pago" <?php if ($result['TIMEZONE'] == "Pacific/Pago_Pago") {echo "selected='selected'";} ?>>Pacific/Pago_Pago</option>
<option value="Pacific/Palau" <?php if ($result['TIMEZONE'] == "Pacific/Palau") {echo "selected='selected'";} ?>>Pacific/Palau</option>
<option value="Pacific/Pitcairn" <?php if ($result['TIMEZONE'] == "Pacific/Pitcairn") {echo "selected='selected'";} ?>>Pacific/Pitcairn</option>
<option value="Pacific/Pohnpei" <?php if ($result['TIMEZONE'] == "Pacific/Pohnpei") {echo "selected='selected'";} ?>>Pacific/Pohnpei</option>
<option value="Pacific/Port_Moresby" <?php if ($result['TIMEZONE'] == "Pacific/Port_Moresby") {echo "selected='selected'";} ?>>Pacific/Port_Moresby</option>
<option value="Pacific/Rarotonga" <?php if ($result['TIMEZONE'] == "Pacific/Rarotonga") {echo "selected='selected'";} ?>>Pacific/Rarotonga</option>
<option value="Pacific/Saipan" <?php if ($result['TIMEZONE'] == "Pacific/Saipan") {echo "selected='selected'";} ?>>Pacific/Saipan</option>
<option value="Pacific/Tahiti" <?php if ($result['TIMEZONE'] == "Pacific/Tahiti") {echo "selected='selected'";} ?>>Pacific/Tahiti</option>
<option value="Pacific/Tarawa" <?php if ($result['TIMEZONE'] == "Pacific/Tarawa") {echo "selected='selected'";} ?>>Pacific/Tarawa</option>
<option value="Pacific/Tongatapu" <?php if ($result['TIMEZONE'] == "Pacific/Tongatapu") {echo "selected='selected'";} ?>>Pacific/Tongatapu</option>
<option value="Pacific/Wake" <?php if ($result['TIMEZONE'] == "Pacific/Wake") {echo "selected='selected'";} ?>>Pacific/Wake</option>
<option value="Pacific/Wallis" <?php if ($result['TIMEZONE'] == "Pacific/Wallis") {echo "selected='selected'";} ?>>Pacific/Wallis</option>
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
                                        <td><input type="text" name="START_DATE" value="<?PHP echo $result['START_DATE'];?>" onchange="this.form.submit()"></td> 
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
                            

                                    echo 'Username <input type="text" name="HT_USERNAME" onchange="this.form.submit()" value="'; echo $result['HT_USERNAME']; echo '"><BR>';
                                    echo 'Password <input type="password" name="HT_PASSWORD" onchange="this.form.submit()" value="'; echo $result['HT_PASSWORD']; echo '"><BR>';    
                                    echo 'URL <BR><input type="text" name="HT_URL" onchange="this.form.submit()" value="'; echo $result['HT_URL']; echo '"><BR>';    
                                }
                                else {
                                    echo '<input type="hidden" name="HT_USERNAME" value="'; echo $result['HT_USERNAME']; echo '"><BR>';
                                    echo '<input type="hidden" name="HT_PASSWORD" value="'; echo $result['HT_PASSWORD']; echo '">';    
                                    echo '<input type="hidden" name="HT_URL" value="'; echo $result['HT_URL']; echo '">';    

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
