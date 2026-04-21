<?php

include_once($_SERVER["DOCUMENT_ROOT"] . "/include/alert_engine.php");

$active_alerts = get_active_alerts($conn);

if (!empty($active_alerts)) {
    foreach ($active_alerts as $alert) {
        $bs_class = 'alert-info';
        switch ($alert['severity']) {
            case 'danger':  $bs_class = 'alert-danger'; break;
            case 'warning': $bs_class = 'alert-warning'; break;
            case 'success': $bs_class = 'alert-success'; break;
            case 'info':    $bs_class = 'alert-info'; break;
        }
        echo '<div class="alert ' . $bs_class . ' alert-dismissible" role="alert">';
        echo '<button type="button" class="close" data-dismiss="alert" aria-label="Close"><span aria-hidden="true">&times;</span></button>';
        echo '<strong><i class="fa ' . htmlspecialchars($alert['icon']) . '"></i> ' . htmlspecialchars($alert['title']) . '</strong> &mdash; ';
        echo htmlspecialchars($alert['message']);
        echo '</div>';
    }
}

?>
