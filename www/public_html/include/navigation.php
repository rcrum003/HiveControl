 <!-- Pull Config -->

<?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

#Get Site Orient Option
$sth3 = $conn->prepare("SELECT SITE_ORIENT,HIVENAME,SITE_TYPE FROM hiveconfig");
$sth3->execute();
$result3 = $sth3->fetch(PDO::FETCH_ASSOC);


#foreach($result as $r)"$r['precip_1hr_in']

$orient = $result3['SITE_ORIENT'];
$id = $result3['HIVENAME'];
$SITE_TYPE = $result3['SITE_TYPE'];

echo '<nav class="navbar navbar-default navbar-static-top" role="navigation" style="margin-bottom: 0">
            <div class="navbar-header">
                <button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-collapse">
                    <span class="sr-only">Toggle navigation</span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                </button>
                <h4 class="text-muted"><img src="/images/hive2.png"> Hive Control - ';  echo "$id"; echo '</h4>
            </div>
            <!-- /.navbar-header -->

            <!-- /.navbar-top-links -->';

#<!-- Wide Orient -->

if ($orient == "wide") {

echo '
            <ul class="nav navbar-top-links navbar-right">
                         <li>
                    <a href="index.php"><i class="fa fa-dashboard fa-fw"></i> Dashboard</a>
                </li>
                    
                <li class="dropdown">
                    <a class="dropdown-toggle" data-toggle="dropdown" href="#">
                        <i class="fa fa-bar-chart-o fa-fw"></i> Detail <i class="fa fa-caret-down"> </i>
                    </a>
                    <ul class="dropdown-menu">
                        <li><a href="/pages/temp.php?chart=line&period=week">Temp/Humidity</a>
                        </li>
                        <li><a href="/pages/weight.php?chart=line&period=week">Weight</a>
                        </li>
                        <li>
                            <a href="/pages/light.php?chart=line&period=week">Light</a>
                        </li>
                            <li>
                            <a href="/pages/gdd.php?chart=line&period=week">GDD</a>
                        </li>
                    </ul>
                    <!-- /.dropdown-user -->
                </li>
            
                 <li>
                    <a href="environment.php"><i class="fa fa-table fa-fw"></i>Weather</a>
                </li>
                    <li>
                            <a href="video.php"><i class="fa fa-video-camera fa-fw"></i>Camera</a>
                        </li>
                <li class="dropdown">
                    <a class="dropdown-toggle" data-toggle="dropdown" href="#">
                        <i class="fa fa-gear fa-fw"></i> Settings <i class="fa fa-caret-down"></i>
                    </a>
                    <ul class="dropdown-menu">
                        <li>
                                    <a href="/pages/admin/hiveconfig.php">Basic Config</a>
                                </li>
                                <li>
                                    <a href="/pages/admin/instrumentconfig.php">Instruments</a>
                                </li>
                                <li>
                                    <a href="/pages/admin/hivebodyconfig.php">Hive Body</a>
                                </li>
                    </ul>
                    <!-- /.dropdown-user -->
                </li>
            </ul>
            <!-- /.navbar-top-links -->

<div class="well">';
}


if ($orient == "normal") {

#<!-- Normal Orient -->
 
 echo '
            <div class="navbar-default sidebar" role="navigation">
                <div class="sidebar-nav navbar-collapse">
                    <ul class="nav" id="side-menu">
                        <li>
                            <a href="index.php"><i class="fa fa-dashboard fa-fw"></i> Dashboard</a>
                        </li>
                        <li>
                            <a href="#"><i class="fa fa-bar-chart-o fa-fw"></i>Data Analysis<span class="fa arrow"></span></a>
                            <ul class="nav nav-second-level">
                                <li>
                                    <a href="/pages/temp.php?chart=line&period=week">Temp/Humidity</a>
                                </li>
                                <li>
                                    <a href="/pages/weight.php?chart=line&period=week">Weight</a>
                                </li>
                                <li>
                                    <a href="/pages/light.php?chart=line&period=week">Light</a>
                                </li>
                                <li>
                                    <a href="/pages/gdd.php?chart=line&period=week">GDD</a>
                                </li>
                                
                            </ul>
                        <li>
                            <a href="environment.php"><i class="fa fa-table fa-fw"></i> Weather</a>
                        </li>
                        <li>
                            <a href="video.php"><i class="fa fa-video-camera fa-fw"></i> Live WebCam</a>
                        </li>
                        <li>
                            <a href="#"><i class="fa fa-gear fa-fw"></i>Settings<span class="fa arrow"></span></a>
                            <ul class="nav nav-second-level">
                                <li>
                                    <a href="/pages/admin/hiveconfig.php">Basic Config</a>
                                </li>
                                <li>
                                    <a href="/pages/admin/instrumentconfig.php">Instruments</a>
                                </li>
                                <li>
                                    <a href="/pages/admin/hivebodyconfig.php">Hive Body</a>
                                </li>
                            </ul>
                            <!-- /.nav-second-level -->
                        </li>
                        
   
                    </ul>
                </div>
                <!-- /.sidebar-collapse -->
            </div>
            <!-- /.navbar-static-side -->
        </nav>
        <div id="page-wrapper">
        ';
}?>
        