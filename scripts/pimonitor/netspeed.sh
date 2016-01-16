#~!/bin/bash

NET=$(ping -c 10 192.168.110.1 |grep rtt)
#echo $NET

NETMIN=$(echo $NET |awk -F\/ '{print $4}' |awk '{print $3}')
NETAVG=$(echo $NET |awk -F\/ '{print $5}')
NETMAX=$(echo $NET |awk -F\/ '{print $6}')
NETMDEV=$(echo $NET |awk -F\/ '{print $7}' |awk '{print $1}')

#rtt min/avg/max/mdev = 3.677/7.084/11.660/2.161 ms

YARD=$(ping -c 10 192.168.110.233 |grep rtt)
YARDMIN=$(echo $YARD |awk -F\/ '{print $4}' |awk '{print $3}')
YARDAVG=$(echo $YARD |awk -F\/ '{print $5}')
YARDMAX=$(echo $YARD |awk -F\/ '{print $6}')
YARDMDEV=$(echo $YARD |awk -F\/ '{print $7}' |awk '{print $1}')

echo "GATEWAY"
echo "NETMIN=$NETMIN"
echo "NETAVG=$NETAVG"
echo "NETMAX=$NETMAX"
echo "NETMDEV=$NETMDEV"
echo "YARD"
echo "YARDMIN=$YARDMIN"
echo "YARDAVG=$YARDAVG"
echo "YARDMAX=$YARDMAX"
echo "YARDMDEV=$YARDMDEV"
