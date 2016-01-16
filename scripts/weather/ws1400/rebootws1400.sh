#!/bin/bash
#Script to reboot the ws1400ip

#Mail Settings
from="Beeyard <Beeyard@crumcasa.com>"
to="rcrum@crumcasa.com"
subject="WS1400ip Reboot Error"

#Address or DNS Name
ws1400=ws1400ip.110uni.com

#Default username/password
username=admin
password=admin
set -x
#Send reboot command
#{ sleep 1; echo "admin"; echo "admin"; sleep 1; echo "connstatus";} | telnet $ws1400
(sleep 1;echo $username;sleep 1;echo $password;sleep 1;echo reboot;sleep 1;) | telnet $ws1400

#Wait 15 seconds for it to come back up
sleep 10
status=$((sleep 1;echo $username;sleep 1;echo $password;sleep 1;echo connstatus;sleep 3;quit;) | telnet $ws1400)
#echo $status

status2=$(echo $status |grep "Connect" |awk '{print $3'})
#echo status2

if [[ $status2 = Connected ]]; then
	echo "We are connected"
	echo "Successfully Rebooted ws1400ip" |mail -r "$from" -s "Success" $to

else
	echo "We had an error rebooting this thing - $status" |mail -r "$from" -s "$subject" $to

fi




