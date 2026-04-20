#Install .htpasswd
echo "Please set an admin password for http://127.0.0.1/admin/ access:"
cd /home/HiveControl/www/
htpasswd -c .htpasswd admin
sudo service apache2 restart

#Set your timezone
dpkg-reconfigure tzdata

echo "========================================================"
echo "Completed Setup of HiveControl"
echo "========================================================"