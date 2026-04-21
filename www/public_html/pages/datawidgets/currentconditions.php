<?php

// Get Hive Data First
$alldatasth = $conn->prepare("SELECT hiveweight, hiverawweight, hivetempf, hivetempc, hiveHum, weather_tempf, weather_tempc, weather_humidity, precip_1hr_in, solarradiation, lux, date AS datetime FROM allhivedata ORDER BY datetime(\"date\") DESC LIMIT 1");
$alldatasth->execute();
$alldata = $alldatasth->fetch(PDO::FETCH_ASSOC);

$sth1 = $conn->prepare("SELECT seasongdd, daygdd FROM gdd ORDER BY datetime(\"calcdate\") DESC LIMIT 1;");
$sth1->execute();
$gdddata = $sth1->fetch(PDO::FETCH_ASSOC);

$pollensth = $conn->prepare("SELECT pollenlevel, pollentypes FROM pollen ORDER BY date DESC LIMIT 1;");
$pollensth->execute();
$pollendata = $pollensth->fetch(PDO::FETCH_ASSOC);

// Check if we have data, if not initialize with defaults
if (!$alldata) {
	$alldata = [
		'hiveweight' => 0,
		'hiverawweight' => 0,
		'hivetempf' => 0,
		'hivetempc' => 0,
		'hiveHum' => 0,
		'weather_tempf' => 0,
		'weather_tempc' => 0,
		'weather_humidity' => 0,
		'datetime' => date('Y-m-d H:i:s')
	];
}

if (!$gdddata) {
	$gdddata = [
		'seasongdd' => 0,
		'daygdd' => 0
	];
}

if (!$pollendata) {
	$pollendata = [
		'pollenlevel' => 0,
		'pollentypes' => 'None'
	];
}
$pollenlevel = $pollendata['pollenlevel'];
$pollentypes = $pollendata['pollentypes'];

if ($pollenlevel <= 0) { $pollen_label = 'None'; $pollen_card_class = 'green'; }
elseif ($pollenlevel <= 4) { $pollen_label = 'Low'; $pollen_card_class = 'green'; }
elseif ($pollenlevel <= 8) { $pollen_label = 'Moderate'; $pollen_card_class = 'hiveyellow'; }
else { $pollen_label = 'High'; $pollen_card_class = 'red'; }

#$getdate = $alldata['datetime'];
#echo $getdate;
$recorddate = date("(M d, H:i)", strtotime($alldata['datetime']));
#$recorddate = date("Y-m-d H:i", );

if ($chart_rounding == "on") {
	if ($SHOW_METRIC == "on") {
		# Show Metric or do calcs
		# Get Hivetempc and pass it via hivetempf
		$hivetempf = ceil($alldata['hivetempc']);
		$wxtempf = ceil($alldata['weather_tempc']);
		$imperialweight = $alldata['hiveweight'];
		$hiveweight = round(($imperialweight * 0.453592),1);
		$imperialraw = $alldata['hiverawweight'];
		$rawweight = round(($imperialraw * 0.453592),1);

	} else {
		#Show Imperial
		$hivetempf = ceil($alldata['hivetempf']);
		$hiveweight = ceil($alldata['hiveweight']);
		$wxtempf = ceil($alldata['weather_tempf']);
		$rawweight = ceil($alldata['hiverawweight']);
	}

	$hivehumi = ceil($alldata['hiveHum']);
	$wxhumi = ceil($alldata['weather_humidity']);
	$datetime = ceil($alldata['datetime']);
	$daygdd = ceil($gdddata['daygdd']);
	$seasongdd = ceil($gdddata['seasongdd']);
}
else {
	if ($SHOW_METRIC == "on") {
		# Show Metric or do calcs
		# Get Hivetempc and pass it via hivetempf
		$hivetempf = $alldata['hivetempc'];
		$wxtempf = $alldata['weather_tempc'];
		$imperialweight = $alldata['hiveweight'];
		$hiveweight = round(($imperialweight * 0.453592),2);
		$imperialraw = $alldata['hiverawweight'];
		$rawweight = round(($imperialraw * 0.453592),2);

	} else {
		#Show Imperial
		$hivetempf = $alldata['hivetempf'];
		$hiveweight = $alldata['hiveweight'];
		$wxtempf = $alldata['weather_tempf'];
		$rawweight = $alldata['hiverawweight'];
	}

	$hivehumi = $alldata['hiveHum'];
	$wxhumi = $alldata['weather_humidity'];
	$datetime = $alldata['datetime'];
	$daygdd = $gdddata['daygdd'];
	$seasongdd = $gdddata['seasongdd'];
}

#Get some trends

#WHERE date > "2016-04-09 15:55" AND date < "2016-04-09 16:00:";

#Get Current Date
# Parse to get the actual date

