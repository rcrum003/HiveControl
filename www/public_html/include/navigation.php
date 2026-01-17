 <!-- Pull Config -->

<?PHP include($_SERVER["DOCUMENT_ROOT"] . "/include/db-connect.php");

#Get Site Orient and Options
$sth3 = $conn->prepare("SELECT SITE_ORIENT,HIVENAME,SITE_TYPE,SHOW_METRIC,HIVEAPI,HCVersion FROM hiveconfig");
$sth3->execute();
$result3 = $sth3->fetch(PDO::FETCH_ASSOC);

$orient = $result3['SITE_ORIENT'];
$id = $result3['HIVENAME'];
$SITE_TYPE = $result3['SITE_TYPE'];
$SHOW_METRIC = $result3['SHOW_METRIC'];
$HCVersion = $result3['HCVersion'];

# =====================================================================
# Set the header
# =====================================================================
 echo ' 
    <head>
   <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="Application to manage hive management tasks, and to report on various instruments.">
    <meta name="author" content="Ryan Crum">

    <title>'; echo $id; echo ' - Hive Control</title>

    <!-- Bootstrap Core CSS -->
    <link href="../bower_components/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet">

    <!-- MetisMenu CSS -->
    <link href="../bower_components/metisMenu/dist/metisMenu.min.css" rel="stylesheet">

    <!-- Timeline CSS -->
    <link href="../dist/css/timeline.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="../dist/css/sb-admin-2.css" rel="stylesheet">
    
    <!-- Custom Fonts -->
    <link href="../bower_components/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesnt work if you view the page via file:// -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->

</head>
<body>
<div id="wrapper">
';


#=====================
# Show either wide/or normal navigations
# Note, two different sets below
#=======================

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
                    <a href="/pages/index.php"><i class="fa fa-dashboard fa-fw"></i> Dashboard</a>
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
                        <li>
                            <a href="/pages/beecount.php?chart=line&period=week">Flights</a>
                        </li>
                        <li>
                            <a href="/pages/air.php?chart=line&period=week">Air Quality</a>
                        </li>
                    </ul>
                    <!-- /.dropdown-user -->
                </li>
                    <li>
                            <a href="/pages/video.php"><i class="fa fa-video-camera fa-fw"></i>Camera</a>
                        </li>
                <li class="dropdown">
                    <a class="dropdown-toggle" data-toggle="dropdown" href="#">
                        <i class="fa fa-gear fa-fw"></i> Settings <i class="fa fa-caret-down"></i>
                    </a>
                    <ul class="dropdown-menu">
                        <li>
                                    <a href="/admin/hiveconfig.php">Basic Config</a>
                                </li>
                                <li>
                                    <a href="/admin/instrumentconfig.php">Instruments</a>
                                </li>
                                <li>
                                    <a href="/admin/hivebodyconfig.php">Hive Components</a>
                                </li>
                                <li>
                                    <a href="/admin/siteconfig.php">Site Preferences</a>
                                </li>
                                <li>
                                    <a href="/admin/system.php">System Commands</a>
                                </li>
                                <li>
                                    <a href="/admin/backup.php">Backup & Restore</a>
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
                            <a href="/pages/index.php"><i class="fa fa-dashboard fa-fw"></i> Dashboard</a>
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
                                <li>
                                     <a href="/pages/beecount.php?chart=line&period=week">flights</a>
                                </li>
                                <li>
                                    <a href="/pages/air.php?chart=line&period=week">Air Quality</a>
                                </li>
                                
                            </ul>
                        <li>
                            <a href="/pages/video.php"><i class="fa fa-video-camera fa-fw"></i> Live WebCam</a>
                        </li>
                        <li>
                            <a href="#"><i class="fa fa-gear fa-fw"></i>Settings<span class="fa arrow"></span></a>
                            <ul class="nav nav-second-level">
                                <li>
                                    <a href="/admin/hiveconfig.php">Basic Config</a>
                                </li>
                                <li>
                                    <a href="/admin/instrumentconfig.php">Instruments</a>
                                </li>
                                <li>
                                    <a href="/admin/hivebodyconfig.php">Hive Body</a>
                                </li>
                                <li>
                                    <a href="/admin/siteconfig.php">Site Preferences</a>
                                </li>
                                <li>
                                    <a href="/admin/system.php">System Commands</a>
                                </li>
                                <li>
                                    <a href="/admin/backup.php">Backup & Restore</a>
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
        