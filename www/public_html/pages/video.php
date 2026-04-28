
<!DOCTYPE html>
<html lang="en">

    <!-- Header and Navigation -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/navigation.php"); ?>
    <!-- /Navigation -->

<?php
$cam_cfg = $conn->prepare("SELECT CAMERATYPE, CAMERA_RTSP_URL FROM hiveconfig WHERE id=1");
$cam_cfg->execute();
$cam = $cam_cfg->fetch(PDO::FETCH_ASSOC);
$is_rtsp = (($cam['CAMERATYPE'] ?? '') === 'RTSP' && !empty($cam['CAMERA_RTSP_URL']));

$env = $conn->prepare("
    SELECT weather_tempf, weather_tempc, weather_humidity,
           weather_dewf, weather_dewc,
           wind_dir, wind_mph, wind_kph,
           wind_gust_mph, wind_gust_kph,
           pressure_mb, pressure_in, pressure_trend,
           solarradiation, lux,
           precip_1hr_in, precip_1hr_metric,
           precip_today_in, precip_today_metric,
           hivetempf, hivetempc, hiveHum, hiveweight,
           date AS datetime
    FROM allhivedata ORDER BY datetime(\"date\") DESC LIMIT 1
");
$env->execute();
$wx = $env->fetch(PDO::FETCH_ASSOC);

if (!$wx) {
    $wx = array_fill_keys([
        'weather_tempf','weather_tempc','weather_humidity',
        'weather_dewf','weather_dewc',
        'wind_dir','wind_mph','wind_kph','wind_gust_mph','wind_gust_kph',
        'pressure_mb','pressure_in','pressure_trend',
        'solarradiation','lux',
        'precip_1hr_in','precip_1hr_metric',
        'precip_today_in','precip_today_metric',
        'hivetempf','hivetempc','hiveHum','hiveweight',
        'datetime'
    ], null);
}

$wx_time = $wx['datetime'] ? date("M j, g:i A", strtotime($wx['datetime'])) : 'No data';

if ($SHOW_METRIC == "on") {
    $wx_temp     = $wx['weather_tempc'];
    $wx_dew      = $wx['weather_dewc'];
    $hive_temp   = $wx['hivetempc'];
    $temp_unit   = '&deg;C';
    $wind_speed  = $wx['wind_kph'];
    $wind_gust   = $wx['wind_gust_kph'];
    $wind_unit   = 'km/h';
    $pressure    = $wx['pressure_mb'];
    $press_unit  = 'mb';
    $precip_1hr  = $wx['precip_1hr_metric'];
    $precip_day  = $wx['precip_today_metric'];
    $precip_unit = 'mm';
} else {
    $wx_temp     = $wx['weather_tempf'];
    $wx_dew      = $wx['weather_dewf'];
    $hive_temp   = $wx['hivetempf'];
    $temp_unit   = '&deg;F';
    $wind_speed  = $wx['wind_mph'];
    $wind_gust   = $wx['wind_gust_mph'];
    $wind_unit   = 'mph';
    $pressure    = $wx['pressure_in'];
    $press_unit  = 'inHg';
    $precip_1hr  = $wx['precip_1hr_in'];
    $precip_day  = $wx['precip_today_in'];
    $precip_unit = 'in';
}

$wx_humidity  = $wx['weather_humidity'];
$hive_hum     = $wx['hiveHum'];
$wind_dir     = $wx['wind_dir'];
$press_trend  = $wx['pressure_trend'];
$solar        = $wx['solarradiation'];
$lux_val      = $wx['lux'];

function fmt($val, $suffix = '') {
    if ($val === null || $val === '') return '<span class="text-muted">--</span>';
    return htmlspecialchars($val) . $suffix;
}
?>

    <div id="wrapper">

            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Hive Video Stream</h1>
            <?PHP if(isset($error)){
                echo '<div class="alert alert-danger">'; echo $error; echo'</div>';} ?>
                </div>
            </div>

            <div class="row">
                <div class="col-lg-8">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            <i class="fa fa-video-camera fa-fw"></i> Live Stream
                        </div>
                        <div class="panel-body">
                            <?php if ($is_rtsp): ?>
                            <video id="rtsp-player" controls autoplay muted playsinline style="width:100%; max-width:960px; background:#000;"></video>
                            <div id="stream-error" class="alert alert-warning" style="display:none;">
                                Stream not available. Make sure the RTSP stream service is running:<br>
                                <code>sudo /etc/init.d/rtsp_stream start</code>
                            </div>
                            <p class="text-muted small" style="margin-top:8px;">
                                <i class="fa fa-plug fa-fw"></i>
                                VLC direct connect: <code><?php echo htmlspecialchars($cam['CAMERA_RTSP_URL']); ?></code>
                            </p>
                            <?php else: ?>
                            <img id="livestream" src="/pages/videostream.php" style="width:100%; max-width:640px;" />
                            <div id="stream-error" class="alert alert-warning" style="display:none;">
                                Stream not available. Make sure the livestream service is running:<br>
                                <code>sudo /etc/init.d/livestream start</code>
                            </div>
                            <p class="text-muted small" style="margin-top:8px;">
                                <i class="fa fa-plug fa-fw"></i>
                                VLC direct connect: <code>http://<?php echo htmlspecialchars($_SERVER['HTTP_HOST']); ?>:8080/?action=stream</code>
                            </p>
                            <?php endif; ?>
                        </div>
                    </div>
                </div>

                <div class="col-lg-4">
                    <!-- Current Conditions -->
                    <div class="panel panel-green">
                        <div class="panel-heading">
                            <i class="fa fa-cloud fa-fw"></i> Current Conditions
                            <span class="pull-right small"><?php echo $wx_time; ?></span>
                        </div>
                        <div class="panel-body" style="padding:0;">
                            <table class="table table-striped" style="margin-bottom:0;">
                                <tbody>
                                    <tr>
                                        <td><i class="fa fa-sun-o fa-fw text-warning"></i> Temperature</td>
                                        <td class="text-right"><strong><?php echo fmt($wx_temp, " $temp_unit"); ?></strong></td>
                                    </tr>
                                    <tr>
                                        <td><i class="fa fa-tint fa-fw text-info"></i> Dew Point</td>
                                        <td class="text-right"><?php echo fmt($wx_dew, " $temp_unit"); ?></td>
                                    </tr>
                                    <tr>
                                        <td><i class="fa fa-tint fa-fw text-primary"></i> Humidity</td>
                                        <td class="text-right"><?php echo fmt($wx_humidity, '%'); ?></td>
                                    </tr>
                                    <?php if ($wind_speed !== null && $wind_speed !== ''): ?>
                                    <tr>
                                        <td><i class="fa fa-flag fa-fw"></i> Wind</td>
                                        <td class="text-right">
                                            <?php echo fmt($wind_dir); ?>
                                            <?php echo fmt($wind_speed, " $wind_unit"); ?>
                                            <?php if ($wind_gust !== null && $wind_gust !== '' && floatval($wind_gust) > 0): ?>
                                                <br><small class="text-muted">Gusts <?php echo fmt($wind_gust, " $wind_unit"); ?></small>
                                            <?php endif; ?>
                                        </td>
                                    </tr>
                                    <?php endif; ?>
                                    <?php if ($pressure !== null && $pressure !== ''): ?>
                                    <tr>
                                        <td><i class="fa fa-dashboard fa-fw"></i> Pressure</td>
                                        <td class="text-right">
                                            <?php echo fmt($pressure, " $press_unit"); ?>
                                            <?php if ($press_trend): ?>
                                                <br><small class="text-muted"><?php echo htmlspecialchars($press_trend); ?></small>
                                            <?php endif; ?>
                                        </td>
                                    </tr>
                                    <?php endif; ?>
                                    <?php if (($precip_1hr !== null && $precip_1hr !== '' && floatval($precip_1hr) > 0) ||
                                              ($precip_day !== null && $precip_day !== '' && floatval($precip_day) > 0)): ?>
                                    <tr>
                                        <td><i class="fa fa-umbrella fa-fw text-info"></i> Precipitation</td>
                                        <td class="text-right">
                                            <?php if (floatval($precip_1hr) > 0): ?>
                                                <?php echo fmt($precip_1hr, " $precip_unit"); ?> <small class="text-muted">/hr</small><br>
                                            <?php endif; ?>
                                            <?php if (floatval($precip_day) > 0): ?>
                                                <?php echo fmt($precip_day, " $precip_unit"); ?> <small class="text-muted">today</small>
                                            <?php endif; ?>
                                        </td>
                                    </tr>
                                    <?php endif; ?>
                                    <?php if (($solar !== null && $solar !== '' && floatval($solar) > 0) ||
                                              ($lux_val !== null && $lux_val !== '' && floatval($lux_val) > 0)): ?>
                                    <tr>
                                        <td><i class="fa fa-bolt fa-fw text-warning"></i> Light</td>
                                        <td class="text-right">
                                            <?php if (floatval($solar) > 0): ?>
                                                <?php echo fmt($solar); ?> <small class="text-muted">W/m&sup2;</small><br>
                                            <?php endif; ?>
                                            <?php if (floatval($lux_val) > 0): ?>
                                                <?php echo fmt($lux_val); ?> <small class="text-muted">lux</small>
                                            <?php endif; ?>
                                        </td>
                                    </tr>
                                    <?php endif; ?>
                                </tbody>
                            </table>
                        </div>
                    </div>

                    <!-- Hive Conditions -->
                    <div class="panel panel-hiveyellow">
                        <div class="panel-heading">
                            <i class="fa fa-fire fa-fw"></i> Hive Conditions
                        </div>
                        <div class="panel-body" style="padding:0;">
                            <table class="table table-striped" style="margin-bottom:0;">
                                <tbody>
                                    <tr>
                                        <td><i class="fa fa-fire fa-fw text-danger"></i> Hive Temp</td>
                                        <td class="text-right"><strong><?php echo fmt($hive_temp, " $temp_unit"); ?></strong></td>
                                    </tr>
                                    <tr>
                                        <td><i class="fa fa-tint fa-fw text-primary"></i> Hive Humidity</td>
                                        <td class="text-right"><?php echo fmt($hive_hum, '%'); ?></td>
                                    </tr>
                                    <?php if ($wx['hiveweight'] !== null && $wx['hiveweight'] !== ''): ?>
                                    <tr>
                                        <td><i class="fa fa-dashboard fa-fw"></i> Hive Weight</td>
                                        <td class="text-right">
                                        <?php
                                            $w = floatval($wx['hiveweight']);
                                            if ($SHOW_METRIC == "on") {
                                                echo fmt(round($w * 0.453592, 1), ' kg');
                                            } else {
                                                echo fmt(round($w, 1), ' lbs');
                                            }
                                        ?>
                                        </td>
                                    </tr>
                                    <?php endif; ?>
                                </tbody>
                            </table>
                        </div>
                    </div>
                </div>
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

    <script src="../dist/js/sb-admin-2.js"></script>

    <?php if ($is_rtsp): ?>
    <script src="/js/hls.min.js"></script>
    <script>
    (function() {
        var video = document.getElementById('rtsp-player');
        var errEl = document.getElementById('stream-error');
        var src = '/stream/live.m3u8';

        function showError() {
            video.style.display = 'none';
            errEl.style.display = 'block';
        }

        if (Hls.isSupported()) {
            var hls = new Hls({ liveSyncDurationCount: 2, liveMaxLatencyDurationCount: 4 });
            hls.loadSource(src);
            hls.attachMedia(video);
            hls.on(Hls.Events.ERROR, function(event, data) {
                if (data.fatal) showError();
            });
        } else if (video.canPlayType('application/vnd.apple.mpegurl')) {
            video.src = src;
            video.addEventListener('error', showError);
        } else {
            showError();
        }
    })();
    </script>
    <?php else: ?>
    <script>
    document.getElementById('livestream').onerror = function() {
        this.style.display = 'none';
        document.getElementById('stream-error').style.display = 'block';
    };
    </script>
    <?php endif; ?>

    <!-- Custom Theme JavaScript -->
<!-- Footer -->
         <?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/footer.php"); ?>
</body>

</html>
