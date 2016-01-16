<?php

try {
    $conn = new PDO("sqlite:/home/hivetool2/data/hive-data.db");
    //$conn = new PDO("sqlite:/Users/rcrum/Documents/Personal/Dev/HiveControl/HiveControl/data/hive-data.db");
    // set the PDO error mode to exception
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    //echo "Connected successfully"; 
    }

catch(PDOException $e)
    { // Always have to catch when making DB connections, so we can sanitize the message for security's sake!
    echo "Connection failed: " . $e->getMessage();
    }
    ?>


