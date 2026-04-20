<?PHP

/*
usage on webpage:
<img src="stream.php">
*/ 
$server = "localhost"; // camera server address
$port = 8080; // camera server port
$url = "/?action=stream"; // image url on server
set_time_limit(0);
$fp = fsockopen($server, $port, $errno, $errstr, 30);
if (!$fp) {
        echo "$errstr ($errno)<br>\n";   // error handling
} else {
        // SECURITY FIX: Whitelist of allowed headers to prevent header injection
        $allowed_headers = array(
            'content-type',
            'content-length',
            'cache-control',
            'pragma',
            'expires',
            'last-modified',
            'etag',
            'accept-ranges',
            'content-disposition'
        );
        $urlstring = "GET ".$url." HTTP/1.0\r\n\r\n";
        fputs ($fp, $urlstring);
        while ($str = trim(fgets($fp, 4096))) {
            // Only pass through whitelisted headers, sanitize values
            $header_parts = explode(':', $str, 2);
            if (count($header_parts) == 2) {
                $header_name = strtolower(trim($header_parts[0]));
                $header_value = trim($header_parts[1]);
                // Remove any newlines/carriage returns to prevent header injection
                $header_value = str_replace(array("\r", "\n"), '', $header_value);
                if (in_array($header_name, $allowed_headers)) {
                    header($header_parts[0] . ': ' . $header_value);
                }
            }
        }
        fpassthru($fp);
        fclose($fp);
} 
?>

