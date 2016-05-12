#!/bin/bash
# Script to setup beecounter
# Only install if you are going to use the beecounter
# If you aren't, then you'll save a ton of space

foo = "yes"

if [[ $foo = "yes" ]]; then
	#statements

#Get Dependencies
# Don't run if part of install.sh script as it just ran
sudo apt-get update
sudo apt-get upgrade

#Make sure we have core developer tools
sudo apt-get install build-essential git cmake pkg-config -y

#Image utils
sudo apt-get install libjpeg-dev libtiff5-dev libjasper-dev libpng12-dev -y

#Video Utils
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev -y
sudo apt-get install libxvidcore-dev libx264-dev -y

#Need GTK, if we want to display on local desktop
sudo apt-get install libgtk2.0-dev -y

#Dependencies to help speed up OpenCV operations
sudo apt-get install libatlas-base-dev gfortran -y

#Python Header Files
sudo apt-get install python2.7-dev python3-dev -y

#Let's get OpenCV
mkdir /opt/OpenCV
cd /opt/OpenCV
wget -O opencv.zip https://github.com/Itseez/opencv/archive/3.1.0.zip
unzip opencv.zip
rm -rf opencv.zip

#Let's get contrib as well, to get some algorithms that aren't included in the base
wget -O opencv_contrib.zip https://github.com/Itseez/opencv_contrib/archive/3.1.0.zip
unzip opencv_contrib.zip
rm -rf opencv_contrib.zip

#Install PIP
mkdir /opt/pip
cd /opt/pip
wget https://bootstrap.pypa.io/get-pip.py
sudo python get-pip.py

#Install VirtualEnv, to enable a virtual environment. This helps protect against dependency conflicts in the future
# Just good practice..

##sudo pip install virtualenv virtualenvwrapper
##sudo rm -rf ~/.cache/pip
# virtualenv and virtualenvwrapper - Set environment variables
##cp ~/.profile ~/.oldprofile
##echo 'export WORKON_HOME=$HOME/.virtualenvs' >>~/.profile
##echo 'source /usr/local/bin/virtualenvwrapper.sh' >>~/.profile
##source ~/.profile


## Everything below is to setup your directory:

#Makes a new Virtual Environment for CV - we are instead going to make one for beecounter
#mkvirtualenv cv
# This makes a Python 2.7 environment
# for python3 - do 'mkvirtualenv beecounter -p python3'
################
#mkvirtualenv beecounter
#Sets up /root/.virtualenvs/beecounter/bin/python

# in the future, if you want to use this environment when you run python from command line, you'll have to run
#source ~/.profile
#workon beecounter

#Let's install OpenCV into our custom environment
####workon beecounter

#Install Numpy - this will take 15minutes
pip install numpy


#Compile and install OpenCV into our container
cd /opt/OpenCV/opencv-3.1.0/
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
	-D CMAKE_INSTALL_PREFIX=/usr/local \
	-D INSTALL_C_EXAMPLES=OFF \
	-D INSTALL_PYTHON_EXAMPLES=ON \
	-D OPENCV_EXTRA_MODULES_PATH=/opt/OpenCV/opencv_contrib-3.1.0/modules \
	-D BUILD_EXAMPLES=ON ..


#Now that our build is setup, we can compile
# This is going to take 1hr & 35m on a Pi2, shorter on a Pi3 , about 1hr 15m on PI3
make -j4

sudo make install
sudo ldconfig

#Install PICamera
sudo apt-get install python-picamera -y
fi

#------------------------------------
# Get mjpg_streamer code

# Compile it

# Install it

#Setup service
#sudo cp /home/HiveControl/install/init.d/beecounter /etc/init.d/beecounter
#Set permissions
#chmod +x /home/HiveControl/scripts/beecount/beecounter_svc.py

echo "Starting BeeCounter Service.........."
#service beecounter start

echo "**************************************************"
echo "BeeCamCounter Installation Completed"
echo "**************************************************"
