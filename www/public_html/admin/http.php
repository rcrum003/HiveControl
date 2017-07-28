<?PHP

require $_SERVER["DOCUMENT_ROOT"] . '/vendor/autoload.php';

use GuzzleHttp\Psr7;
use GuzzleHttp\Exception\ClientException;

use GuzzleHttp\Client;
$httpclient = new Client([
    // Base URI is used with relative requests
    'base_uri' => 'http://api.110uni.com',
    // You can set any number of default request options.
    'timeout'  => 2.0,
]);

?>