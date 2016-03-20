#!/bin/bash
#
# Reads the weight from an Adam Euipment CPW200plus scale
#
#
# get the device ( -d option )
#
while getopts d: option
do
        case "${option}"
        in
                d) DEVICE=${OPTARG};;
        esac
done

#stty -F $DEVICE -hupcl ignbrk  -icrnl -ixon -opost  -onlcr -isig -icanon -iexten -echo -echoe -echok  -echoctl -echoke igncr  cs8 -parenb 9600 -parodd

# read the scale
#   
COUNTER=1
while [[ ! $SCALE && COUNTER -lt 11 ]]
do
# Set the port to 9600 baud, 8 bit, no parity
  stty -F /dev/ttyUSB0 -hupcl ignbrk  -icrnl -ixon -opost  -onlcr -isig -icanon -iexten -echo -echoe -echok  -echoctl -echoke igncr  cs8 -parenb 9600 -parodd
  if [ "$?" -eq "0" ]
  then
#
# clear any garbage with a read
    read -t 0 SCALE < /dev/ttyUSB0
#
# spwan a process that waits a few miliseconds and then sends the command to the scale
# this gives the read command time to start listening
    `sleep 0.01;echo -e -n "N\r\n" > /dev/ttyUSB0` &
#
# read the scale but give it plenty of time to respond

    read -t 7 SCALE < $DEVICE
    SCALE=`echo $SCALE | gawk --posix '/^\+ [0-9]{1,3}\.[0-9] lb$/'`
    sleep 1
  else
    echo "$DATE ERROR stty failed to set $DEVICE"  >> /home/HiveControl/logs/error.log
  fi
let "COUNTER += 1"
done

if [[ $COUNTER -gt 10 ]]
then
  echo "$DATE ERROR reading Scale $DEVICE" >> /home/HiveControl/logs/error.log 
  SCALE=-555
fi 
if test $COUNTER -gt 2 
then 
  echo "$DATE WARNING reading Scale /dev/ttyS0: retried $COUNTER" >> /home/HiveControl/logs/error.log  
fi
echo "$SCALE"

