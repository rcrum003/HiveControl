
       <nav class="navbar navbar-default navbar-static-top" role="navigation" style="margin-bottom: 0">
            <div class="navbar-header">
                <button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-collapse">
                    <span class="sr-only">Toggle navigation</span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                </button>
                <h4 class="text-muted"><img src="/images/hive2.png"> Hive Control </h4>
            </div>
            <!-- /.navbar-header -->

            <!-- /.navbar-top-links -->

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
                                    <a href="hiveconfig.php">Basic Config</a>
                                </li>
                                <li>
                                    <a href="instrumentconfig.php">Instruments</a>
                                </li>
                                <li>
                                    <a href="hivebodyconfig.php">Hive Body</a>
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