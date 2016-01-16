<?php
// Gets data on current conditions from the JSON file
// copy file content into a string var
$someJSON = file_get_contents('../data/current.json');
$someObject = json_decode($someJSON);
// Set variable to use below
$id = $someObject[0]->current_conditions->hive->id; // Access Object data
$datetime = $someObject[0]->current_conditions->hive->observation_time;
$hivetempf = $someObject[0]->current_conditions->hive->temp_f;
$hivehumi = $someObject[0]->current_conditions->hive->relative_humidity;
$hiveweight = $someObject[0]->current_conditions->hive->weight_lbs;
$wxtempf = $someObject[0]->current_conditions->weather->a_temp_f;
$wxhumi = $someObject[0]->current_conditions->weather->a_relative_humidity;
$wxdewf = $someObject[0]->current_conditions->weather->a_dewpoint_f;
$rawweight = $someObject[0]->current_conditions->hive->rawweight;
$city = $someObject[0]->current_conditions->observation_location->city; // Access Object data
$state = $someObject[0]->current_conditions->observation_location->state; // Access Object data

$daygdd = file_get_contents('../data/todaygdd.txt');
$seasongdd = file_get_contents('../data/seasongdd.txt');


?>