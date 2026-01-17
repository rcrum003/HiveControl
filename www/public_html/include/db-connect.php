<?php

try {
    $conn = new PDO("sqlite:/home/HiveControl/data/hive-data.db");
    // set the PDO error mode to exception
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    //echo "Connected successfully";
    }

catch(PDOException $e)
    { // SECURITY FIX: Don't expose detailed database error messages to users
    error_log("Database connection failed: " . $e->getMessage());
    die("Database connection error. Please contact administrator.");
    }
    ?>


