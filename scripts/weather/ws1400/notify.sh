#!/bin/bash
#Script to Notify

#Mail Settings
from="Name <NAme@Somedomain.com>"
to="you@email.com"
subject="Some Subject"

#Address or DNS Name


status2=$(echo $status |grep "Connect" |awk '{print $3'})
#echo status2

if [[ $status2 = Connected ]]; then
	echo "We are connected"
	echo "Successfully Rebooted ws1400ip" |mail -r "$from" -s "Success" $to

else
	echo "We had an error rebooting this thing - $status" |mail -r "$from" -s "$subject" $to

fi