#$now = date('Y-m-d H:i');
#$minute = date('i');
#$startdate = strtotime("-1440 minutes");

 $shortName = exec('date +%Z');
 $longName = timezone_name_from_abbr($shortName);
 date_default_timezone_set($longName);

 switch ($period) {
    case "today":
		$startdate = date("Y-m-d 00:00");
        break;
    case "day":
    	$s = strtotime("-1440 minutes");
		$startdate = date("Y-m-d H:i", $s);
        break;
    case "week":
        $s = strtotime("-10080 minutes");
		$startdate = date("Y-m-d H:i", $s);
        break;
    case "month":
        $s = strtotime("-43200 minutes");
        $startdate = date("Y-m-d H:i", $s);
		break;
    case "year":
        $s = strtotime("-525600 minutes");
        $startdate = date("Y-m-d H:i", $s);
		break;
    case "all":
        $sqlperiod =  "-20 years";
        break;
    } 

#WHERE date > datetime('now', 'localtime', '$sqlperiod')");

$trendsth = $conn->prepare("SELECT hiveweight, hiverawweight, date AS datetime FROM allhivedata WHERE date > '$startdate' ORDER BY date LIMIT 1;");
$trendsth->execute();
$trenddata = $trendsth->fetch(PDO::FETCH_ASSOC);

$pasthiveweight = $trenddata['hiveweight'];
#$pastrawweight = $trenddata['hiverawweight'];

if ($SHOW_METRIC == "on") {
$pasthiveweight = round(($pasthiveweight * 0.453592),2);
}

if (empty($pasthiveweight) || !is_numeric($pasthiveweight) || !is_numeric($hiveweight)) {

	$changeweight = "NA";
}
else {
	$changeweight = round(($hiveweight - $pasthiveweight), 2);
}

#echo "<br>starting weight = $pasthiveweight";

// Compute 1-hour trend deltas for metric cards
$trend_sth = $conn->prepare("SELECT hivetempf, hivetempc, hiveHum, weather_tempf, weather_tempc, weather_humidity FROM allhivedata WHERE date <= datetime('now', 'localtime', '-1 hours') ORDER BY datetime(date) DESC LIMIT 1");
$trend_sth->execute();
$trend_1hr = $trend_sth->fetch(PDO::FETCH_ASSOC);

$temp_delta = '';
$hum_delta = '';
$temp_card_class = 'green';
$weight_card_class = 'hiveyellow';
$hum_card_class = 'primary';

if ($trend_1hr) {
    if ($SHOW_METRIC == "on") {
        $past_temp = floatval($trend_1hr['hivetempc']);
        $cur_temp_val = floatval($alldata['hivetempc']);
    } else {
        $past_temp = floatval($trend_1hr['hivetempf']);
        $cur_temp_val = floatval($alldata['hivetempf']);
    }

    if (is_numeric($past_temp) && $past_temp != 0 && is_numeric($cur_temp_val) && $cur_temp_val != 0) {
        $tdiff = round($cur_temp_val - $past_temp, 1);
        if ($tdiff > 0) {
            $temp_delta = "&#9650; +{$tdiff}";
        } elseif ($tdiff < 0) {
            $temp_delta = "&#9660; {$tdiff}";
        } else {
            $temp_delta = "&#8212; 0";
        }
    }

    $past_hum = floatval($trend_1hr['hiveHum']);
    $cur_hum_val = floatval($alldata['hiveHum']);
    if (is_numeric($past_hum) && $past_hum != 0 && is_numeric($cur_hum_val) && $cur_hum_val != 0) {
        $hdiff = round($cur_hum_val - $past_hum, 1);
        if ($hdiff > 0) {
            $hum_delta = "&#9650; +{$hdiff}%";
        } elseif ($hdiff < 0) {
            $hum_delta = "&#9660; {$hdiff}%";
        } else {
            $hum_delta = "&#8212; 0%";
        }
    }
}

// Alert-aware card coloring
include_once($_SERVER["DOCUMENT_ROOT"] . "/include/alert_engine.php");
$card_alerts = get_active_alerts($conn);
foreach ($card_alerts as $ca) {
    if ($ca['type'] === 'high_temp' || $ca['type'] === 'low_temp') {
        $temp_card_class = ($ca['severity'] === 'danger') ? 'red' : 'yellow';
    }
    if ($ca['type'] === 'robbing' || $ca['type'] === 'swarm') {
        $weight_card_class = 'red';
    }
    if ($ca['type'] === 'honey_flow') {
        $weight_card_class = 'green';
    }
}

// Weight trend text
$weight_trend_text = '';
foreach ($card_alerts as $ca) {
    if ($ca['type'] === 'robbing') {
        $weight_trend_text = 'Rapid loss';
    } elseif ($ca['type'] === 'swarm') {
        $weight_trend_text = 'Possible swarm';
    } elseif ($ca['type'] === 'honey_flow') {
        $weight_trend_text = 'Honey flow';
    }
}


?>